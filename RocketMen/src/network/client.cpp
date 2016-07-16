
#include <network/client.h>

#include <core/action_buffer.h>
#include <core/entity.h>
#include <core/game.h>
#include <core/input.h>
#include <core/debug.h>
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
	clearSession();
}

Client::~Client()
{
}

bool Client::initialize()
{
	clearSession();
	m_isInitialized = true;
	return true;
}

bool Client::isInitialized() const
{
	return m_isInitialized;
}

void Client::clearSession()
{
	m_session.isActive        = false;
	m_session.sequenceCounter = 0;
	m_session.pendingMessages = 0;
	
	m_recentlyProcessed.fill(-1);
	m_recentlyDestroyedEntities.fill(-1);
	m_recentlyPredictedSpawns.fill(0);
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
			msg.data.reset();
			msg.data.writeBuffer(message.data.getBuffer(), message.data.getLength());
			msg.isReliable = message.isReliable;
			msg.sequenceNr = ++m_session.sequenceCounter;
			m_session.pendingMessages++;
			return;
		}
	}

	LOG_WARNING("Message Queue is full! Message discarded.");
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

bool Client::requestEntity(Entity* entity)
{
	const int32_t tempID = entity->getNetworkID();

	if (m_recentlyPredictedSpawns.contains(tempID))
		return false; // Too many requests!

	m_recentlyPredictedSpawns.insert(tempID);

	NetworkMessage msg = {};
	msg.type           = MessageType::ENTITY_REQUEST;
	msg.isOrdered      = true;
	msg.isReliable     = true;

	msg.data.writeInt16(tempID);

	WriteStream ws = {};
	ws.m_buffer = new uint32_t[128];
	ws.m_bufferLength = 128;

	Entity::serializeFull(entity, ws);
	msg.data.writeData((char*)ws.m_buffer, ws.getLength());
	delete[] ws.m_buffer;

	queueMessage(msg);
	return true;
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

void Client::onHandshake(IncomingMessage& msg)
{
	if (m_state != State::STATE_CONNECTING)
		return;
	LOG_INFO("Client: Received handshake from the server! I have received ID ");
	//========================
	
	// Read my ID
	int32_t myID = msg.data.readInt32();
	LOG_INFO("%d\n", myID);
	setState(State::STATE_CONNECTED);

	if (Network::isServer())
	{
		Network::getLocalServer()->registerLocalClient(myID);
	}

	// Introduce Players
	NetworkMessage outMessage = {};
	outMessage.type = MessageType::PLAYER_INTRO;
	outMessage.data.writeInt32(getNumLocalPlayers());
	outMessage.isReliable = true;
	
	//for (auto player : m_localPlayers)
	//{
	//	// Write player info
	//}

	queueMessage(outMessage);
}

void Client::onAckMessage(const IncomingMessage& msg)
{
}

void Client::onPlayerAccepted(IncomingMessage& msg)
{
	if (m_localPlayers[0].playerID >= 0)
		return; // Player ID already set, dismiss duplicate packet

	for (uint32_t i = 0; i < getNumLocalPlayers(); i++)
	{
		m_localPlayers[i].playerID = msg.data.readInt32();
		LOG_INFO("Received player id %i", m_localPlayers[i].playerID);
	}
}

void Client::onSpawnEntity(IncomingMessage& msg)
{
	LOG_INFO("onSpawnEntity");

	ReadStream rs = {};
	rs.m_buffer = new uint32_t[msg.data.getLength()];
	rs.m_bufferLength = int32_t(msg.data.getLength());
	msg.data.readBytes((char*)rs.m_buffer, 
                        msg.data.getLength() - msg.data.getReadTotalBytes());

	Entity::instantiate(rs);

	delete[] rs.m_buffer;
}

void Client::onApproveEntity(IncomingMessage& msg)
{
	int32_t localID  = msg.data.readInt32();
	int32_t remoteID = msg.data.readInt32();
	Entity* entity   = nullptr;

	int32_t index = m_recentlyPredictedSpawns.find(localID);
	if ( index >= 0)
	{
		m_recentlyPredictedSpawns[index] = 0;
	}


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

	if (m_recentlyDestroyedEntities.contains(remoteID))
	{
		entity->kill();
		return;
	}

	entity->setNetworkID(remoteID);
}

void Client::onDestroyEntity(IncomingMessage& msg)
{
	int32_t networkID = msg.data.readInt32();
	//LOG_INFO("onDestroyEntity (%d)", networkID);

	if (networkID < -s_maxSpawnPredictedEntities
		|| networkID > s_maxNetworkedEntities)
	{
		return;
	}

	for (auto& entity : Entity::getList())
	{
		if (entity->getNetworkID() == networkID)
		{
			entity->kill();
			return;
		}
	}

	// Not found, but remember it incase a spawn message comes in delayed
	m_recentlyDestroyedEntities.insert(networkID);
}

void Client::onGameState(IncomingMessage& msg)
{
	std::vector<Entity*>& entityList = Entity::getList();
	ReadStream rs = {};
	rs.m_bufferLength = 256;
	rs.m_buffer = new uint32_t[256];
	msg.data.readBytes((char*)rs.m_buffer, 
                        msg.data.getLength() - msg.data.getReadTotalBytes());

	for (int32_t netID = 0; netID < s_maxNetworkedEntities; netID++)
	{
		bool written = false;
		serializeBit(rs, written);
		if (written)
		{
			for (auto& entity : entityList)
			{
				if (entity->getNetworkID() == netID)
				{
					Entity::serialize(entity, rs);
				}
			}
		}
	}

	delete[] rs.m_buffer;
}

void Client::processIncomingMessages(float deltaTime)
{
	m_networkInterface.update(deltaTime);
	std::queue<IncomingMessage>& orderedMessages = 
		m_networkInterface.getOrderedMessages();

	std::queue<IncomingMessage>& unorderedMessages = 
		m_networkInterface.getMessages();

	if (orderedMessages.size() >= 2048)
	{
		LOG_WARNING("Client: orderedMessages is very big!");
		__debugbreak();
	}

	if (unorderedMessages.size() >= 2048)
	{
		LOG_WARNING("Client: unorderedMessages is very big!");
		__debugbreak();
	}

	// Process ordered queue
	while (!orderedMessages.empty())
	{
		IncomingMessage& incomingMessage = orderedMessages.front();
		if (incomingMessage.sequenceNr > (int32_t)m_lastOrderedMessaged)
		{
			m_lastOrderedMessaged = incomingMessage.sequenceNr;
			processMessage(incomingMessage);
		}
		orderedMessages.pop();
	}

	// Process unordered queue
	while (!unorderedMessages.empty())
	{
		IncomingMessage& incomingMessage = unorderedMessages.front();
		processMessage(incomingMessage);
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

void Client::processMessage(IncomingMessage& msg)
{
	static int32_t index = 0;

	// Disregard duplicate messages
	if (m_recentlyProcessed.contains(msg.sequenceNr))
	{
		return;
	}

	switch (msg.type)
	{
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
		case MessageType::DESTROY_ENTITY:
		{
			onDestroyEntity(msg);
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

	m_recentlyProcessed.insert(msg.sequenceNr);
	
	//if (msg.isReliable)
	//{
	////	LOG_DEBUG("Received reliable msg %i", msg.sequenceNr);
	//	m_reliableAckList.push_back(msg.sequenceNr);
	//}
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

	if (m_session.pendingMessages > 60)
	{
		LOG_WARNING("Client: pendingMessages is very big!");
	}
	
	for (NetworkMessage& msg : m_session.messageBuffer)
	{
		if (msg.type == MessageType::MESSAGE_CLEAR) continue;
		messages.push_back(msg);
		msg.type = MessageType::MESSAGE_CLEAR;
	}
	if (!messages.empty())
	{
		m_networkInterface.sendMessages(m_session.serverAddress, messages);
	}
	messages.clear();
	m_session.pendingMessages = 0;
}


