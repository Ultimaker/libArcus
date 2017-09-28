/*
 * This file is part of libArcus
 *
 * Copyright (C) 2016 Ultimaker b.v. <a.hiemstra@ultimaker.com>
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

#include "PlatformSocket_p.h"

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
    #include <errno.h>
#endif

#ifndef MSG_NOSIGNAL
    #define MSG_NOSIGNAL 0x0 //Don't request NOSIGNAL on systems where this is not implemented.
#endif

#ifndef MSG_DONTWAIT
    #define MSG_DONTWAIT 0x0
#endif

using namespace Arcus::Private;

#ifdef _WIN32
void initializeWSA()
{
    static bool wsaInitialized = false;

    if(!wsaInitialized)
    {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        wsaInitialized = true;
    }
}
#endif

// Create a sockaddr_in structure from an address and port.
sockaddr_in createAddress(const std::string& address, int port)
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

Arcus::Private::PlatformSocket::PlatformSocket()
{
#ifdef _WIN32
    initializeWSA();
#endif
}

Arcus::Private::PlatformSocket::~PlatformSocket()
{
}

bool Arcus::Private::PlatformSocket::create()
{
    _socket_id = ::socket(AF_INET, SOCK_STREAM, 0);
    return _socket_id != -1;
}

bool Arcus::Private::PlatformSocket::connect(const std::string& address, int port)
{
    auto address_data = createAddress(address, port);
    int result = ::connect(_socket_id, reinterpret_cast<sockaddr*>(&address_data), sizeof(address_data));
    return result == 0;
}

bool Arcus::Private::PlatformSocket::bind(const std::string& address, int port)
{
    auto address_data = createAddress(address, port);
    int result = ::bind(_socket_id, reinterpret_cast<sockaddr*>(&address_data), sizeof(address_data));
    return result == 0;
}

bool Arcus::Private::PlatformSocket::listen(int backlog)
{
    int result = ::listen(_socket_id, backlog);
    return result == 0;
}

bool Arcus::Private::PlatformSocket::accept()
{
    int new_socket = ::accept(_socket_id, 0, 0);

    #ifdef _WIN32
        ::closesocket(_socket_id);
    #else
        ::close(_socket_id);
    #endif

    if(new_socket == -1)
    {
        return false;
    }
    else
    {
        _socket_id = new_socket;
        return true;
    }
}

bool Arcus::Private::PlatformSocket::close()
{
    int result = 0;
    #ifdef _WIN32
        result = ::closesocket(_socket_id);
    #else
        result = ::close(_socket_id);
    #endif

    return result == 0;
}

bool Arcus::Private::PlatformSocket::shutdown(PlatformSocket::ShutdownDirection direction)
{
    int flag = 0;
    switch(direction)
    {
        case ShutdownDirection::ShutdownRead:
        #ifdef _WIN32
            flag = SD_RECEIVE;
        #else
            flag = SHUT_RD;
        #endif
            break;
        case ShutdownDirection::ShutdownWrite:
        #ifdef _WIN32
            flag = SD_SEND;
        #else
            flag = SHUT_WR;
        #endif
            break;
        case ShutdownDirection::ShutdownBoth:
        #ifdef _WIN32
            flag = SD_BOTH;
        #else
            flag = SHUT_RDWR;
        #endif
    }

    return (::shutdown(_socket_id, flag) == 0);
}

void Arcus::Private::PlatformSocket::flush()
{
    char* buffer = new char[256];
    socket_size num = 0;

    while(num > 0)
    {
        num = ::recv(_socket_id, buffer, 256, MSG_DONTWAIT);
    }
}

socket_size Arcus::Private::PlatformSocket::writeUInt32(uint32_t data)
{
    uint32_t temp = htonl(data);
    socket_size sent_size = ::send(_socket_id, reinterpret_cast<const char*>(&temp), 4, MSG_NOSIGNAL);
    return sent_size;
}

socket_size Arcus::Private::PlatformSocket::writeBytes(std::size_t size, const char* data)
{
    return ::send(_socket_id, data, size, MSG_NOSIGNAL);
}

socket_size Arcus::Private::PlatformSocket::readUInt32(uint32_t* output)
{
    #ifndef _WIN32
        errno = 0;
    #endif

    uint32_t buffer;
    socket_size num = ::recv(_socket_id, reinterpret_cast<char*>(&buffer), 4, 0);

    if(num != 4)
    {
        #ifdef _WIN32
            if(num == WSAETIMEDOUT)
            {
                return 0;
            }
            else if(WSAGetLastError() == WSAETIMEDOUT)
            {
                return 0;
            }
        #else
            if(errno == EAGAIN)
            {
                return 0;
            }
        #endif

        return -1;
    }

    *output = ntohl(buffer);
    return num;
}

socket_size Arcus::Private::PlatformSocket::readBytes(std::size_t size, char* output)
{
    #ifndef _WIN32
        errno = 0;
    #endif

    socket_size num = ::recv(_socket_id, output, size, 0);

    #ifdef _WIN32
        if(num == SOCKET_ERROR && WSAGetLastError() == WSAETIMEDOUT)
        {
            return 0;
        }
    #else
        if(num <= 0 && errno == EAGAIN)
        {
            return 0;
        }
    #endif

    return num;
}

bool Arcus::Private::PlatformSocket::setReceiveTimeout(int timeout)
{
    int result = 0;
    #ifdef _WIN32
        result = ::setsockopt(_socket_id, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));
        return result != SOCKET_ERROR;
    #else
        timeval t;
        t.tv_sec = 0;
        t.tv_usec = timeout * 1000;
        result = ::setsockopt(_socket_id, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&t), sizeof(t));
        return result == 0;
    #endif
}

int Arcus::Private::PlatformSocket::getNativeErrorCode()
{
    #ifdef _WIN32
        return WSAGetLastError();
    #else
        return errno;
    #endif
}
