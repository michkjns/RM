
#include "connection.h"

#include <core/debug.h>
#include <game/game_time.h>
#include <network/reliable_ordered_channel.h>
#include <network/socket.h>
#include <network/unreliable_channel.h>

using namespace network;

static const uint32_t s_maxConnectionAttemptDuration = 10;
static const float    s_timeout = 20.f;

Connection::Connection(Socket* socket, const Address& address, ConnectionCallbackMethod callback) :
	m_address(address),
	m_connectionAttempt(0),
	m_timeSinceLastPacketReceived(0.f),
	m_state(State::Disconnected),
	m_connectionAttemptDuration(0.f),
	m_unreliableChannel(new UnreliableChannel()),
	m_reliableOrderedChannel(new ReliableOrderedChannel()),
	m_connectionCallback(callback)
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

void Connection::sendMessage(Message& message)
{
	if (getMessageChannel(message) == ChannelType::ReliableOrdered)
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
	m_reliableOrderedChannel->sendPendingMessages(m_socket, m_address, time);
	m_unreliableChannel->sendPendingMessages(m_socket, m_address, time);
}

void Connection::receivePacket(Packet& packet)
{
	m_timeSinceLastPacketReceived = 0.f;

	if (packet.getChannel() == ChannelType::Unreliable)
	{
		m_unreliableChannel->receivePacket(packet);
	}
	else
	{
		assert(packet.getChannel() == ChannelType::ReliableOrdered);
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

IncomingMessage* Connection::getNextMessage()
{
	if (auto message = m_reliableOrderedChannel->getNextMessage())
	{
		message->address = m_address;
		return message;
	}

	if (auto message = m_unreliableChannel->getNextMessage())
	{
		message->address = m_address;
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

	Message message = {};
	message.type = MessageType::RequestConnection;
	sendMessage(message);
	setState(State::Connecting);
}

bool Connection::isClosed() const
{
	return m_state == State::Closed;
}
