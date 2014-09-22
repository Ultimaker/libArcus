#ifndef LIB_ARCUS_CONNECTION_H
#define LIB_ARCUS_CONNECTION_H

#include "arc_message.h"

//Pure virtual base class for any connection

namespace arc
{
	class Connection
	{
	private:
	public:
		virtual ~Connection() {}
		
		virtual bool sendMessage(Message& message) = 0;
		virtual Message recieveMessage() = 0;
		
		virtual bool isActive() = 0;
	};
}

#endif//LIB_ARCUS_CONNECTION_H
