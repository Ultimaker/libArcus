#ifdef __WIN32
#include <winsock2.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include <string.h>

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

TcpSocketConnection::TcpSocketConnection(const char* host, int port_nr)
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
	serv_addr.sin_port = htons(port_nr);
	serv_addr.sin_addr.s_addr = inet_addr(host);
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

bool TcpSocketConnection::sendMessage(Message& message)
{
	int32_t command = htonl(message.getMessageType()); //endian conversion
	int32_t data_size = htonl(message.getRawDataSize()); //endian conversion
	if (send(socket_fd, &command, sizeof(int32_t), 0) <= 0)
	{
		closeSocket();
		return false;
	}
	if (send(socket_fd, &data_size, sizeof(int32_t), 0) <= 0)
	{
		closeSocket();
		return false;
	}
	if (send(socket_fd, message.getRawData(), message.getRawDataSize(), 0) <= 0)
	{
		closeSocket();
		return false;
	}
	return true;
}

Message TcpSocketConnection::recieveMessage()
{
	int32_t command;
	int32_t size;
	if (!recv(&command, sizeof(int32_t)))
		return Message(MSG_ERROR_RECV_FAILED);
	if (!recv(&size, sizeof(int32_t)))
		return Message(MSG_ERROR_RECV_FAILED);
	command = ntohl(command); //endian conversion
	size = ntohl(size); //endian conversion
	
	EMessageType message_type = EMessageType(command);
	Message message(message_type);
	message.reserveRawData(size);
	if (!recv(message.getRawData(), size))
		return Message(MSG_ERROR_RECV_FAILED);
	return message;
}

bool TcpSocketConnection::isActive()
{
	return socket_fd != -1;
}

bool TcpSocketConnection::recv(void* data, int length)
{
    if (socket_fd == -1)
        return false;
    char* ptr = static_cast<char*>(data);
    while(length > 0)
    {
        int n = ::recv(socket_fd, ptr, length, 0);
        if (n <= 0)
        {
            closeSocket();
            return false;
        }
        ptr += n;
        length -= n;
    }
	return true;
}

void TcpSocketConnection::closeSocket()
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
