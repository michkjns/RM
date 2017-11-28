
#include <network/packet.h>
#include <core/debug.h>

#include <assert.h>
#include <cstring>

using namespace network;
using std::memcpy;

Packet::Packet() :
	m_channel(ChannelType::ReliableOrdered),
	m_error(false)
{
	header = {};
}

Packet::Packet(ChannelType channel) :
	m_read(0),
	m_channel(channel),
	m_error(false)
{
	header = {};
}

void Packet::writeMessage(const OutgoingMessage& message)
{
	assert(header.dataLength + g_messageOverhead +
		message.data.getLength() < g_maxBlockSize);

	assert(getMessageChannel(message) == m_channel);

	assert(message.type > MessageType::None 
		&& message.type < MessageType::NUM_MESSAGE_TYPES);
	
	writeData(&message.type, sizeof(MessageType));
	writeData(&message.sequence, sizeof(Sequence));
	m_messageIDs[header.numMessages] = message.sequence;

	const int32_t messageDataSize = static_cast<int32_t>(message.data.getLength());
	writeData(&messageDataSize, sizeof(messageDataSize));
	
	if(messageDataSize > 0)
	{
		writeData(message.data.getBuffer(), messageDataSize);
#ifdef _DEBUG
	//	LOG_DEBUG("WriteMessage: ID: %d, size: %d, %s", message.sequence, size, messageTypeAsString(message.type));
#endif
	}

	assert(header.dataLength < g_maxPacketSize);
	header.numMessages++;
}

void Packet::writeData(const void* data, const size_t length)
{
	assert(data != nullptr);
	assert(length > 0 && 
		header.dataLength + length < g_maxBlockSize);

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
		m_error = true;
		delete message;
		return nullptr;
	}

	memcpy(&message->sequence, m_data + m_read, sizeof(Sequence));
	m_read += sizeof(Sequence);

	int32_t messageDataSize;
	memcpy(&messageDataSize, m_data + m_read, sizeof(messageDataSize));
	m_read += sizeof(messageDataSize);

	if (messageDataSize > g_maxPacketSize)
	{
		m_error = true;
		delete message;
		return nullptr;
	}

	if (messageDataSize > 0)
	{
		message->data.writeData(reinterpret_cast<char*>(m_data + m_read), messageDataSize);
		m_read += messageDataSize;
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
	return header.numMessages == 0;
}

ChannelType Packet::getChannel() const
{
	return m_channel;
}

bool Packet::getError() const
{
	return m_error;
}
