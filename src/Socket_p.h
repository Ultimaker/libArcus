/*
 * This file is part of libArcus
 *
 * Copyright (C) 2015 Ultimaker b.v. <a.hiemstra@ultimaker.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License v3.0 as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License v3.0 for more details.
 * You should have received a copy of the GNU Lesser General Public License v3.0
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <thread>
#include <mutex>
#include <string>
#include <list>
#include <unordered_map>
#include <deque>
#include <iostream>
#include <condition_variable>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <sys/time.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <signal.h>
#endif

#include <google/protobuf/message.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>

#include "Socket.h"
#include "Types.h"
#include "SocketListener.h"
#include "MessageTypeStore.h"
#include "Error.h"

#include "WireMessage_p.h"
#include "PlatformSocket_p.h"

#define VERSION_MAJOR 1
#define VERSION_MINOR 0

#define ARCUS_SIGNATURE 0x2BAD
#define SIG(n) (((n) & 0xffff0000) >> 16)

#define SOCKET_CLOSE 0xf0f0f0f0

#ifdef ARCUS_DEBUG
    #define DEBUG(message) debug(message)
#else
    #define DEBUG(message)
#endif

/**
 * Private implementation details for Socket.
 */
namespace Arcus
{
    using namespace Private;

    class ARCUS_NO_EXPORT Socket::Private
    {
    public:
        Private()
            : state(SocketState::Initial)
            , next_state(SocketState::Initial)
            , received_close(false)
            , port(0)
            , thread(nullptr)
        {
        }

        void run();
        void sendMessage(const MessagePtr& message);
        void receiveNextMessage();
        void handleMessage(const std::shared_ptr<WireMessage>& wire_message);
        void checkConnectionState();

        #ifdef ARCUS_DEBUG
        void debug(const std::string& message);
        #endif
        void error(ErrorCode::ErrorCode error_code, const std::string& message);
        void fatalError(ErrorCode::ErrorCode error_code, const std::string& msg);

        SocketState::SocketState state;
        SocketState::SocketState next_state;

        bool received_close;

        std::string address;
        uint port;

        std::thread* thread;

        std::list<SocketListener*> listeners;

        MessageTypeStore message_types;

        std::shared_ptr<Arcus::Private::WireMessage> current_message;

        std::deque<MessagePtr> sendQueue;
        std::mutex sendQueueMutex;
        std::deque<MessagePtr> receiveQueue;
        std::mutex receiveQueueMutex;

        std::mutex receiveQueueMutexBlock;
        std::condition_variable message_received_condition_variable;

        Arcus::Private::PlatformSocket platform_socket;

        Error last_error;

        std::chrono::system_clock::time_point last_keep_alive_sent;

        static const int keep_alive_rate = 500; //Number of milliseconds between sending keepalive packets

        // This value determines when protobuf should warn about very large messages.
        static const int message_size_warning = 400 * 1048576;

        // This value determines when protobuf should error out because the message is too large.
        // Due to the way Protobuf is implemented, messages large than 512MiB will cause issues.
        static const int message_size_maximum = 500 * 1048576;
    };

    #ifdef ARCUS_DEBUG
    void Socket::Private::debug(const std::string& message)
    {
        Error error(ErrorCode::Debug, std::string("[DEBUG] ") + message);
        for(auto listener : listeners)
        {
            listener->error(error);
        }
    }
    #endif

    // Report an error that should not cause the connection to abort.
    void Socket::Private::error(ErrorCode::ErrorCode error_code, const std::string& message)
    {
        Error error(error_code, message);
        error.setNativeErrorCode(platform_socket.getNativeErrorCode());

        last_error = error;

        for(auto listener : listeners)
        {
            listener->error(error);
        }
    }

    // Report an error that should cause the socket to go into an error state and abort the connection.
    void Socket::Private::fatalError(ErrorCode::ErrorCode error_code, const std::string& message)
    {
        Error error(error_code, message);
        error.setFatalError(true);
        error.setNativeErrorCode(platform_socket.getNativeErrorCode());

        last_error = error;

        platform_socket.close();
        next_state = SocketState::Error;

        for(auto listener : listeners)
        {
            listener->error(error);
        }
    }

    // Thread run method.
    void Socket::Private::run()
    {
        while(state != SocketState::Closed && state != SocketState::Error)
        {
            switch(state)
            {
                case SocketState::Connecting:
                {
                    if(!platform_socket.create())
                    {
                        fatalError(ErrorCode::CreationError, "Could not create a socket");
                    }
                    else if(!platform_socket.connect(address, port))
                    {
                        fatalError(ErrorCode::ConnectFailedError, "Could not connect to the given address");
                    }
                    else
                    {
                        if(!platform_socket.setReceiveTimeout(250))
                        {
                            fatalError(ErrorCode::ConnectFailedError, "Failed to set socket receive timeout");
                        }
                        else
                        {
                            DEBUG("Socket connected");
                            next_state = SocketState::Connected;
                        }
                    }
                    break;
                }
                case SocketState::Opening:
                {
                    if(!platform_socket.create())
                    {
                        fatalError(ErrorCode::CreationError, "Could not create a socket");
                    }
                    else if(!platform_socket.bind(address, port))
                    {
                        fatalError(ErrorCode::BindFailedError, "Could not bind to the given address and port");
                    }
                    else
                    {
                        next_state = SocketState::Listening;
                    }
                    break;
                }
                case SocketState::Listening:
                {
                    platform_socket.listen(1);
                    if(!platform_socket.accept())
                    {
                        fatalError(ErrorCode::AcceptFailedError, "Could not accept the incoming connection");
                    }
                    else
                    {
                        if(!platform_socket.setReceiveTimeout(250))
                        {
                            fatalError(ErrorCode::AcceptFailedError, "Could not set receive timeout of socket");
                        }
                        else
                        {
                            DEBUG("Socket connected");
                            next_state = SocketState::Connected;
                        }
                    }
                    break;
                }
                case SocketState::Connected:
                {
                    //Get all the messages from the queue and store them in a temporary array so we can
                    //unlock the queue before performing the send.
                    std::list<MessagePtr> messagesToSend;
                    sendQueueMutex.lock();
                    while(sendQueue.size() > 0)
                    {
                        messagesToSend.push_back(sendQueue.front());
                        sendQueue.pop_front();
                    }
                    sendQueueMutex.unlock();

                    for(auto message : messagesToSend)
                    {
                        sendMessage(message);
                    }

                    receiveNextMessage();

                    if(next_state != SocketState::Error)
                    {
                        checkConnectionState();
                    }

                    break;
                }
                case SocketState::Closing:
                {
                    if(!received_close)
                    {
                        // We want to close the socket.
                        // First, flush the send queue so it is empty.
                        std::list<MessagePtr> messagesToSend;
                        sendQueueMutex.lock();
                        while(sendQueue.size() > 0)
                        {
                            messagesToSend.push_back(sendQueue.front());
                            sendQueue.pop_front();
                        }
                        sendQueueMutex.unlock();

                        for(auto message : messagesToSend)
                        {
                            sendMessage(message);
                        }

                        // Communicate to the other side that we want to close.
                        platform_socket.writeUInt32(SOCKET_CLOSE);
                        // Disable further writing to the socket.
                        error(ErrorCode::Debug, "We got a request to close the socket.");
                        platform_socket.shutdown(PlatformSocket::ShutdownDirection::ShutdownWrite);

                        // Wait until we receive confirmation from the other side to actually close.
                        uint32_t data = 0;
                        while(data != SOCKET_CLOSE && next_state == SocketState::Closing)
                        {
                            if(platform_socket.readUInt32(&data) == -1)
                            {
                                break;
                            }
                        }
                    }
                    else
                    {
                        // The other side requested a close. Drop all pending messages
                        // since the other socket will not process them anyway.
                        sendQueueMutex.lock();
                        sendQueue.clear();
                        sendQueueMutex.unlock();

                        // Send confirmation to the other side that we received their close
                        // request and are also closing down.
                        platform_socket.writeUInt32(SOCKET_CLOSE);
                        // Prevent further writing to the socket.
                        platform_socket.shutdown(PlatformSocket::ShutdownDirection::ShutdownWrite);

                        // At this point the socket can safely be closed, assuming that SOCKET_CLOSE
                        // is the last data received from the other socket and everything was received
                        // in order (which should be guaranteed by TCP).
                    }

                    error(ErrorCode::Debug, "Closing socket because other side requested close.");
                    platform_socket.close();
                    next_state = SocketState::Closed;
                    break;
                }
                default:
                    break;
            }

            if(next_state != state)
            {
                state = next_state;

                for(auto listener : listeners)
                {
                    listener->stateChanged(state);
                }
            }
        }

        message_received_condition_variable.notify_all();
    }

    // Send a message to the connected socket.
    void Socket::Private::sendMessage(const MessagePtr& message)
    {
        uint32_t header = (ARCUS_SIGNATURE << 16) | (VERSION_MAJOR << 8) | (VERSION_MINOR);
        if(platform_socket.writeUInt32(header) == -1)
        {
            error(ErrorCode::SendFailedError, "Could not send message header");
            return;
        }

        uint32_t message_size = message->ByteSize();
        if(platform_socket.writeUInt32(message_size) == -1)
        {
            error(ErrorCode::SendFailedError, "Could not send message size");
            return;
        }

        uint32_t type_id = message_types.getMessageTypeId(message);
        if(platform_socket.writeUInt32(type_id) == -1)
        {
            error(ErrorCode::SendFailedError, "Could not send message type");
            return;
        }

        std::string data = message->SerializeAsString();
        if(platform_socket.writeBytes(data.size(), data.data()) == -1)
        {
            error(ErrorCode::SendFailedError, "Could not send message data");
        }
        DEBUG(std::string("Sending message of type ") + std::to_string(type_id) + " and size " + std::to_string(message_size));
    }

    // Handle receiving data until we have a proper message.
    void Socket::Private::receiveNextMessage()
    {
        int result = 0;

        if(!current_message)
        {
            current_message = std::make_shared<WireMessage>();
        }

        if(current_message->state == WireMessage::MessageState::Header)
        {
            uint32_t header = 0;
            platform_socket.readUInt32(&header);

            if(header == 0) // Keep-alive, just return
            {
                return;
            }
            else if(header == SOCKET_CLOSE)
            {
                // We received a close request from the other socket, so close this socket as well.
                next_state = SocketState::Closing;
                received_close = true;
                return;
            }

            int signature = (header & 0xffff0000) >> 16;
            int major_version = (header & 0x0000ff00) >> 8;
            int minor_version = header & 0x000000ff;

            if(signature != ARCUS_SIGNATURE)
            {
                // Someone might be speaking to us in a different protocol?
                error(ErrorCode::ReceiveFailedError, "Header mismatch");
                current_message.reset();
                platform_socket.flush();
                return;
            }

            if(major_version != VERSION_MAJOR)
            {
                error(ErrorCode::ReceiveFailedError, "Protocol version mismatch");
                current_message.reset();
                platform_socket.flush();
                return;
            }

            if(minor_version != VERSION_MINOR)
            {
                error(ErrorCode::ReceiveFailedError, "Protocol version mismatch");
                current_message.reset();
                platform_socket.flush();
                return;
            }

            DEBUG("Incoming message, header ok");
            current_message->state = WireMessage::MessageState::Size;
        }

        if(current_message->state == WireMessage::MessageState::Size)
        {
            uint32_t size = 0;
            result = platform_socket.readUInt32(&size);
            if(result == 0)
            {
                return;
            }
            else if(result == -1)
            {
                error(ErrorCode::ReceiveFailedError, "Size invalid");
                current_message.reset();
                platform_socket.flush();
                return;
            }

            DEBUG(std::string("Incoming message size: ") + std::to_string(size));
            current_message->size = size;
            current_message->state = WireMessage::MessageState::Type;
        }

        if (current_message->state == WireMessage::MessageState::Type)
        {
            uint32_t type = 0;
            result = platform_socket.readUInt32(&type);
            if(result == 0)
            {
                return;
            }
            else if(result == -1)
            {
                error(ErrorCode::ReceiveFailedError, "Receiving type failed");
                current_message->valid = false;
            }

            uint32_t real_type = static_cast<uint32_t>(type);

            try
            {
                current_message->allocateData();
            }
            catch (std::bad_alloc&)
            {
                // Either way we're in trouble.
                current_message.reset();
                fatalError(ErrorCode::ReceiveFailedError, "Out of memory");
                return;
            }

            DEBUG(std::string("Incoming message type: ") + std::to_string(real_type));
            current_message->type = real_type;
            current_message->state = WireMessage::MessageState::Data;
        }

        if (current_message->state == WireMessage::MessageState::Data)
        {
            result = platform_socket.readBytes(current_message->getRemainingSize(), &current_message->data[current_message->received_size]);

            if(result < 0)
            {
                error(ErrorCode::ReceiveFailedError, "Could not receive data for message");
                current_message.reset();
                return;
            }
            else
            {
                current_message->received_size = current_message->received_size + result;

                DEBUG("Received " + std::to_string(result) + " bytes data");

                if(current_message->isComplete())
                {
                    if(!current_message->valid)
                    {
                        current_message.reset();
                        return;
                    }

                    current_message->state = WireMessage::MessageState::Dispatch;
                }
            }
        }

        if (current_message->state == WireMessage::MessageState::Dispatch)
        {
            handleMessage(current_message);
            current_message.reset();
        }
    }

    // Parse and process a message received on the socket.
    void Socket::Private::handleMessage(const std::shared_ptr<WireMessage>& wire_message)
    {
        if(!message_types.hasType(wire_message->type))
        {
            DEBUG(std::string("Received message type: ") + std::to_string(wire_message->type));
            error(ErrorCode::UnknownMessageTypeError, "Unknown message type");
            return;
        }

        MessagePtr message = message_types.createMessage(wire_message->type);

        google::protobuf::io::ArrayInputStream array(wire_message->data, wire_message->size);
        google::protobuf::io::CodedInputStream stream(&array);
        stream.SetTotalBytesLimit(message_size_maximum, message_size_warning);
        if(!message->ParseFromCodedStream(&stream))
        {
            error(ErrorCode::ParseFailedError, "Failed to parse message:" + std::string(wire_message->data));
            return;
        }

        DEBUG(std::string("Received a message of type ") + std::to_string(wire_message->type) + " and size " + std::to_string(wire_message->size));

        receiveQueueMutex.lock();
        receiveQueue.push_back(message);
        receiveQueueMutex.unlock();

        for(auto listener : listeners)
        {
            listener->messageReceived();
        }

        message_received_condition_variable.notify_all();
    }

    // Send a keepalive packet to check whether we are still connected.
    void Socket::Private::checkConnectionState()
    {
        auto now = std::chrono::system_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_keep_alive_sent);

        if(diff.count() > keep_alive_rate)
        {
            int32_t keepalive = 0;
            if(platform_socket.writeUInt32(keepalive) == -1)
            {
                error(ErrorCode::ConnectionResetError, "Connection reset by peer");
                next_state = SocketState::Closing;
            }
            last_keep_alive_sent = now;
        }
    }
}
