
#include <network/packet.h>

#include <assert.h>
#include <cstring>

using namespace network;
using std::memcpy;

Packet::Packet() :
	m_read(0)
{
}

void Packet::writeMessage(const NetworkMessage& message)
{
	assert(header.dataLength + sizeof(MessageType) + sizeof(int32_t) +
			   message.data.getLength() < g_maxPacketSize);

	int32_t sequenceNr = (message.isReliable) ? -message.sequenceNr : message.sequenceNr;
	int8_t  type = (message.isOrdered) ? -(int8_t)message.type : (int8_t)message.type;
	this->writeData(&type, sizeof(type)); // Write type, negative=ordered
	this->writeData(&sequenceNr, sizeof(sequenceNr)); // Write sequence, negative=reliable
	if(message.data.getLength() > 0)
	{
			int32_t size = static_cast<int32_t>(message.data.getLength());
			this->writeData(&size, sizeof(size)); // Write data size
			this->writeData(message.data.getBuffer(), message.data.getLength()); // Write data
	}
	else
	{
		int32_t size = 0;
		this->writeData(&size, sizeof(size));
	}

	header.messageCount++;
}

void Packet::writeData(const void* data, const size_t length)
{
	assert(data != nullptr);
	assert(length > 0 && length < 1500);

	memcpy(m_data + header.dataLength, data, length);
	header.dataLength += static_cast<uint16_t>(length);
}

IncomingMessage Packet::readMessage()
{
	IncomingMessage msg = {};
	memcpy(&msg.type, m_data+m_read, sizeof(MessageType));
	m_read += sizeof(MessageType);

	if ((int32_t)msg.type < 0)
	{
		msg.type = (MessageType)-(int32_t)msg.type;
		msg.isOrdered = true;
	}

	memcpy(&msg.sequence, m_data + m_read, sizeof(int32_t));
	m_read += sizeof(int32_t);

	int32_t dataSize;
	memcpy(&dataSize, m_data + m_read, sizeof(dataSize));
	m_read += sizeof(dataSize);

	if (dataSize > 0)
	{
		msg.data.writeData(reinterpret_cast<char*>(m_data + m_read), dataSize);
		m_read += dataSize;
	}

	return msg;
}

char* Packet::getData() const
{
	return (char*)m_data;
}
