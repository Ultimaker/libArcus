#ifdef __WIN32
#include <winsock2.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include "arc_tcpsocket_connection.h"

namespace arc
{
#ifdef __WIN32
static bool wsaStartupDone = false;
#endif

TcpSocketConnection::TcpSocketConnection(int listen_port_nr)
{
#ifdef __WIN32
    if (!wsaStartupDone)
    {
        WSADATA wsaData;
        memset(&wsaData, 0, sizeof(WSADATA));
        //WSAStartup needs to be called on windows before sockets can be used. Request version 1.1, which is supported on windows 98 and higher.
        WSAStartup(MAKEWORD(1, 1), &wsaData);
        wsaStartupDone = true;
    }
#endif
	socket_fd = -1;
}

TcpSocketConnection::TcpSocketConnection(const char* host, int port_nr);
{
#ifdef __WIN32
    if (!wsaStartupDone)
    {
        WSADATA wsaData;
        memset(&wsaData, 0, sizeof(WSADATA));
        //WSAStartup needs to be called on windows before sockets can be used. Request version 1.1, which is supported on windows 98 and higher.
        WSAStartup(MAKEWORD(1, 1), &wsaData);
        wsaStartupDone = true;
    }
#endif

    struct sockaddr_in serv_addr;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    memset(&serv_addr, '0', sizeof(serv_addr)); 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = inet_addr(host.c_str());
    // TODO: Check this: C style cast replaced by reinterpret_cast!!
    if (connect(socket_fd, reinterpret_cast<struct sockaddr*>(&serv_addr), sizeof(serv_addr)) < 0)
    {
		closeSocket();
        return;
    }
}

TcpSocketConnection::~TcpSocketConnection()
{
	closeSocket();
}

bool sendMessage(Message& message)
{
}

Message& recieveMessage()
{
}

bool TcpSocketConnection::isActive()
{
	return socket_fd != -1;
}

TcpSocketConnection::closeSocket()
{
    if (socket_fd == -1)
        return;
#ifdef __WIN32
    closesocket(socket_fd);
#else
    ::close(socket_fd);
#endif
	socket_fd = -1;
}

}

#endif//LIB_ARCUS_TCP_SOCKET_CONNECTION_H
