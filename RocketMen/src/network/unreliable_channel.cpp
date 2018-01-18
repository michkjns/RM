
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
	Message emptyMessage(MessageType::None);
	m_sendQueue.fill(nullptr);
}

UnreliableChannel::~UnreliableChannel()
{
}

void UnreliableChannel::sendMessage(Message* message)
{
	assert(getMessageChannel(message) == ChannelType::Unreliable);
	message->data.flush();
	m_sendQueue.insert(message);
	m_queuedSendMessages++;
}

void UnreliableChannel::sendPendingMessages(Socket* socket, const Address& address, const Time& time)
{
	if (hasMessagesToSend())
	{
		Packet* packet = createPacket(time);
		sendPacket(socket, address, packet);
		delete packet;
	}
}

void UnreliableChannel::receivePacket(Packet& packet)
{
	for (int32_t i = 0; i < packet.header.numMessages; i++)
	{
		IncomingMessage* message = new IncomingMessage(packet.messages[i]->type, 
			packet.messageIds[i],
			packet.messages[i]->data.getData(),
			packet.messages[i]->data.getBufferSize());

		packet.messages[i]->data.release();
		delete packet.messages[i];

		m_receiveQueue.insert(message);
	}
}

IncomingMessage* UnreliableChannel::getNextMessage()
{
	for (IncomingMessage*& message : m_receiveQueue)
	{
		if (message != nullptr)
		{
			if (message->type != MessageType::None)
			{
				return message;
			}
			else
			{
				delete message;
				message = nullptr;
			}
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
			assert(getMessageChannel(message) == ChannelType::Unreliable);
			message->id = m_nextMessageId++;
			packet->messages[packet->header.numMessages++] = message;

			m_sendQueue[i] = nullptr;
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
