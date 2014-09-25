#ifdef __WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif
#include <string.h>
#include <stdio.h>
#include "arc_message.h"

namespace arc
{

Message::Message()
{
	message_type = MSG_NO_MESSAGE;
	read_position = 0;
	is_valid = true;
}

Message::Message(EMessageType message_type)
: message_type(message_type)
{
	read_position = 0;
	is_valid = true;
}

Message::~Message()
{
}

//Check if its possible to extract the data.
bool Message::checkSize(std::size_t size)
{
    is_valid = is_valid && (read_position + size <= data.size());
    return is_valid;
}

EMessageType Message::getMessageType()
{
	return message_type;
}

void Message::setMessageType(EMessageType type)
{
	message_type = type;
}

void Message::append(const void* data, std::size_t size)
{
    size_t position = this->data.size();
    this->data.resize(position + size);
    memcpy(&this->data[position], data, size);
}

Message& Message::operator << (bool data)
{
	*this << static_cast<uint8_t>(data);
    return *this;
}

Message& Message::operator << (int8_t data)
{
	append(&data, sizeof(data));
    return *this;
}

Message& Message::operator << (int16_t data)
{
	int16_t to_write = htons(data);
    append(&to_write, sizeof(to_write));
    return *this;
}

Message& Message::operator << (int32_t data)
{
	int32_t to_write = htonl(data);
    append(&to_write, sizeof(to_write));
    return *this;
}

Message& Message::operator << (uint8_t data)
{
	append(&data, sizeof(data));
    return *this;
}

Message& Message::operator << (uint16_t data)
{
	uint16_t to_write = htons(data);
    append(&to_write, sizeof(to_write));
    return *this;
}

Message& Message::operator << (uint32_t data)
{
	uint32_t to_write = htonl(data);
    append(&to_write, sizeof(to_write));
    return *this;
}

Message& Message::operator << (float data)
{
	append(&data, sizeof(data));
   	return *this;
}

Message& Message::operator << (double data)
{
	append(&data, sizeof(data));
    return *this;
}

Message& Message::operator << (std::string data)
{
	// First insert string length
	uint32_t length = static_cast<uint32_t>(data.size());
	*this << length;

	// Then insert characters
	if (length > 0)
		append(data.c_str(), length * sizeof(std::string::value_type));

	return *this;
}

Message& Message::operator >> (bool& data)
{
	if (checkSize(sizeof(data)))
    {
		data = *reinterpret_cast<const int8_t*>(&this->data[read_position]);
		read_position += sizeof(data);
	}
	return *this;
}

Message& Message::operator >> (int8_t& data)
{
	if (checkSize(sizeof(data)))
    {
		data = *reinterpret_cast<const int8_t*>(&this->data[read_position]);
		read_position += sizeof(data);
	}
	return *this;
}

Message& Message::operator >> (int16_t& data)
{
	if (checkSize(sizeof(data)))
    {
		data = htons(*reinterpret_cast<const int16_t*>(&this->data[read_position]));
		read_position += sizeof(data);
	}
	return *this;
}

Message& Message::operator >> (int32_t& data)
{
	if (checkSize(sizeof(data)))
    {
		data = htonl(*reinterpret_cast<const int32_t*>(&this->data[read_position]));
		read_position += sizeof(data);
	}
	return *this;
}

Message& Message::operator >> (uint8_t& data)
{
	if (checkSize(sizeof(data)))
    {
		data = *reinterpret_cast<const uint8_t*>(&this->data[read_position]);
		read_position += sizeof(data);
	}
	return *this;
}

Message& Message::operator >> (uint16_t& data)
{
	if (checkSize(sizeof(data)))
    {
		data = htons(*reinterpret_cast<const uint16_t*>(&this->data[read_position]));
		read_position += sizeof(data);
	}
	return *this;
}

Message& Message::operator >> (uint32_t& data)
{
	if (checkSize(sizeof(data)))
    {
		data = htonl(*reinterpret_cast<const uint32_t*>(&this->data[read_position]));
		read_position += sizeof(data);
	}
	return *this;
}

Message& Message::operator >> (float& data)
{
	if (checkSize(sizeof(data)))
    {
		data = *reinterpret_cast<const float*>(&this->data[read_position]);
		read_position += sizeof(data);
	}
	return *this;
}

Message& Message::operator >> (double& data)
{
	if (checkSize(sizeof(data)))
    {
		data = *reinterpret_cast<const double*>(&this->data[read_position]);
		read_position += sizeof(data);
	}
	return *this;
}

Message& Message::operator >> (std::string& data)
{
	int32_t length = 0; //Extract length.
	*this >> length;

	data.clear();
	if ((length > 0) && checkSize(length))
	{
		data.assign((const char*)&this->data[read_position], length); //Extract characters.
		read_position += length;
	}

	return *this;
}

void Message::reserveRawData(size_t size)
{
	data.resize(size);
}

void* Message::getRawData()
{
	return data.data();
}

size_t Message::getRawDataSize()
{
	return data.size();
}

} //namespace ARC
