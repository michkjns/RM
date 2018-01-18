
#include "reliable_ordered_channel.h"

#include <common.h>
#include <core/debug.h>
#include <core/game_time.h>
#include <network/network_message.h>
#include <network/sequence_buffer.h>

using namespace network;

static const uint32_t s_packetWindowSize        = 1024;
static const uint32_t s_packetReceiveQueueSize  = 1024;
static const uint32_t s_packetSendQueueSize     = 256;
static const uint32_t s_messageSendQueueSize    = 1024;
static const uint32_t s_messageReceiveQueueSize = 256;
static const float    s_messageResendTime       = 0.1f;
static const float    s_keepAliveTime           = 1.f;

ReliableOrderedChannel::ReliableOrderedChannel() :
	m_sendMessageId(0),
	m_receiveMessageId(0),
	m_lastReceivedSequence(0),
	m_lastPacketSendTime(.0f),
	m_messageSendQueue(s_messageSendQueueSize),
	m_messageReceiveQueue(s_messageReceiveQueueSize),
	m_sentPackets(s_packetWindowSize),
	m_receivedPackets(s_packetReceiveQueueSize)
{
}

ReliableOrderedChannel::~ReliableOrderedChannel()
{
}

void ReliableOrderedChannel::sendMessage(Message* message)
{
	assert(canSendMessage());
	if (OutgoingMessageEntry* messageEntry = m_messageSendQueue.insert(m_sendMessageId))
	{
		message->data.flush();

		message->id = m_sendMessageId;
		messageEntry->message = message;
		messageEntry->timeLastSent = -1.f;
		m_sendMessageId++;
	}
	else
	{
		LOG_ERROR("ReliableOrderedChannel::sendMessage: Failed to queue message");
		assert(false);
	}
}

void ReliableOrderedChannel::sendPendingMessages(Socket* socket, const Address& address, const Time& time)
{
	if (hasMessagesToSend(time))
	{
		Packet* packet = createPacket(time);

		writeAcksToPacket(*packet);
		sendPacket(socket, address, packet);
		m_lastPacketSendTime = time.getSeconds();

		delete packet;
	}
	else if (time.getSeconds() - m_lastPacketSendTime > s_keepAliveTime)
	{
		sendMessage(new Message(MessageType::KeepAlive));
	}

	m_receivedPackets.removeOldEntries();
}

void ReliableOrderedChannel::receivePacket(Packet& packet)
{
	readAcksFromPacket(packet);

	for (int32_t i = 0; i < packet.header.numMessages; i++)
	{
		IncomingMessage* message = new IncomingMessage(packet.messages[i]->type,
			packet.messageIds[i],
			packet.messages[i]->data.getData(), 
			packet.messages[i]->data.getBufferSize());

		packet.messages[i]->data.release();
		delete packet.messages[i];

		if (IncomingMessageEntry* messageEntry = m_messageReceiveQueue.insert(message->id))
		{
			messageEntry->message = message;
		}
	}

	m_receivedPackets.insert(packet.header.sequence);
}

IncomingMessage* ReliableOrderedChannel::getNextMessage()
{
	if (IncomingMessageEntry* messageEntry = m_messageReceiveQueue.getEntry(m_receiveMessageId))
	{
		if (IncomingMessage* message = messageEntry->message)
		{
			if (message->type != MessageType::None)
			{
				assert(message->id == m_receiveMessageId);
				assert(getMessageChannel(message->type) == ChannelType::ReliableOrdered);
				m_receiveMessageId++;
				return message;
			}
			else
			{
				m_messageReceiveQueue.remove(message->id);
				messageEntry->message = nullptr;
				delete message;
			}
		}
	}

	return nullptr;
}

float ReliableOrderedChannel::getLastPacketSendTime() const
{
	return m_lastPacketSendTime;
}

void ReliableOrderedChannel::writeAcksToPacket(Packet& packet)
{
	packet.header.ackSequence = m_lastReceivedSequence;

	for (Sequence i = 0; i < 32; i++)
	{
		Sequence sequence = m_lastReceivedSequence - 1 - i ;

		if (m_receivedPackets.exists(sequence))
		{
			packet.header.ackBits |= (1 << i);
		}
	}
}

void ReliableOrderedChannel::readAcksFromPacket(const Packet& packet)
{
	if (sequenceGreaterThan(packet.header.sequence, m_lastReceivedSequence))
	{
		m_lastReceivedSequence = packet.header.sequence;
	}

	if (!sequenceLessThan(packet.header.ackSequence, m_sentPackets.getCurrentSequence() + 1))
	{
		return;
	}

	for (Sequence i = 0; i < 32; i++)
	{
		if (packet.header.ackBits & (1 << i))
		{
			ack(packet.header.ackSequence - i - 1);
		}
	}

	ack(packet.header.ackSequence);
}

void ReliableOrderedChannel::ack(Sequence ackSequence)
{
	if (SentPacketEntry* packetData = m_sentPackets.getEntry(ackSequence))
	{
		for (int16_t j = 0; j < packetData->numMessages; j++)
		{
			if (OutgoingMessageEntry* messageEntry = m_messageSendQueue.getEntry(packetData->messageIds[j]))
			{
				delete messageEntry->message;
				messageEntry->message = nullptr;
			}
			m_messageSendQueue.remove(packetData->messageIds[j]);
		}
		m_sentPackets.remove(ackSequence);
	}
}

bool ReliableOrderedChannel::hasMessagesToSend(const Time& time) const
{
	for (Sequence i = 0; i < s_messageSendQueueSize; i++)
	{
		if (m_messageSendQueue.exists(i))
		{
			OutgoingMessageEntry* message = m_messageSendQueue.getAtIndex(i);
			if (time.getSeconds() - message->timeLastSent >= s_messageResendTime)
			{
				return true;
			}
		}
	}
	return false;
}

bool ReliableOrderedChannel::canSendMessage() const
{
	return m_messageSendQueue.isAvailable(m_sendMessageId);
}

Packet* ReliableOrderedChannel::createPacket(const Time& time)
{
	const Sequence packetSequence = m_sentPackets.getCurrentSequence();
	SentPacketEntry* packetEntry = m_sentPackets.insert(packetSequence);
	assert(packetEntry != nullptr);
	packetEntry->numMessages = 0;

	Packet* packet = new Packet();
	packet->header = {};
	packet->header.sequence = packetSequence;

	for (uint32_t i = 0; i < s_messageSendQueueSize; i++)
	{
		if (OutgoingMessageEntry* messageEntry = m_messageSendQueue.getAtIndex(i))
		{
			assert(messageEntry->message->type != MessageType::None);
			assert(getMessageChannel(messageEntry->message->type) == ChannelType::ReliableOrdered);
			if (time.getSeconds() - messageEntry->timeLastSent >= s_messageResendTime)
			{
				packet->messageIds[packet->header.numMessages] = messageEntry->message->id;
				packet->messages[packet->header.numMessages] = messageEntry->message;
				packet->header.numMessages++;

				packetEntry->messageIds[packetEntry->numMessages++] = messageEntry->message->id;
			
				messageEntry->timeLastSent = time.getSeconds();
			}
		}

		if (packet->header.numMessages == g_maxMessagesPerPacket)
		{
			break;
		}
	}

	return packet;
}
