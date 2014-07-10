#ifndef LIB_ARCUS_MESSAGE_H
#define LIB_ARCUS_MESSAGE_H

#include <vector>
#include <string>
#include <stdint.h>

#include "arc_message_types.h"

// Container for a single message. Each message has a type (see arc_message_types.h for more details), a blob of data and a length. 
// The operators allow the message to be filled and read.

namespace arc
{
	class Message
	{
	private:
		EMessageType message_type;
		std::vector<uint8_t> data;
		std::size_t read_position;

	public:
		Message();
		Message(EMessageType message_type);
		~Message();
		
		EMessageType getMessageType();
		
		Message& operator << (bool data);
		Message& operator << (int8_t data);
		Message& operator << (int16_t data);
		Message& operator << (int32_t data);
		Message& operator << (float data);
		Message& operator << (double data);
		Message& operator << (std::string data);

		Message& operator >> (bool& data);
		Message& operator >> (int8_t& data);
		Message& operator >> (int16_t& data);
		Message& operator >> (int32_t& data);
		Message& operator >> (float& data);
		Message& operator >> (double& data);
		Message& operator >> (std::string& data);
		
		void reserveRawData(size_t size);
		void* getRawData();
		size_t getRawDataSize();
	};
}

#endif//LIB_ARCUS_MESSAGE_H
