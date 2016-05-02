
#include "client.h"

#include "address.h"
#include "debug.h"
#include "game_time.h"

#include <assert.h>

using namespace network;

static const uint32_t s_maxConnectionAttempts = 3;
static const uint32_t s_connectionRetryTime   = 5;
//==============================================================================

Client::Client(Time& time) :
	m_gameTime(time),
	m_lastReceivedState(0),
	m_lastOrderedMessaged(0),
	m_state(State::STATE_DISCONNECTED),
	m_stateTimer(0.0f),
	m_messageSentTime(0.0f),
	m_maxMessageSentTime(0.02f),
	m_connectionAttempt(0),
	m_isInitialized(false)
{
	m_session = {};
}

Client::~Client()
{
}

bool Client::initialize()
{
	m_isInitialized = true;
	return true;
}

void Client::update()
{
	const float deltaTime = m_gameTime.getDeltaSeconds();

	processIncomingMessages(deltaTime);
	processOutgoingMessages(deltaTime);

	m_stateTimer += deltaTime;
	switch (m_state)
	{
		case State::STATE_CONNECTING:
		{
			if (!m_networkInterface.isConnecting())
			{
				setState(State::STATE_DISCONNECTED);
				break;
			}

			if (static_cast<uint32_t>(m_stateTimer) >= m_connectionAttempt * s_connectionRetryTime)
			{
				if (m_connectionAttempt < s_maxConnectionAttempts)
				{
					connect(m_session.serverAddress);
				}
				else
				{
					LOG_INFO("Connecting to %s timed out after %i attempts\n",
							 m_session.serverAddress.toString().c_str(), m_connectionAttempt);
					m_connectionAttempt = 0;
					m_session.serverAddress = Address();
					setState(State::STATE_DISCONNECTED);
				}
			}
			break;
		}
		default: break;
	}
}

void Client::fixedUpdate()
{
}

void Client::connect(const Address& address)
{
	m_connectionAttempt++;
	LOG_INFO("Client: Attempting to connect to %s, attempt %i", address.toString().c_str(),
			 m_connectionAttempt);

	m_session.serverAddress = address;
	m_networkInterface.connect(address, m_gameTime);

	if (m_state != State::STATE_CONNECTING)
	{
		setState(State::STATE_CONNECTING);
	}
}

void Client::queueMessage(const NetworkMessage& message)
{
	for (NetworkMessage& msg : m_session.messageBuffer)
	{
		if (msg.type == MessageType::MESSAGE_CLEAR)
		{
			msg.type       = message.type;
			msg.data       = message.data;
			msg.isReliable = message.isReliable;
			msg.sequenceNr = ++m_session.sequenceCounter;
			return;
		}
	}

	LOG_WARNING("Message Queue is full");
}

uint32_t Client::getNumLocalPlayers() const
{
	uint32_t count = 0;
	for (auto player : m_localPlayers)
	{
		if (player.isUsed) count++;
	}

	return count;
}

void Client::onHandshake(const IncomingMessage& msg)
{
	assert(msg.data != nullptr);
	if (m_state != State::STATE_CONNECTING)
		return;
	LOG_INFO("Client: Received handshake from the server! I have received ID ");
	//========================
	
	// Read my ID
	int32_t myID = msg.data->readInt32();
	LOG_INFO("%d\n", myID);
	setState(State::STATE_CONNECTED);

	// Introduce Players
	NetworkMessage outMessage = {};
	outMessage.type = MessageType::PLAYER_INTRO;
	outMessage.data = BitStream::create();
	outMessage.data->writeInt32(getNumLocalPlayers());
	outMessage.isReliable = true;
	
	//for (auto player : m_localPlayers)
	//{
	//	// Write player info
	//}

	queueMessage(outMessage);
}

void Client::onAckMessage(const IncomingMessage& msg)
{
	assert(msg.data != nullptr);

	int32_t count = msg.data->readInt32();
	for (int32_t i = 0; i < count; i++)
	{
		uint32_t seq = msg.data->readInt32();
		for (NetworkMessage& outMsg : m_session.messageBuffer)
		{
			if (outMsg.type == MessageType::MESSAGE_CLEAR) continue;
			if (outMsg.sequenceNr == seq)
			{
				LOG_DEBUG("Client: Message %i has been ACK'd", seq);
				outMsg.type = MessageType::MESSAGE_CLEAR;
				delete outMsg.data;
				outMsg.data = nullptr;
				break;
			}
		}
	}
}

void Client::processIncomingMessages(float deltaTime)
{
	m_networkInterface.update(deltaTime);
	std::queue<IncomingMessage>& orderedMessages = m_networkInterface.getOrderedMessages();
	std::queue<IncomingMessage>& unorderedMessages = m_networkInterface.getMessages();

	// Process ordered queue
	while (!orderedMessages.empty())
	{
		IncomingMessage& incomingMessage = orderedMessages.front();
		if (incomingMessage.sequenceNr > m_lastOrderedMessaged)
		{
			m_lastOrderedMessaged = incomingMessage.sequenceNr;
			processMessage(incomingMessage);
		}
		destroyMessage(incomingMessage);
		orderedMessages.pop();
	}

	// Process unordered queue
	while (!unorderedMessages.empty())
	{
		IncomingMessage& incomingMessage = unorderedMessages.front();
		processMessage(incomingMessage);
		destroyMessage(incomingMessage);
		unorderedMessages.pop();
	}
}

void Client::processOutgoingMessages(float deltaTime)
{
	m_messageSentTime += deltaTime;

	if (m_messageSentTime >= m_maxMessageSentTime)
	{
		sendMessages();
	}
}

void Client::processMessage(const IncomingMessage& msg)
{
	switch (msg.type)
	{
		case MessageType::MESSAGE_ACK:
		{
			onAckMessage(msg);
			break;
		}
		/** Server to client */
		case MessageType::SERVER_GAMESTATE:
		{
			//if (msg.sequenceNumber < m_lastReceivedState)
			//{
			//	// discard packet
			//}
			//else
			//{
			//	m_lastReceivedState = packet.header.sequenceNumber;
			//}
			break;
		}

		/** Connection */
		case MessageType::CLIENT_CONNECT_ACCEPT:
		{
			onHandshake(msg);
			break;
		}
		case MessageType::CLIENT_DISCONNECT:
		{
			break;
		}

		default: break;
	}

	if (msg.isReliable)
	{
		LOG_DEBUG("Received reliable msg %i", msg.sequenceNr);
		m_reliableAckList.push_back(msg.sequenceNr);
	}

}

void Client::sendInput()
{

}

void Client::setState(State state)
{
	m_state = state;
	m_stateTimer = 0.0f;

	switch (state)
	{
		case State::STATE_DISCONNECTED:
		{
			m_connectionAttempt = 0;
		};
		default: break;
	}
}

void Client::sendMessages()
{
	static std::vector<NetworkMessage> messages;

	// Create ACK message
	{
		NetworkMessage ackMsg = {};
		ackMsg.type = MessageType::MESSAGE_ACK;
		ackMsg.isOrdered = true;
		if (m_reliableAckList.size() > 0)
		{
			ackMsg.data = BitStream::create();
			ackMsg.data->writeInt32(static_cast<int32_t>(
				m_reliableAckList.size())); // write length
			for (auto seq : m_reliableAckList)
			{
				ackMsg.data->writeInt32(seq); // write seq
				LOG_DEBUG("Sending ack %i", seq);
			}
			m_reliableAckList.clear();
			queueMessage(ackMsg);
		}
	}

	// Send other messages
	for (NetworkMessage& msg : m_session.messageBuffer)
	{
		if (msg.type == MessageType::MESSAGE_CLEAR) continue;
		messages.push_back(msg);
		if (!msg.isReliable)
		{
			msg.type = MessageType::MESSAGE_CLEAR;
		}
	}

	m_networkInterface.sendMessages(m_session.serverAddress, messages);
	messages.clear();
}

