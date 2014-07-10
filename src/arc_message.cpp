#include "arc_message.h"

namespace arc
{

Message::Message()
{
	messageType = MSG_NO_MESSAGE;
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
	if (checkSize(sizeof(data)))
    	{
		data = *reinterpret_cast<const Int8*>(&this->data[read_position]);
		read_position += sizeof(data);
	}
	return *this;
}

Message& Message::operator >> (int16_t& data)
{
	if (checkSize(sizeof(data)))
    	{
		data = *reinterpret_cast<const Int16*>(&this->data[read_position]);
		read_position += sizeof(data);
	}
	return *this;
}

Message& Message::operator >> (int32_t& data)
{
	if (checkSize(sizeof(data)))
    	{
		data = *reinterpret_cast<const Int32*>(&this->data[read_position]);
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
	Uint32 length = 0; //Extract length.
	*this >> length;

	data.clear();
	if ((length > 0) && checkSize(length))
	{
		data.assign(&this->data[read_position], length); //Extract characters.
		read_position += length;
	}

	return *this;
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

} //namespace ARC

#endif//LIB_ARCUS_MESSAGE_H
