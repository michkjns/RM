
#include "reliable_ordered_channel.h"

#include <common.h>
#include <core/debug.h>
#include <core/game_time.h>
#include <network/message.h>
#include <network/message_factory.h>
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
	m_nextSendMessageId(0),
	m_nextReceiveMessageId(0),
	m_lastReceivedSequence(0),
	m_lastPacketSendTime(.0f),
	m_messageSendQueue(s_messageSendQueueSize),
	m_messageReceiveQueue(s_messageReceiveQueueSize),
	m_sentPackets(s_packetWindowSize),
	m_receivedPackets(s_packetReceiveQueueSize)
#ifdef _DEBUG 
	, m_numReceivedPackets(0),
	m_numReceivedMessages(0),
	m_numSentPackets(0),
	m_numSentMessages(0),
	m_numAcksReceived(0)
#endif
{
}

ReliableOrderedChannel::~ReliableOrderedChannel()
{
}

void ReliableOrderedChannel::sendMessage(Message* message)
{
	assert(canSendMessage());
	if (OutgoingMessageEntry* messageEntry = m_messageSendQueue.insert(m_nextSendMessageId))
	{
		message->assignId(m_nextSendMessageId);
		messageEntry->message = message;
		messageEntry->timeLastSent = -1.f;
		m_nextSendMessageId++;
#ifdef _DEBUG
		m_numSentMessages++;
#endif
	}
	else
	{
		LOG_ERROR("ReliableOrderedChannel::sendMessage: Failed to queue message");
		assert(false);
	}
}

void ReliableOrderedChannel::sendPendingMessages(Socket* socket, const Address& address, const Time& time, MessageFactory* messageFactory)
{
	if (hasMessagesToSend(time))
	{
		Packet* packet = createPacket(time);

		writeAcksToPacket(*packet);
		sendPacket(socket, address, packet, messageFactory);
		m_lastPacketSendTime = time.getSeconds();
		delete packet;

#ifdef _DEBUG
		m_numSentPackets++;
#endif
	}
	else if (time.getSeconds() - m_lastPacketSendTime > s_keepAliveTime)
	{
		sendMessage(messageFactory->createMessage(MessageType::KeepAlive));
	}

	m_receivedPackets.removeOldEntries();
}

void ReliableOrderedChannel::receivePacket(Packet& packet)
{
	readAcksFromPacket(packet);

	for (int32_t i = 0; i < packet.header.numMessages; i++)
	{
		if (IncomingMessageEntry* messageEntry = m_messageReceiveQueue.insert(packet.messageIds[i]))
		{
			messageEntry->message = packet.messages[i]->addRef();
		}
	}

	m_receivedPackets.insert(packet.header.sequence);

#ifdef _DEBUG
	m_numReceivedPackets++;
	m_numReceivedMessages += packet.header.numMessages;
#endif
}

Message* ReliableOrderedChannel::getNextMessage()
{
	if (IncomingMessageEntry* messageEntry = m_messageReceiveQueue.getEntry(m_nextReceiveMessageId))
	{
		Message* message = messageEntry->message;
		assert(message != nullptr);
		assert(message->getType() != MessageType::None);
		assert(message->getId() == m_nextReceiveMessageId);
		assert(message->getChannel() == ChannelType::ReliableOrdered);
		
		m_messageReceiveQueue.remove(m_nextReceiveMessageId);

		m_nextReceiveMessageId++;
		return message;
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
			const Sequence messageId = packetData->messageIds[j];
			if (OutgoingMessageEntry* messageEntry = m_messageSendQueue.getEntry(messageId))
			{
				messageEntry->message->releaseRef();
				messageEntry->message = nullptr;
				m_messageSendQueue.remove(messageId);
#ifdef _DEBUG
				m_numAcksReceived++;
#endif
			}
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
			OutgoingMessageEntry* messageEntry = m_messageSendQueue.getAtIndex(i);
			if (time.getSeconds() - messageEntry->timeLastSent >= s_messageResendTime)
			{
				return true;
			}
		}
	}
	return false;
}

bool ReliableOrderedChannel::canSendMessage() const
{
	return m_messageSendQueue.isAvailable(m_nextSendMessageId);
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
			Message* message = messageEntry->message;
			assert(message->getType() != MessageType::None);
			assert(message->getChannel() == ChannelType::ReliableOrdered);

			if (time.getSeconds() - messageEntry->timeLastSent >= s_messageResendTime)
			{
				const int32_t currentIndex = packet->header.numMessages;
				packet->messages[currentIndex] = message->addRef();
				packet->messageIds[currentIndex] = message->getId();
				packet->messageTypes[currentIndex] = message->getType();

				packetEntry->messageIds[currentIndex] = message->getId();
			
				packet->header.numMessages++;
				packetEntry->numMessages = (int16_t)packet->header.numMessages;
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
