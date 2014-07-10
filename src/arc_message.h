#ifndef LIB_ARCUS_MESSAGE_H
#define LIB_ARCUS_MESSAGE_H

#include <vector>
#include <string>
#include <stdint.h>

#include "arc_message_types.h"

namespace arc
{
	class Message
	{
	private:
		ECommand command;
		std::vector<uint8_t> data;
		std::size_t read_position;

	public:
		Message();
		Message(ECommand command);
		~Message();
		
		ECommand getCommand();
		
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
	};
}

#endif//LIB_ARCUS_MESSAGE_H
