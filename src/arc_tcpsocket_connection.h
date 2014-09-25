#ifndef LIB_ARCUS_TCP_SOCKET_CONNECTION_H
#define LIB_ARCUS_TCP_SOCKET_CONNECTION_H

#include "arc_connection.h"

//Socket connection that sends/recieves messages. See arc_message_types.h and arc_message.h for more details on how to format the data.

namespace arc
{
	class TcpSocketConnection : public Connection
	{
	private:
		static const int default_port_nr = 9001; // What does the scouter say?

        int listen_socket_fd;
		int socket_fd;
	public:
		TcpSocketConnection(int listen_port_nr = default_port_nr);
		TcpSocketConnection(const char* host, int port_nr = default_port_nr);
		virtual ~TcpSocketConnection();

		virtual bool sendMessage(Message& message);
		virtual Message recieveMessage();
		
		virtual bool isActive();
	
	protected:
		void closeSocket();
		void closeListenSocket();
		bool recv(void* data, int length);
	};
}

#endif//LIB_ARCUS_TCP_SOCKET_CONNECTION_H
