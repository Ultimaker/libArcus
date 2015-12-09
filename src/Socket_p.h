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

#include "Types.h"
#include "SocketListener.h"

#define VERSION_MAJOR 0
#define VERSION_MINOR 1

#define ARCUS_SIGNATURE 0x2BAD
#define SIG(n) (((n) & 0xffff0000) >> 16)

#ifndef MSG_NOSIGNAL
	#define MSG_NOSIGNAL 0x0 //Don't request NOSIGNAL on systems where this is not implemented.
#endif

/**
 * Private implementation details for Socket.
 */
namespace Arcus
{
    enum MessageState {
        MESSAGE_STATE_INIT,
        MESSAGE_STATE_HEADER,
        MESSAGE_STATE_SIZE,
        MESSAGE_STATE_TYPE,
        MESSAGE_STATE_DATA,
        MESSAGE_STATE_DISPATCH,
    };

    class SocketPrivate
    {
    public:
        SocketPrivate()
            : state(SocketState::Initial)
            , next_state(SocketState::Initial)
            , port(0)
            , thread(nullptr)
            , message({})
            , last_keep_alive_sent(std::chrono::system_clock::now())
        {



        #ifdef _WIN32
            initializeWSA();
        #endif
        }

        void run();
        sockaddr_in createAddress();
        void sendMessage(MessagePtr message);
        void receiveNextMessage();
        int readInt32(int32_t *dest);
        int readBytes(int size, char* dest);
        void handleMessage(int type, int size, char* buffer);
        void setSocketReceiveTimeout(int socketId, int timeout);
        void checkConnectionState();
        void error(std::string msg);

        SocketState::State state;
        SocketState::State next_state;

        std::string address;
        int port;

        std::thread* thread;

        std::list<SocketListener*> listeners;

        std::unordered_map<int, const google::protobuf::Message*> message_types;
        std::unordered_map<const google::protobuf::Descriptor*, int> message_type_mapping;

        std::deque<MessagePtr> send_queue;
        std::mutex send_queue_mutex;
        std::deque<MessagePtr> receive_queue;
        std::mutex receive_queue_mutex;

        struct {
            Arcus::MessageState state;
            int type;
            int size;
            int size_recv;
            bool valid;
            char *data;
        } message;

        std::string error_string;

        int socket_id;

        std::chrono::system_clock::time_point last_keep_alive_sent;

        static const int keep_alive_rate = 500; //Number of milliseconds between sending keepalive packets

    #ifdef _WIN32
        static bool wsa_initialized;
        static void initializeWSA();
    #endif
    };

#ifdef _WIN32
    bool SocketPrivate::wsa_initialized = false;
#endif

    void SocketPrivate::error(std::string msg)
    {
    	error_string = msg;
#ifdef _WIN32
        ::closesocket(socketId);
#else
        ::close(socket_id);
#endif
    	next_state = SocketState::Error;

        for (auto listener : listeners)
        {
            listener->error(error_string);
        }
    }

    // This is run in a thread.
    void SocketPrivate::run()
    {
        while (state != SocketState::Closed && state != SocketState::Error)
        {
            switch(state)
            {
                case SocketState::Connecting:
                {
                    socket_id = ::socket(AF_INET, SOCK_STREAM, 0);
                    sockaddr_in address_data = createAddress();
                    if (::connect(socket_id, reinterpret_cast<sockaddr*>(&address_data), sizeof(address_data)))
                    {
                        error_string = "Could not connect to the given address";
                        next_state = SocketState::Error;

                        for (auto listener : listeners)
                        {
                            listener->error(error_string);
                        }
                    }
                    else
                    {
                        setSocketReceiveTimeout(socket_id, 250);
                        next_state = SocketState::Connected;
                    }
                    break;
                }
                case SocketState::Opening:
                {
                    socket_id = ::socket(AF_INET, SOCK_STREAM, 0);
                    sockaddr_in address_data = createAddress();
                    if (::bind(socket_id, reinterpret_cast<sockaddr*>(&address_data), sizeof(address_data)))
                    {
                        error_string =  "Could not bind to the given address";
                        next_state = SocketState::Error;

                        for (auto listener : listeners)
                        {
                            listener->error(error_string);
                        }
                    }
                    else
                    {
                        next_state = SocketState::Listening;
                    }
                    break;
                }
                case SocketState::Listening:
                {
                    ::listen(socket_id, 1);

                    int new_socket = ::accept(socket_id, 0, 0);
                    if (new_socket == -1)
                    {
                        error_string = "Could not accept connection";
                        next_state = SocketState::Error;

                        for (auto listener : listeners)
                        {
                            listener->error(error_string);
                        }
                    }

                #ifdef _WIN32
                    ::closesocket(socketId);
                #else
                    ::close(socket_id);
                #endif
                    socket_id = new_socket;
                    setSocketReceiveTimeout(socket_id, 250);
                    next_state = SocketState::Connected;
                    break;
                }
                case SocketState::Connected:
                {
                    //Get all the messages from the queue and store them in a temporary array so we can
                    //unlock the queue before performing the send.
                    std::list<MessagePtr> messages_to_send;
                    send_queue_mutex.lock();
                    while (send_queue.size() > 0)
                    {
                        messages_to_send.push_back(send_queue.front());
                        send_queue.pop_front();
                    }
                    send_queue_mutex.unlock();

                    for (auto message : messages_to_send)
                    {
                        sendMessage(message);
                    }

                    receiveNextMessage();

                    if (next_state != SocketState::Error)
                    	checkConnectionState();

                    break;
                }
                case SocketState::Closing:
                {
                    std::list<MessagePtr> messages_to_send;
                    send_queue_mutex.lock();
                    while (send_queue.size() > 0)
                    {
                        messages_to_send.push_back(send_queue.front());
                        send_queue.pop_front();
                    }
                    send_queue_mutex.unlock();

                    for (auto message : messages_to_send)
                    {
                        sendMessage(message);
                    }

                #ifdef _WIN32
                    ::closesocket(socketId);
                #else
                    ::close(socket_id);
                #endif
                    next_state = SocketState::Closed;
                    break;
                }
                default:
                    break;
            }

            if (next_state != state)
            {
                state = next_state;

                for (auto listener : listeners)
                {
                    listener->stateChanged(state);
                }
            }
        }
    }

    // Create a sockaddr_in structure from the address and port variables.
    sockaddr_in SocketPrivate::createAddress()
    {
        sockaddr_in a;
        a.sin_family = AF_INET;
    #ifdef _WIN32
        InetPton(AF_INET, address.c_str(), &(a.sin_addr)); //Note: Vista and higher only.
    #else
        ::inet_pton(AF_INET, address.c_str(), &(a.sin_addr));
    #endif
        a.sin_port = htons(port);
        return a;
    }

    void SocketPrivate::sendMessage(MessagePtr message)
    {
        //TODO: Improve error handling.
        uint32_t hdr = htonl((ARCUS_SIGNATURE << 16) | (VERSION_MAJOR << 8) | VERSION_MINOR);
        size_t sent_size = ::send(socket_id, reinterpret_cast<const char*>(&hdr), 4, MSG_NOSIGNAL);

        int size = htonl(message->ByteSize());
        sent_size = ::send(socket_id, reinterpret_cast<const char*>(&size), 4, MSG_NOSIGNAL);

        int type = htonl(message_type_mapping[message->GetDescriptor()]);
        sent_size = ::send(socket_id, reinterpret_cast<const char*>(&type), 4, MSG_NOSIGNAL);

        std::string data = message->SerializeAsString();
        sent_size = ::send(socket_id, data.data(), data.size(), MSG_NOSIGNAL);
    }


    void SocketPrivate::receiveNextMessage()
    {
        int32_t hdr;
        int ret;

        if (message.state == MESSAGE_STATE_INIT)
        {
            message.valid = true;
            message.size = 0;
            message.size_recv = 0;
            message.type = 0;
            message.state = MESSAGE_STATE_HEADER;
        }

        if (message.state == MESSAGE_STATE_HEADER)
        {
            if (readInt32(&hdr) || hdr == 0) /* Keep-alive, just return */
                return;

            if (SIG(hdr) != ARCUS_SIGNATURE)
            {
                /* Someone might be speaking to us in a different protocol? */
                error("Header mismatch");
                return;
            }

            message.state = MESSAGE_STATE_SIZE;
        }

        if (message.state == MESSAGE_STATE_SIZE)
        {
            ret = readInt32(&message.size);
            if (ret) {
#ifndef _WIN32
                if (errno == EAGAIN)
                    return;
#endif
                error("Size invalid");
                message.state = MESSAGE_STATE_INIT;
                return;
            }

            if (message.size < 0)
            {
                message.state = MESSAGE_STATE_INIT;
                error("Size invalid");
                return;
            }

            message.state = MESSAGE_STATE_TYPE;
        }

        if (message.state == MESSAGE_STATE_TYPE)
        {
            ret = readInt32(&message.type);
            if (ret) {
#ifndef _WIN32
                if (errno == EAGAIN)
                    return;
#endif
                error("Type invalid");
                message.valid = false;
            }

            if (message.type < 0)
            {
                error("Type invalid");
                message.valid = false;
            }

            try {
                message.data = new char[message.size];
            } catch (std::bad_alloc& ba) {
                /* Either way we're in trouble. */
                error("Received malformed package or out of memory");
                message.state = MESSAGE_STATE_INIT;
                return;
            }
            message.state = MESSAGE_STATE_DATA;
        }

        if (message.state == MESSAGE_STATE_DATA)
        {
            ret = readBytes(message.size - message.size_recv,
                    &message.data[message.size_recv]);
            if (ret == -1)
            {
            #ifndef _WIN32
                if (errno != EAGAIN)
            #endif
                {
                    delete[] message.data;
                    message.data = nullptr;
                    message.size_recv = 0;
                    message.state = MESSAGE_STATE_INIT;
                }
            }
            else
            {
                message.size_recv += ret;
                if (message.size_recv >= message.size)
                {
                    if (message.valid) {
                        message.state = MESSAGE_STATE_DISPATCH;
                    }
                    else
                    {
                        delete[] message.data;
                        message.data = nullptr;
                        message.state = MESSAGE_STATE_INIT;
                    }
                }
            }
        }

        if (message.state == MESSAGE_STATE_DISPATCH)
        {
            handleMessage(message.type, message.size, message.data);
            delete[] message.data;
            message.data = nullptr;
            message.state = MESSAGE_STATE_INIT;
        }
    }

    int SocketPrivate::readInt32(int32_t *dest)
    {
        int32_t buffer;
        int num = ::recv(socket_id, reinterpret_cast<char*>(&buffer), 4, 0);

        if (num != 4)
        {
            return -1;
        }

        *dest = ntohl(buffer);
        return 0;
    }

    int SocketPrivate::readBytes(int size, char* dest)
    {
        int num = ::recv(socket_id, dest, size, 0);
        if (num == -1)
        {
            return -1;
        }
        else
        {
            return num;
        }
    }

    void SocketPrivate::handleMessage(int type, int size, char* buffer)
    {
        if (message_types.find(type) == message_types.end())
        {
            error_string = "Unknown message type";
            return;
        }

        MessagePtr message = MessagePtr(message_types[type]->New());
        google::protobuf::io::ArrayInputStream array(buffer, size);
        google::protobuf::io::CodedInputStream stream(&array);
        stream.SetTotalBytesLimit(500 * 1048576, 128 * 1048576); //Set size limit to 500MiB, warn at 128MiB
        if (!message->ParseFromCodedStream(&stream))
        {
            error_string = "Failed to parse message";
            return;
        }

        receive_queue_mutex.lock();
        receive_queue.push_back(message);
        receive_queue_mutex.unlock();

        for (auto listener : listeners)
        {
            listener->messageReceived();
        }
    }

    // Set socket timeout value in milliseconds
    void SocketPrivate::setSocketReceiveTimeout(int socket_id, int timeout)
    {
        timeval t;
        t.tv_sec = 0;
        t.tv_usec = timeout * 1000;
        ::setsockopt(socket_id, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&t), sizeof(t));
    }

    // Send a keepalive packet to check whether we are still connected.
    void SocketPrivate::checkConnectionState()
    {
        auto now = std::chrono::system_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_keep_alive_sent);

        if (diff.count() > keep_alive_rate)
        {
            int32_t keepalive = 0;
            if (::send(socket_id, reinterpret_cast<const char*>(&keepalive), 4, MSG_NOSIGNAL) == -1)
            {
                error_string = "Connection reset by peer";
                next_state = SocketState::Closing;

                for (auto listener : listeners)
                {
                    listener->error(error_string);
                }
            }
            last_keep_alive_sent = now;
        }
    }

#ifdef _WIN32
    void SocketPrivate::initializeWSA()
    {
        if (!wsa_initialized)
        {
            WSADATA wsa_data;
            WSAStartup(MAKEWORD(2, 2), &wsa_data);
            wsa_initialized = true;
        }
    }
#endif

}


