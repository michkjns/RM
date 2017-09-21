
#include <network/packet.h>
#include <core/debug.h>

#include <assert.h>
#include <cstring>

using namespace network;
using std::memcpy;

Packet::Packet() :
	m_read(0)
{
	header = {};
}

void Packet::writeMessage(const OutgoingMessage& message)
{
	assert(header.dataLength + sizeof(MessageType) + sizeof(int32_t) +
			   message.data.getLength() < g_maxPacketSize);

	int8_t type = (message.isOrdered) ? -(int8_t)message.type : (int8_t)message.type;
	writeData(&type, sizeof(type)); // negative=ordered

	if (message.isOrdered)
	{
		writeData(&message.sequence, sizeof(message.sequence));
	}

	if(message.data.getLength() > 0)
	{
		int32_t size = static_cast<int32_t>(message.data.getLength());
		writeData(&size, sizeof(size));
		writeData(message.data.getBuffer(), message.data.getLength()); 

#ifdef _DEBUG
	//	LOG_DEBUG("WriteMessage: ID: %d, size: %d, %s", message.sequence, size, messageTypeAsString(message.type));
#endif
	}
	else
	{
		int32_t size = 0;
		writeData(&size, sizeof(size));
#ifdef _DEBUG
	//	LOG_DEBUG("WriteMessage: ID: %d, size: %d, %s", message.sequence, size, messageTypeAsString(message.type));
#endif
	}

	m_messageIDs[header.messageCount] = message.sequence;
	header.messageCount++;
}

void Packet::writeData(const void* data, const size_t length)
{
	assert(data != nullptr);
	assert(length > 0 && length < 1500);

	memcpy(m_data + header.dataLength, data, length);
	header.dataLength += static_cast<uint16_t>(length);
}

IncomingMessage Packet::readNextMessage()
{
	IncomingMessage message = {};
	int8_t messageType = 0;
	memcpy(&messageType, m_data + m_read, sizeof(int8_t));
	m_read += sizeof(MessageType);
	
	if (messageType < 0)
	{
		message.type = (MessageType)-messageType;
		message.isOrdered = true;
	}
	else
	{
		message.type = (MessageType)messageType;
	}

	if (message.isOrdered)
	{
		memcpy(&message.sequence, m_data + m_read, sizeof(message.sequence));
		m_read += sizeof(message.sequence);
	}


	int32_t dataSize;
	memcpy(&dataSize, m_data + m_read, sizeof(dataSize));
	m_read += sizeof(dataSize);

#ifdef _DEBUG
	//LOG_DEBUG("ReadNextMessage: ID: %d, Size: %d, %s", message.sequence, dataSize, messageTypeAsString(message.type));
#endif

	if (dataSize > 0)
	{
		message.data.writeData(reinterpret_cast<char*>(m_data + m_read), dataSize);
		m_read += dataSize;
	}

	return message;
}

char* Packet::getData() const
{
	return (char*)m_data;
}

bool network::Packet::isEmpty() const
{
	return header.messageCount == 0;
}
