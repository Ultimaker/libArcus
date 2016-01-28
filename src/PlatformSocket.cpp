/*
 * This file is part of libArcus
 *
 * Copyright (C) 2016 Ultimaker b.v. <a.hiemstra@ultimaker.com>
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
#endif

#ifndef MSG_NOSIGNAL
    #define MSG_NOSIGNAL 0x0 //Don't request NOSIGNAL on systems where this is not implemented.
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

bool Arcus::Private::PlatformSocket::shutdown()
{
    int result = 0;
    #ifdef _WIN32
        result = ::shutdown(_socket_id, SD_BOTH);
    #else
        result = ::shutdown(_socket_id, SHUT_RDWR);
    #endif
    return result == 0;
}

void Arcus::Private::PlatformSocket::flush()
{
    char* buffer = new char[256];
    std::size_t num = 0;

    while(num > 0)
    {
        num = ::recv(_socket_id, buffer, 256, MSG_DONTWAIT);
    }
}

int Arcus::Private::PlatformSocket::writeInt32(int32_t data)
{
    uint32_t temp = htonl(data);
    int sent_size = ::send(_socket_id, reinterpret_cast<const char*>(&temp), 4, MSG_NOSIGNAL);
    return sent_size;
}

int Arcus::Private::PlatformSocket::writeBytes(std::size_t size, const char* data)
{
    return ::send(_socket_id, data, size, MSG_NOSIGNAL);
}

int Arcus::Private::PlatformSocket::readInt32(int32_t* output)
{
    int32_t buffer;
    std::size_t num = ::recv(_socket_id, reinterpret_cast<char*>(&buffer), 4, 0);

    if(num != 4)
    {
        return -1;
    }

    *output = ntohl(buffer);
    return num;
}

int Arcus::Private::PlatformSocket::readBytes(std::size_t size, char* output)
{
    return ::recv(_socket_id, output, size, 0);
}

void Arcus::Private::PlatformSocket::setReceiveTimeout(int timeout)
{
    timeval t;
    t.tv_sec = 0;
    t.tv_usec = timeout * 1000;
    ::setsockopt(_socket_id, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&t), sizeof(t));
}
