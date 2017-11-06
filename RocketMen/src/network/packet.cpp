
#include <network/packet.h>
#include <core/debug.h>

#include <assert.h>
#include <cstring>

using namespace network;
using std::memcpy;

Packet::Packet(ChannelType channel) :
	m_read(0),
	m_channel(channel)
{
	header = {};
}

void Packet::writeMessage(const OutgoingMessage& message)
{
	assert(header.dataLength + sizeof(MessageType) + sizeof(int32_t) +
			   message.data.getLength() < g_maxPacketSize);

	assert(getMessageChannel(message) == m_channel);

	writeData(&message.type, sizeof(MessageType));

	writeData(&message.sequence, sizeof(Sequence));
	m_messageIDs[header.messageCount] = message.sequence;

	if(message.data.getLength() > 0)
	{
		const int32_t size = static_cast<int32_t>(message.data.getLength());
		assert(size < g_maxPacketSize);
		writeData(&size, sizeof(size));
		writeData(message.data.getBuffer(), size);

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

	header.messageCount++;
}

void Packet::writeData(const void* data, const size_t length)
{
	assert(data != nullptr);
	assert(length > 0 && length < 1500);

	memcpy(m_data + header.dataLength, data, length);
	header.dataLength += static_cast<uint16_t>(length);
}

IncomingMessage* Packet::readNextMessage()
{
	IncomingMessage* message = new IncomingMessage();

	memcpy(&message->type, m_data + m_read, sizeof(MessageType));
	m_read += sizeof(MessageType);
	
	if (!ensure(message->type > MessageType::None && message->type < MessageType::NUM_MESSAGE_TYPES))
	{
		LOG_WARNING("Packet: Invalid message type found");
		return nullptr;
	}

	memcpy(&message->sequence, m_data + m_read, sizeof(Sequence));
	m_read += sizeof(Sequence);

	int32_t dataSize;
	memcpy(&dataSize, m_data + m_read, sizeof(dataSize));
	assert(dataSize < g_maxPacketSize);
	m_read += sizeof(dataSize);
	if (dataSize > 0)
	{
		message->data.writeData(reinterpret_cast<char*>(m_data + m_read), dataSize);
		m_read += dataSize;
	}

	return message;
}

void Packet::resetReading()
{
	m_read = 0;
}

char* Packet::getData() const
{
	return (char*)m_data;
}

bool network::Packet::isEmpty() const
{
	return header.messageCount == 0;
}

ChannelType Packet::getChannel() const
{
	return m_channel;
}
