#ifndef LIB_ARCUS_TCP_SOCKET_CONNECTION_H
#define LIB_ARCUS_TCP_SOCKET_CONNECTION_H

#include "arc_connection.h"

namespace arc
{
	class TcpSocketConnection : public Connection
	{
	private:
		static const int default_port_nr 9001; // What does the scouter say?

		int socket_fd;
	public:
		TcpSocketConnection(int listen_port_nr = default_port_nr);
		TcpSocketConnection(const char* host, int port_nr = default_port_nr);
		virtual ~TcpSocketConnection();

		virtual bool sendMessage(Message& message) = 0;
		virtual Message& recieveMessage() = 0;
		
		virtual bool isActive() = 0;
	
	protected:
		void closeSocket();
		bool recv(void* data, int length);
	};
}

#endif//LIB_ARCUS_TCP_SOCKET_CONNECTION_H
