
#include "connection.h"

#include <core/debug.h>
#include <core/game_time.h>
#include <network/message_factory.h>
#include <network/reliable_ordered_channel.h>
#include <network/socket.h>
#include <network/unreliable_channel.h>

using namespace network;

static const uint32_t s_maxConnectionAttemptDuration = 10;
static const float    s_timeout = 20.f;

Connection::Connection(Socket* socket, const Address& address, ConnectionCallbackMethod callback, MessageFactory& messageFactory) :
	m_address(address),
	m_connectionAttempt(0),
	m_timeSinceLastPacketReceived(0.f),
	m_state(State::Disconnected),
	m_connectionAttemptDuration(0.f),
	m_unreliableChannel(new UnreliableChannel()),
	m_reliableOrderedChannel(new ReliableOrderedChannel()),
	m_connectionCallback(callback),
	m_messageFactory(messageFactory)
{
	assert(socket != nullptr);
	assert(socket->isInitialized());
	m_socket = socket;
}

Connection::~Connection()
{
	assert(m_state == State::Closed);
	
	delete m_unreliableChannel;
	delete m_reliableOrderedChannel;
}

void Connection::update(const Time& time)
{
	switch (m_state)
	{
	case State::Connected:
	{
		m_timeSinceLastPacketReceived += time.getDeltaSeconds();
		if (m_timeSinceLastPacketReceived > 2.f)
		{
			LOG_INFO("Connection: Connection lost, reconnecting..");
			setState(State::Connecting);
			m_connectionCallback(ConnectionCallback::ConnectionLost, this);
		}
		break;
	}
	case State::Connecting:
	{
		m_connectionAttemptDuration += time.getDeltaSeconds();
		if (m_connectionAttemptDuration > static_cast<float>(s_maxConnectionAttemptDuration))
		{
			LOG_INFO("Connection: Connection attempt failed after %d seconds", s_maxConnectionAttemptDuration);
			m_connectionCallback(ConnectionCallback::ConnectionFailed, this);
		}
		break;
	}
	case State::Closed:
	case State::Disconnected:
	{
		break;
	}
	}
}

void Connection::sendMessage(Message* message)
{
	if (message->getChannel() == ChannelType::ReliableOrdered)
	{
		m_reliableOrderedChannel->sendMessage(message);
	}
	else
	{
		m_unreliableChannel->sendMessage(message);
	}
}

void Connection::sendPendingMessages(const Time& time)
{
	m_reliableOrderedChannel->sendPendingMessages(m_socket, m_address, time, &m_messageFactory);
	m_unreliableChannel->sendPendingMessages(m_socket, m_address, time, &m_messageFactory);
}

void Connection::receivePacket(Packet& packet)
{
	m_timeSinceLastPacketReceived = 0.f;

	const ChannelType channel = (packet.header.sequence == (Sequence)INDEX_NONE
		&& packet.header.ackBits == (uint32_t)INDEX_NONE
		&& packet.header.ackSequence == (Sequence)INDEX_NONE) ?
		ChannelType::UnreliableUnordered :
		ChannelType::ReliableOrdered;

	if (channel == ChannelType::UnreliableUnordered)
	{
		m_unreliableChannel->receivePacket(packet);
	}
	else
	{
		assert(channel == ChannelType::ReliableOrdered);
		m_reliableOrderedChannel->receivePacket(packet);
	}

	if (m_state == State::Connecting)
	{
		setState(State::Connected);
	}
}

void Connection::close()
{
	setState(State::Closed);
}

Message* Connection::getNextMessage()
{
	if (auto message = m_reliableOrderedChannel->getNextMessage())
	{
		return message;
	}

	if (auto message = m_unreliableChannel->getNextMessage())
	{
		return message;
	}

	return nullptr;
}

const Address& Connection::getAddress() const
{
	return m_address;
}

void Connection::setState(State state)
{
	m_state = state;
	if (state == Connection::State::Connecting)
	{
		m_connectionAttemptDuration = 0.f;
	}
}

Connection::State Connection::getState() const
{
	return m_state;
}

void Connection::tryConnect()
{
	assert(m_state == State::Disconnected);

	Message* message = m_messageFactory.createMessage(MessageType::RequestConnection);
	sendMessage(message);

	setState(State::Connecting);
}

bool Connection::isClosed() const
{
	return m_state == State::Closed;
}
