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

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/socket.h>
    #include <sys/time.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
	#include <signal.h>
#endif

#include <google/protobuf/message.h>

#include "Types.h"
#include "SocketListener.h"

#define VERSION_MAJOR 0
#define VERSION_MINOR 1

#define ARCUS_SIGNATURE 0x2BAD
#define SIG(n) (((n) & 0xffff0000) >> 16)

/**
 * Private implementation details for Socket.
 */
namespace Arcus
{
    class SocketPrivate
    {
    public:
        SocketPrivate()
            : state(SocketState::Initial)
            , nextState(SocketState::Initial)
            , port(0)
            , thread(nullptr)
            , partialMessage(nullptr)
            , messageType(0)
            , messageSize(0)
            , amountReceived(0)
            , messageValid(0)
            , lastKeepAliveSent(std::chrono::system_clock::now())
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
        SocketState::State nextState;

        std::string address;
        int port;

        std::thread* thread;

        std::list<SocketListener*> listeners;

        std::unordered_map<int, const google::protobuf::Message*> messageTypes;
        std::unordered_map<const google::protobuf::Descriptor*, int> messageTypeMapping;

        std::deque<MessagePtr> sendQueue;
        std::mutex sendQueueMutex;
        std::deque<MessagePtr> receiveQueue;
        std::mutex receiveQueueMutex;

        char* partialMessage;
        int messageType;
        int messageSize;
        int amountReceived;
        int messageValid;

        std::string errorString;

        int socketId;

        std::chrono::system_clock::time_point lastKeepAliveSent;

        static const int keepAliveRate = 500; //Number of milliseconds between sending keepalive packets

    #ifdef _WIN32
        static bool wsaInitialized;
        static void initializeWSA();
    #endif
    };

#ifdef _WIN32
    bool SocketPrivate::wsaInitialized = false;
#endif

    void SocketPrivate::error(std::string msg)
    {
    	errorString = msg;
#ifdef _WIN32
        ::closesocket(socketId);
#else
        ::close(socketId);
#endif
    	nextState = SocketState::Error;

        for(auto listener : listeners)
        {
            listener->error(errorString);
        }
    }

    // This is run in a thread.
    void SocketPrivate::run()
    {
#ifndef _WIN32
        signal(SIGPIPE, SIG_IGN);
#endif

        while(state != SocketState::Closed && state != SocketState::Error)
        {
            switch(state)
            {
                case SocketState::Connecting:
                {
                    socketId = ::socket(AF_INET, SOCK_STREAM, 0);
                    sockaddr_in address_data = createAddress();
                    if(::connect(socketId, reinterpret_cast<sockaddr*>(&address_data), sizeof(address_data)))
                    {
                        errorString = "Could not connect to the given address";
                        nextState = SocketState::Error;

                        for(auto listener : listeners)
                        {
                            listener->error(errorString);
                        }
                    }
                    else
                    {
                        setSocketReceiveTimeout(socketId, 250);
                        nextState = SocketState::Connected;
                    }
                    break;
                }
                case SocketState::Opening:
                {
                    socketId = ::socket(AF_INET, SOCK_STREAM, 0);
                    sockaddr_in address_data = createAddress();
                    if(::bind(socketId, reinterpret_cast<sockaddr*>(&address_data), sizeof(address_data)))
                    {
                        errorString =  "Could not bind to the given address";
                        nextState = SocketState::Error;

                        for(auto listener : listeners)
                        {
                            listener->error(errorString);
                        }
                    }
                    else
                    {
                        nextState = SocketState::Listening;
                    }
                    break;
                }
                case SocketState::Listening:
                {
                    ::listen(socketId, 1);

                    int newSocket = ::accept(socketId, 0, 0);
                    if(newSocket == -1)
                    {
                        errorString = "Could not accept connection";
                        nextState = SocketState::Error;

                        for(auto listener : listeners)
                        {
                            listener->error(errorString);
                        }
                    }

                #ifdef _WIN32
                    ::closesocket(socketId);
                #else
                    ::close(socketId);
                #endif
                    socketId = newSocket;
                    setSocketReceiveTimeout(socketId, 250);
                    nextState = SocketState::Connected;
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

                    if (nextState != SocketState::Error)
                    	checkConnectionState();

                    break;
                }
                case SocketState::Closing:
                {
                #ifdef _WIN32
                    ::closesocket(socketId);
                #else
                    ::close(socketId);
                #endif
                    nextState = SocketState::Closed;
                    break;
                }
                default:
                    break;
            }

            if(nextState != state)
            {
                state = nextState;

                for(auto listener : listeners)
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
        a.sin_port = ::htons(port);
        return a;
    }

    void SocketPrivate::sendMessage(MessagePtr message)
    {
        //TODO: Improve error handling.
        uint32_t hdr = ::htonl((ARCUS_SIGNATURE << 16) | (VERSION_MAJOR << 8) | VERSION_MINOR);
        size_t sent_size = ::send(socketId, reinterpret_cast<const char*>(&hdr), 4, 0);

        int size = ::htonl(message->ByteSize());
        sent_size = ::send(socketId, reinterpret_cast<const char*>(&size), 4, 0);

        int type = ::htonl(messageTypeMapping[message->GetDescriptor()]);
        sent_size = ::send(socketId, reinterpret_cast<const char*>(&type), 4, 0);

        std::string data = message->SerializeAsString();
        sent_size = ::send(socketId, data.data(), data.size(), 0);
    }

    void SocketPrivate::receiveNextMessage()
    {
        char* buffer;
        int32_t hdr;

        //Continuation of message receive from previous call.
        if(partialMessage)
        {
            int readSize = readBytes(messageSize - amountReceived, partialMessage + amountReceived);
            if(readSize == -1)
            {
                delete[] partialMessage;
                partialMessage = nullptr;
                amountReceived = 0;
            }
            else
            {
                amountReceived += readSize;
                if(amountReceived >= messageSize)
                {
                    handleMessage(messageType, messageSize, partialMessage);
                    delete[] partialMessage;
                    partialMessage = nullptr;
                    amountReceived = 0;
                }
            }

            return;
        }

        messageValid = 1;
        if (readInt32(&hdr) || hdr == 0) /* Keep-alive, just return */
            return;

        if (SIG(hdr) != ARCUS_SIGNATURE) {
            /* Someone might be speaking to us in a different protocol? */
            error("Header mismatch");
            return;
        }

        if (readInt32(&messageSize) || messageSize < 0) {
            error("Size invalid");
            return;
        }

        if (readInt32(&messageType)) {
            error("Could not read message type");
            return;
        }

        if (messageType <= 0)
            messageValid = 0; /* Try and skip over it */

        try {
            buffer = new char[messageSize];
        } catch (std::bad_alloc& ba) {
            /* Either way we're in trouble. */
            error("Received malformed package or out of memory");
            return;
        }

        int readSize = readBytes(messageSize, buffer);
        if (readSize == messageSize) {
            handleMessage(messageType, messageSize, buffer);
            delete[] buffer;
        } else {
            partialMessage = buffer;
            amountReceived = readSize;
        }
    }

    int SocketPrivate::readInt32(int32_t *dest)
    {
        int32_t buffer;
        size_t num = ::recv(socketId, reinterpret_cast<char*>(&buffer), 4, 0);
        if(num != 4)
        {
            return -1;
        }

        *dest = ntohl(buffer);
        return 0;
    }

    int SocketPrivate::readBytes(int size, char* dest)
    {
        size_t num = ::recv(socketId, dest, size, 0);
        if(num == -1)
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
        if(!messageValid)
            return;

        if(messageTypes.find(type) == messageTypes.end())
        {
            errorString = "Unknown message type";
            return;
        }

        MessagePtr message = MessagePtr(messageTypes[type]->New());
        if(!message->ParseFromArray(buffer, size))
        {
            errorString = "Failed to parse message";
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

    // Set socket timeout value in milliseconds
    void SocketPrivate::setSocketReceiveTimeout(int socketId, int timeout)
    {
        timeval t;
        t.tv_sec = 0;
        t.tv_usec = timeout * 1000;
        ::setsockopt(socketId, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&t), sizeof(t));
    }

    // Send a keepalive packet to check whether we are still connected.
    void SocketPrivate::checkConnectionState()
    {
        auto now = std::chrono::system_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastKeepAliveSent);

        if(diff.count() > keepAliveRate)
        {
            int32_t keepalive = 0;
            if(::send(socketId, reinterpret_cast<const char*>(&keepalive), 4, 0) == -1)
            {
                errorString = "Connection reset by peer";
                nextState = SocketState::Closing;

                for(auto listener : listeners)
                {
                    listener->error(errorString);
                }
            }
            lastKeepAliveSent = now;
        }
    }

#ifdef _WIN32
    void SocketPrivate::initializeWSA()
    {
        if(!wsaInitialized)
        {
            WSADATA wsaData;
            WSAStartup(MAKEWORD(2, 2), &wsaData);
            wsaInitialized = true;
        }
    }
#endif

}


