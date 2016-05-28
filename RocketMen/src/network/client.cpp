
#include <network/client.h>

#include <core/action_buffer.h>
#include <core/entity.h>
#include <core/game.h>
#include <core/input.h>
#include <core/debug.h>
#include <core/serialize.h>
#include <network/server.h>
#include <game_time.h>
#include <network/address.h>
#include <network.h>

#include <algorithm>
#include <assert.h>

using namespace network;

static const uint32_t s_maxConnectionAttempts = 3;
static const uint32_t s_connectionRetryTime   = 5;
//==============================================================================

Client::Client(Time& time, Game* game) :
	m_gameTime(time),
	m_game(game),
	m_lastReceivedState(0),
	m_lastOrderedMessaged(0),
	m_state(State::STATE_DISCONNECTED),
	m_stateTimer(0.0f),
	m_messageSentTime(0.0f),
	m_maxMessageSentTime(0.02f),
	m_connectionAttempt(0),
	m_isInitialized(false)
{
	std::fill(m_recentNetworkIDs, m_recentNetworkIDs + s_sequenceMemorySize, -1);
}

Client::~Client()
{
}

bool Client::initialize()
{
	m_session = {};
	std::fill(m_recentlyProcessed, m_recentlyProcessed + 32, -1);
	m_isInitialized = true;
	return true;
}

bool Client::isInitialized() const
{
	return m_isInitialized;
}

void Client::fixedUpdate(ActionBuffer& actions)
{
	if (actions.getNumActions() == 0)
		return;
	

	for (uint32_t i = 0; i < actions.getNumActions(); i++)
	{
		input::Action action = actions[i];
		action.getHash();
	}


	//NetworkMessage msg = {};
	//msg.type = MessageType::PLAYER_INPUT;
	//msg.isOrdered  = true;
	//msg.isReliable = true;
	//msg.data = BitStream::create();
	//msg.data->writeInt32(actions.getNumActions());

	//for (uint32_t i = 0; i < actions.getNumActions(); i++)
	//{
	//	input::Action action = actions[i];
	//	msg.data->writeInt64(action.getHash());
	//}

	//queueMessage(msg);
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

			if (static_cast<uint32_t>(m_stateTimer) >= m_connectionAttempt 
				* s_connectionRetryTime)
			{
				if (m_connectionAttempt < s_maxConnectionAttempts)
				{
					connect(m_session.serverAddress);
				}
				else
				{
					LOG_INFO("Connecting to %s timed out after %i attempts\n",
							 m_session.serverAddress.toString().c_str(), 
							 m_connectionAttempt);
					m_connectionAttempt = 0;
					m_session.serverAddress = Address();
					setState(State::STATE_DISCONNECTED);
					clearSession();
				}
			}
			break;
		}
		default: break;
	}
}

void Client::connect(const Address& address)
{
	m_connectionAttempt++;
	LOG_INFO("Client: Attempting to connect to %s, attempt %i", 
	         address.toString().c_str(),
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

	LOG_WARNING("Message Queue is full! Message discarded.");
	delete message.data;
}

void Client::setLocalPlayers(uint32_t numPlayers)
{
	m_localPlayers = {};

	for (uint32_t i = 0; i < numPlayers; i++)
	{
		m_localPlayers[i].isUsed       = true;
		m_localPlayers[i].controllerID = i;
		m_localPlayers[i].playerID     = -1;
	}
}

void Client::requestEntity(Entity* entity)
{
	NetworkMessage msg = {};
	msg.type           = MessageType::ENTITY_REQUEST;
	msg.isOrdered      = true;
	msg.isReliable     = true;
	msg.data           = new BitStream();
	
	entity->serializeFull(*msg.data);
	LOG_DEBUG("Requesting ent %i", entity->getNetworkID());
	queueMessage(msg);
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

bool Client::isLocalPlayer(int32_t playerID) const
{
	for (auto player : m_localPlayers)
	{
		if (player.playerID == playerID)
			return true;
	}

	return false;
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

	if (Network::isServer())
	{
		Network::getLocalServer()->registerLocalClient(myID);
	}

	// Introduce Players
	NetworkMessage outMessage = {};
	outMessage.type = MessageType::PLAYER_INTRO;
	outMessage.data = new BitStream();
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
			//	LOG_DEBUG("Client: Message %i has been ACK'd", seq);
				outMsg.type = MessageType::MESSAGE_CLEAR;
				delete outMsg.data;
				outMsg.data = nullptr;
				break;
			}
		}
	}
}

void Client::onPlayerAccepted(const IncomingMessage& msg)
{
	if (m_localPlayers[0].playerID >= 0)
		return; // Player ID already set, dismiss duplicate packet

	for (uint32_t i = 0; i < getNumLocalPlayers(); i++)
	{
		m_localPlayers[i].playerID = msg.data->readInt32();
		LOG_INFO("Received player id %i", m_localPlayers[i].playerID);
	}
}

void Client::onSpawnEntity(const IncomingMessage& msg)
{
	static int32_t nextID = 0;

	int32_t size = msg.data->readInt32();
	EntityInitializer* init = (EntityInitializer*)malloc(size);
	msg.data->readBytes((char*)init, size);

	for (auto id : m_recentNetworkIDs)
	{
		if (id == init->networkID)
		{
			// drop duplicate packet..
			free(init);
			return;
		}
	}

	m_recentNetworkIDs[nextID++] = init->networkID;
	if (nextID >= 15) nextID = 0;

	Entity::instantiate(init);

	free(init);
}

void Client::onApproveEntity(const IncomingMessage& msg)
{
	int32_t localID = msg.data->readInt32();
	LOG_DEBUG("Rec ent appr %i (%i)", localID, msg.sequenceNr);
	Entity* entity = nullptr;
	for (auto& _entity : Entity::getList())
	{
		if (_entity->getNetworkID() == localID)
		{
			entity = _entity;
			break;
		}
	}

	if (entity == nullptr)
	{
		LOG_DEBUG("Error, unknown netID (%i)", localID);
		return;
	}
	int32_t size = msg.data->readInt32();
	EntityInitializer* init = (EntityInitializer*)malloc(size);
	msg.data->readBytes((char*)init, size);

	Entity::instantiate(init, false, entity);

}

void Client::onGameState(const IncomingMessage& msg)
{
	std::vector<Entity*>& entityList = Entity::getList();
	int32_t numObjects = msg.data->readInt32();

	for (int32_t i = 0; i < numObjects; i++)
	{
		int32_t networkID = msg.data->readInt32();
		for (auto& entity : entityList)
		{
			if (entity->getNetworkID() == networkID)
			{
				entity->getTransform().setLocalPosition(
					Serialize::deserializePosition(msg.data)
				);
				
				entity->getTransform().setLocalRotation(
					Serialize::deserializeAngle(msg.data)
				);
			}
		}
	}
}

void Client::processIncomingMessages(float deltaTime)
{
	m_networkInterface.update(deltaTime);
	std::queue<IncomingMessage>& orderedMessages = 
		m_networkInterface.getOrderedMessages();

	std::queue<IncomingMessage>& unorderedMessages = 
		m_networkInterface.getMessages();

	// Process ordered queue
	while (!orderedMessages.empty())
	{
		IncomingMessage& incomingMessage = orderedMessages.front();
		if (incomingMessage.sequenceNr > (int32_t)m_lastOrderedMessaged)
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
	static int32_t index = 0;
	for (auto i : m_recentlyProcessed)
	{
		if (i == msg.sequenceNr)
			return; // Disregard duplicate message
	}
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
			if (msg.sequenceNr <= m_lastReceivedState)
			{// discard packet
				break;
			}
			else
			{
				m_lastReceivedState = msg.sequenceNr;
				onGameState(msg);
			}
			break;
		}
		case MessageType::SPAWN_ENTITY:
		{
			onSpawnEntity(msg);
			break;
		}
		case MessageType::APPROVE_ENTITY:
		{
			onApproveEntity(msg);
			break;
		}
		/** Connection */
		case MessageType::CLIENT_CONNECT_ACCEPT:
		{
			onHandshake(msg);
			break;
		}
		case MessageType::PLAYER_JOIN_ACCEPT:
		{
			onPlayerAccepted(msg);
			break;
		}
		case MessageType::CLIENT_DISCONNECT:
		{
			break;
		}

		default: break;
	}

	m_recentlyProcessed[index++] = msg.sequenceNr;
	if (index >= s_sequenceMemorySize) index = 0;

	if (msg.isReliable)
	{
	//	LOG_DEBUG("Received reliable msg %i", msg.sequenceNr);
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
			ackMsg.data = new BitStream();
			ackMsg.data->writeInt32(static_cast<int32_t>(
				m_reliableAckList.size())); // write length
			for (auto seq : m_reliableAckList)
			{
				ackMsg.data->writeInt32(seq); // write seq
			//	LOG_DEBUG("Sending ack %i", seq);
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
	if (!messages.empty())
	{
		m_networkInterface.sendMessages(m_session.serverAddress, messages);
	}
	messages.clear();
}

void Client::clearSession()
{
	memset(&m_session, 0, sizeof(m_session));
}

