
#include "unreliable_channel.h"

#include <core/debug.h>
#include <network/socket.h>

using namespace network;

UnreliableChannel::UnreliableChannel() : 
	m_nextMessageId(0),
	m_queuedSendMessages(0)
{
	Message emptyMessage = {};
	m_sendQueue.fill(emptyMessage);
}

UnreliableChannel::~UnreliableChannel()
{
}

void UnreliableChannel::sendMessage(Message& message)
{
	assert(getMessageChannel(message) == ChannelType::Unreliable);
	m_sendQueue.insert(message);
	m_queuedSendMessages++;
}

void UnreliableChannel::sendPendingMessages(Socket* socket, const Address& address, const Time& time)
{
	if (hasMessagesToSend())
	{
		Packet* packet = createPacket(time);
		sendPacket(socket, address, packet);
	}
}

void UnreliableChannel::receivePacket(Packet& packet)
{
	for (int32_t i = 0; i < packet.header.messageCount; i++)
	{
		IncomingMessage* message = packet.readNextMessage();
		m_receiveQueue.insert(*message);
	}
}

IncomingMessage* UnreliableChannel::getNextMessage()
{
	for (auto& message : m_receiveQueue)
	{
		if (message.type != MessageType::None)
		{
			return &message;
		}
	}
	return nullptr;
}

bool UnreliableChannel::hasMessagesToSend() const
{
	return m_queuedSendMessages > 0;
}

Packet* UnreliableChannel::createPacket(const Time& /*time*/)
{
	Packet* packet = new Packet(ChannelType::Unreliable);
	packet->header             = {};
	packet->header.ackBits     = (uint32_t)-1;
	packet->header.sequence    = (Sequence)-1;
	packet->header.ackSequence = (Sequence)-1;

	for (uint32_t i = 0; i < s_messageQueueSize; i++)
	{
		Message& message = m_sendQueue[i];
		if (message.type != MessageType::None)
		{
			assert(getMessageChannel(message) == ChannelType::Unreliable);
			OutgoingMessage outMessage;
			outMessage.type         = message.type;
			outMessage.data         = message.data;
			outMessage.sequence     = m_nextMessageId++;

			packet->writeMessage(outMessage);
			packet->writeData(&m_nextMessageId, sizeof(m_nextMessageId));

			m_queuedSendMessages--;
			m_sendQueue[i].type = MessageType::None;
		}

		if (packet->header.messageCount == g_maxMessagesPerPacket)
		{
			break;
		}
	}

	return packet;
}
