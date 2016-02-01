/*
 * This file is part of libArcus
 *
 * Copyright (C) 2015 Ultimaker b.v. <a.hiemstra@ultimaker.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <thread>
#include <mutex>
#include <string>
#include <list>
#include <unordered_map>
#include <deque>
#include <iostream>

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

/**
 * Private implementation details for Socket.
 */
namespace Arcus
{
    using namespace Private;

    class Socket::Private
    {
    public:
        Private()
            : state(SocketState::Initial)
            , next_state(SocketState::Initial)
            , port(0)
            , thread(nullptr)
        {
        }

        void run();
        void sendMessage(const MessagePtr& message);
        void receiveNextMessage();
        void handleMessage(const std::shared_ptr<WireMessage>& wire_message);
        void checkConnectionState();

        void error(ErrorCode::ErrorCode error_code, const std::string& message);
        void fatalError(ErrorCode::ErrorCode error_code, const std::string& msg);

        SocketState::SocketState state;
        SocketState::SocketState next_state;

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

    // Report an error that should not cause the connection to abort.
    void Socket::Private::error(ErrorCode::ErrorCode error_code, const std::string& message)
    {
        Error error(error_code, message);
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
        last_error = error;

        platform_socket.close();
        current_message.reset();
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
                        platform_socket.setReceiveTimeout(250);
                        next_state = SocketState::Connected;
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
                        platform_socket.setReceiveTimeout(250);
                        next_state = SocketState::Connected;
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
    }

    // Send a message to the connected socket.
    void Socket::Private::sendMessage(const MessagePtr& message)
    {
        int32_t header = (ARCUS_SIGNATURE << 16) | (VERSION_MAJOR << 8) | (VERSION_MINOR);
        if(platform_socket.writeInt32(header) == -1)
        {
            error(ErrorCode::SendFailedError, "Could not send message header");
            return;
        }

        if(platform_socket.writeInt32(message->ByteSize()) == -1)
        {
            error(ErrorCode::SendFailedError, "Could not send message size");
            return;
        }

        if(platform_socket.writeInt32(message_types.getMessageTypeId(message)) == -1)
        {
            error(ErrorCode::SendFailedError, "Could not send message type");
            return;
        }

        std::string data = message->SerializeAsString();
        if(platform_socket.writeBytes(data.size(), data.data()) == -1)
        {
            error(ErrorCode::SendFailedError, "Could not send message data");
        }
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
            int32_t header = 0;
            platform_socket.readInt32(&header);

            if(header == 0) // Keep-alive, just return
                return;

            int signature = (header & 0xffff0000) >> 16;
            int major_version = (header & 0x0000ff00) >> 8;
            int minor_version = header & 0x000000ff;

            if(signature != ARCUS_SIGNATURE)
            {
                // Someone might be speaking to us in a different protocol?
                error(ErrorCode::ReceiveFailedError, "Header mismatch");
                platform_socket.flush();
                return;
            }

            if(major_version != VERSION_MAJOR)
            {
                error(ErrorCode::ReceiveFailedError, "Protocol version mismatch");
                platform_socket.flush();
                return;
            }

            if(minor_version != VERSION_MINOR)
            {
                error(ErrorCode::ReceiveFailedError, "Protocol version mismatch");
                platform_socket.flush();
                return;
            }

            current_message->state = WireMessage::MessageState::Size;
        }

        if(current_message->state == WireMessage::MessageState::Size)
        {
            int32_t size = 0;
            result = platform_socket.readInt32(&size);
            if(result == -1)
            {
                #ifndef _WIN32
                if (errno == EAGAIN)
                    return;
                #endif

                error(ErrorCode::ReceiveFailedError, "Size invalid");
                platform_socket.flush();
                return;
            }

            if(size < 0)
            {
                error(ErrorCode::ReceiveFailedError, "Size invalid");
                platform_socket.flush();
                return;
            }

            current_message->size = size;
            current_message->state = WireMessage::MessageState::Type;
        }

        if (current_message->state == WireMessage::MessageState::Type)
        {
            int32_t type = 0;
            result = platform_socket.readInt32(&type);
            if(result == -1)
            {
                #ifndef _WIN32
                if (errno == EAGAIN)
                    return;
                #endif
                current_message->valid = false;
            }

            uint32_t real_type = static_cast<uint32_t>(type);

            try
            {
                current_message->allocateData();
            }
            catch (std::bad_alloc& ba)
            {
                // Either way we're in trouble.
                fatalError(ErrorCode::ReceiveFailedError, "Out of memory");
                return;
            }

            current_message->type = real_type;
            current_message->state = WireMessage::MessageState::Data;
        }

        if (current_message->state == WireMessage::MessageState::Data)
        {
            result = platform_socket.readBytes(current_message->getRemainingSize(), &current_message->data[current_message->received_size]);

            if(result == -1)
            {
            #ifndef _WIN32
                if(errno != EAGAIN)
            #endif
                {
                    current_message.reset();
                }
            }
            else
            {
                current_message->received_size = current_message->received_size + result;
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
            error(ErrorCode::UnknownMessageTypeError, "Unknown message type");
            return;
        }

        MessagePtr message = message_types.createMessage(wire_message->type);

        google::protobuf::io::ArrayInputStream array(wire_message->data, wire_message->size);
        google::protobuf::io::CodedInputStream stream(&array);
        stream.SetTotalBytesLimit(message_size_maximum, message_size_warning);
        if(!message->ParseFromCodedStream(&stream))
        {
            error(ErrorCode::ParseFailedError, "Failed to parse message");
            return;
        }

        receiveQueueMutex.lock();
        receiveQueue.push_back(message);
        receiveQueueMutex.unlock();

        for(auto listener : listeners)
        {
            listener->messageReceived();
        }
    }

    // Send a keepalive packet to check whether we are still connected.
    void Socket::Private::checkConnectionState()
    {
        auto now = std::chrono::system_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_keep_alive_sent);

        if(diff.count() > keep_alive_rate)
        {
            int32_t keepalive = 0;
            if(platform_socket.writeInt32(keepalive) == -1)
            {
                error(ErrorCode::ConnectionResetError, "Connection reset by peer");
                next_state = SocketState::Closing;
            }
            last_keep_alive_sent = now;
        }
    }
}
