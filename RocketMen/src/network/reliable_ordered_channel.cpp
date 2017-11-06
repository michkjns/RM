
#include "reliable_ordered_channel.h"

#include <common.h>
#include <core/debug.h>
#include <game_time.h>
#include <network/network_message.h>
#include <network/sequence_buffer.h>

using namespace network;

static const uint32_t s_packetWindowSize  = 1024;
static const uint32_t s_receiveQueueSize  = 1024;
static const uint32_t s_sendQueueSize     = 256;
static const float    s_messageResendTime = 0.1f;
static const float    s_keepAliveTime     = 1.f;

ReliableOrderedChannel::ReliableOrderedChannel() :
	m_sendMessageId(0),
	m_receiveMessageId(0),
	m_lastReceivedSequence(0),
	m_lastPacketSendTime(.0f),
	m_messageSendQueue(s_sendQueueSize),
	m_messageReceiveQueue(s_receiveQueueSize),
	m_sentPackets(s_packetWindowSize),
	m_receivedPackets(s_receiveQueueSize)
{
}

ReliableOrderedChannel::~ReliableOrderedChannel()
{
}

void ReliableOrderedChannel::sendMessage(Message& message)
{
	assert(canSendMessage());
	if (OutgoingMessage* messageEntry = m_messageSendQueue.insert(m_sendMessageId))
	{
		messageEntry->type         = message.type;
		messageEntry->data         = message.data;
		messageEntry->sequence     = m_sendMessageId;
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

		if (!packet->isEmpty())
		{
			writeAcks(packet);
			sendPacket(socket, address, packet);
			m_lastPacketSendTime = time.getSeconds();
		}

		delete packet;
	}
	else if (time.getSeconds() - m_lastPacketSendTime > s_keepAliveTime)
	{
		Message pingMessage = {};
		pingMessage.type = MessageType::KeepAlive;
		sendMessage(pingMessage);
	}
}

void ReliableOrderedChannel::receivePacket(Packet& packet)
{
	readAcks(packet.header);

	for (int32_t i = 0; i < packet.header.messageCount; i++)
	{
		IncomingMessage* message = packet.readNextMessage();

		IncomingMessage* messageEntry = m_messageReceiveQueue.insert(message->sequence);
		*messageEntry = *message;

		delete message;
	}
}

IncomingMessage* ReliableOrderedChannel::getNextMessage()
{
	if (IncomingMessage* message = m_messageReceiveQueue.getEntry(m_receiveMessageId))
	{
		assert(message->sequence == m_receiveMessageId);
		m_messageReceiveQueue.remove(m_receiveMessageId);
		m_receiveMessageId++;
		return message;
	}

	return nullptr;
}

float ReliableOrderedChannel::getLastPacketSendTime() const
{
	return m_lastPacketSendTime;
}

void ReliableOrderedChannel::writeAcks(Packet* packet)
{
	packet->header.ackSequence = m_lastReceivedSequence;

	for (Sequence i = 0; i < 32; i++)
	{
		Sequence sequence = m_lastReceivedSequence - i - 1;
		if (m_receivedPackets.exists(sequence))
		{
			packet->header.ackBits |= (1 << i);
		}
	}
}

void ReliableOrderedChannel::readAcks(const PacketHeader& packetHeader)
{
	m_receivedPackets.insert(packetHeader.sequence);

	if (sequenceGreaterThan(packetHeader.sequence, m_lastReceivedSequence))
	{
		m_lastReceivedSequence = packetHeader.sequence;
	}

	if (!m_sentPackets.exists(packetHeader.ackSequence) || 
		!sequenceLessThan(packetHeader.ackSequence, m_sentPackets.getCurrentSequence() + 1))
	{
		return;
	}

	for (Sequence i = 0; i < 33; i++)
	{
		if (packetHeader.ackBits & (1 << (i - 1)) || i == 0)
		{
			const Sequence ackSequence = packetHeader.ackSequence - i;
			if (SentPacketData* packetData = m_sentPackets.getEntry(ackSequence))
			{
				for (int16_t j = 0; j < packetData->numMessages; j++)
				{
					m_messageSendQueue.remove(packetData->messageIds[j]);
				}
				m_sentPackets.remove(ackSequence);
			}
		}
	}
}

bool ReliableOrderedChannel::hasMessagesToSend(const Time& time) const
{
	for (Sequence i = 0; i < s_messageQueueSize; i++)
	{
		if (m_messageSendQueue.exists(i))
		{
			OutgoingMessage* message = m_messageSendQueue.getAtIndex(i);
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
	SentPacketData* packetEntry = m_sentPackets.insert(packetSequence);
	assert(packetEntry != nullptr);
	packetEntry->numMessages = 0;

	Packet* packet = new Packet(ChannelType::ReliableOrdered);
	packet->header = {};
	packet->header.sequence = packetSequence;

	for (uint32_t i = 0; i < s_messageQueueSize; i++)
	{
		OutgoingMessage* message = m_messageSendQueue.getAtIndex(i);
		if (message != nullptr)
		{
			assert(message->type != MessageType::None);
			if (time.getSeconds() - message->timeLastSent >= s_messageResendTime)
			{
				packet->writeMessage(*message);
				packetEntry->messageIds[packetEntry->numMessages++] = message->sequence;
				if (getMessageChannel(message->type) == ChannelType::ReliableOrdered)
				{
					message->timeLastSent = time.getSeconds();
				}
				else
				{
					message->data.reset();
					m_messageSendQueue.remove(message->sequence);
				}
			}
		}

		if (packet->header.messageCount == g_maxMessagesPerPacket)
		{
			break;
		}
	}

	return packet;
}
