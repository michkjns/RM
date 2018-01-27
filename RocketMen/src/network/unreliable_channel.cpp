
#include "unreliable_channel.h"

#include <core/debug.h>
#include <network/socket.h>

using namespace network;

static const uint32_t s_messageSendQueueSize = 64;
static const uint32_t s_messageReceiveQueueSize = 64;

UnreliableChannel::UnreliableChannel() : 
	m_nextMessageId(0),
	m_queuedSendMessages(0),
	m_sendQueue(s_messageSendQueueSize),
	m_receiveQueue(s_messageReceiveQueueSize)
{
	m_sendQueue.fill(nullptr);
}

UnreliableChannel::~UnreliableChannel()
{
	for (Message*& message : m_sendQueue)
	{
		if (message)
		{
			assert(message->releaseRef());
		}
	}

	for (Message*& message : m_receiveQueue)
	{
		if (message)
		{
			assert(message->releaseRef());
		}
	}
}

void UnreliableChannel::sendMessage(Message* message)
{
	assert(message->getChannel() == ChannelType::UnreliableUnordered);
	m_sendQueue.insert(message);
	m_queuedSendMessages++;
}

void UnreliableChannel::sendPendingMessages(Socket* socket, const Address& address, const Time& time, MessageFactory* messageFactory)
{
	if (hasMessagesToSend())
	{
		Packet* packet = createPacket(time);
		sendPacket(socket, address, packet, messageFactory);
		delete packet;
	}
}

void UnreliableChannel::receivePacket(Packet& packet)
{
	for (int32_t i = 0; i < packet.header.numMessages; i++)
	{

		m_receiveQueue.insert(packet.messages[i]->addRef());
	}
}

Message* UnreliableChannel::getNextMessage()
{
	for (Message*& messageEntry : m_receiveQueue)
	{
		if (messageEntry != nullptr)
		{
			Message* message = messageEntry;
			assert(message != nullptr);
			assert(message->getType() != MessageType::None);
			assert(message->getChannel() == ChannelType::UnreliableUnordered);

			messageEntry = nullptr;
			return message;
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
	Packet* packet             = new Packet();
	packet->header             = {};
	packet->header.ackBits     = (uint32_t)INDEX_NONE;
	packet->header.sequence    = (Sequence)INDEX_NONE;
	packet->header.ackSequence = (Sequence)INDEX_NONE;

	for (uint32_t i = 0; i < s_messageSendQueueSize; i++)
	{
		Message* message = m_sendQueue[i];
		if (message != nullptr)
		{
			assert(message->getChannel() == ChannelType::UnreliableUnordered);

			message->assignId(m_nextMessageId++);

			packet->messages[packet->header.numMessages] = message->addRef();
			packet->messageIds[packet->header.numMessages] = message->getId();
			packet->messageTypes[packet->header.numMessages] = message->getType();

			m_sendQueue[i] = nullptr;
			packet->header.numMessages++;
			m_queuedSendMessages--;
		}

		if (m_queuedSendMessages == 0 
			|| packet->header.numMessages == g_maxMessagesPerPacket)
		{
			break;
		}
	}

	return packet;
}
