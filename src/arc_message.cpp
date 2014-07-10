#include "arc_message.h"

namespace arc
{

Message::Message()
{
	messageType = MSG_NO_MESSAGE;
	read_position = 0;
}

Message::Message(EMessageType message_type)
: message_type(message_type)
{
	read_position = 0;
}

Message::~Message()
{
}

EMessageType Message::getMessageType()
{
	return message_type;
}

Message& Message::operator << (bool data)
{
}

Message& Message::operator << (int8_t data)
{
}

Message& Message::operator << (int16_t data)
{
}

Message& Message::operator << (int32_t data)
{
}

Message& Message::operator << (float data)
{
}

Message& Message::operator << (double data)
{
}

Message& Message::operator << (std::string data)
{
}

Message& Message::operator >> (bool& data)
{
}

Message& Message::operator >> (int8_t& data)
{
}

Message& Message::operator >> (int16_t& data)
{
}

Message& Message::operator >> (int32_t& data)
{
}

Message& Message::operator >> (float& data)
{
}

Message& Message::operator >> (double& data)
{
}

Message& Message::operator >> (std::string& data)
{
}

void Message::reserveRawData(size_t size)
{
	data.reserve(size);
}

void* Message::getRawData()
{
	return data.data();
}

size_t Message::getRawDataSize()
{
	return data.size();
}

}

#endif//LIB_ARCUS_MESSAGE_H
