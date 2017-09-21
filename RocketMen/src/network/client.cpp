
#include <network/client.h>

#include <core/action_buffer.h>
#include <core/entity.h>
#include <core/game.h>
#include <core/input.h>
#include <core/debug.h>
#include <game_time.h>
#include <network.h>
#include <network/address.h>
#include <network/server.h>
#include <utility.h>

using namespace network;

static const uint32_t s_maxConnectionAttempts = 3;
static const uint32_t s_connectionRetryTime   = 5;

static const int32_t s_firstTempNetworkID = -2; // Reserve -1 for INDEX_NONE
static int32_t s_nextTempNetworkID = s_firstTempNetworkID;



Client::Client(Time& time, Game* game) :
	m_gameTime(time),
	m_game(game),
	m_lastReceivedState(0),
	m_lastOrderedMessaged(0),
	m_state(State::Disconnected),
	m_stateTimer(0.0f),
	m_messageSentTime(0.0f),
	m_maxMessageSentTime(0.05f),
	m_connectionAttempt(0),
	m_isInitialized(false),
	m_numLocalPlayers(0)
{
	clearSession();
}

Client::~Client()
{
}

bool Client::initialize()
{
	clearSession();
	clearLocalPlayers();

	m_isInitialized = true;
	return true;
}

bool Client::isInitialized() const
{
	return m_isInitialized;
}

void Client::update()
{
	const float deltaTime = m_gameTime.getDeltaSeconds();

	if (m_state != State::Disconnected)
	{
		m_networkInterface.update(m_gameTime);
		processIncomingMessages(deltaTime);
	}

	processOutgoingMessages(deltaTime);

	m_stateTimer += deltaTime;
	switch (m_state)
	{
		case State::Connecting:
		{
			if (!m_networkInterface.isConnecting())
			{
				setState(State::Disconnected);
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

					setState(State::Disconnected);
					clearSession();
				}
			}
			break;
		}
		case State::Connected:
		case State::Disconnecting:
		case State::Disconnected:
			break;
	}
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

void Client::connect(const Address& address)
{
	m_connectionAttempt++;
	LOG_DEBUG("Client: Connecting.., attempt %i",
	          m_connectionAttempt);

	m_session.serverAddress = address;

	m_networkInterface.connect(address, m_gameTime);

	if (m_state != State::Connecting)
	{
		setState(State::Connecting);
	}
}

void Client::disconnect()
{
	if (m_state != State::Connected || m_state != State::Connecting)
	{
		ensure(false);
		return;
	}

	m_networkInterface.disconnect(m_session.serverAddress);
	setState(State::Disconnecting);
}

void Client::queueMessage(const NetworkMessage& inMessage)
{
	for (OutgoingMessage& message : m_session.outgoingMessages)
	{
		if (message.type == MessageType::None)
		{
			message.data.reset();
			message.data.writeBuffer(inMessage.data.getBuffer(), inMessage.data.getLength());
			message.type       = inMessage.type;
			message.isReliable = inMessage.isReliable;
			message.timestamp  = m_gameTime.getSeconds();
			message.sequence   = m_session.nextMessageID++;
			m_session.pendingMessages++;
			break;
		}
	}
}

void Client::queueMessage(const OutgoingMessage& inMessage)
{
	for (OutgoingMessage& message : m_session.outgoingMessages)
	{
		if (message.type == MessageType::None)
		{
			message = inMessage;
			m_session.pendingMessages++;
			break;
		}
	}
}

void Client::clearLocalPlayers()
{
	for (LocalPlayer& localPlayer : m_localPlayers)
	{
		localPlayer.controllerID = INDEX_NONE;
		localPlayer.playerID     = INDEX_NONE;
	}
}

bool Client::addLocalPlayer(int32_t controllerID)
{
	if (m_numLocalPlayers < s_maxPlayersPerClient)
	{
		m_localPlayers[m_numLocalPlayers++].controllerID = controllerID;
		return true;
	}

	return false;
}

bool Client::requestEntity(Entity* entity)
{
	const int32_t tempID = getNextTempNetworkID();
	entity->setNetworkID(tempID);

	if (m_recentlyPredictedSpawns.contains(tempID) == false)
	{
		m_recentlyPredictedSpawns.insert(tempID);

		NetworkMessage message = {};
		message.type           = MessageType::RequestEntity;
		message.isOrdered      = true;
		message.isReliable     = true;

		message.data.writeInt16(tempID);

		WriteStream stream(128);
		Entity::serializeFull(entity, stream);
		message.data.writeData((char*)stream.getBuffer(), stream.getLength());
	
		queueMessage(message);
		return true;
	}

	return false;
}

uint32_t Client::getNumLocalPlayers() const
{
	return m_numLocalPlayers;
}

bool Client::isLocalPlayer(int32_t playerID) const
{
	for (auto player : m_localPlayers)
	{
		if (player.playerID == playerID)
		{
			return true;
		}
	}

	return false;
}

void Client::onHandshake(IncomingMessage& msg)
{
	if (!ensure(m_state == State::Connecting))
		return;

	int32_t myID = msg.data.readInt32();
	LOG_INFO("Client: Received handshake from the server! I have received ID  %d\n", myID);
	setState(State::Connected);

	if (Network::isServer())
	{
		Network::getLocalServer()->registerLocalClientID(myID);
	}

	// Introduce Players
	NetworkMessage outMessage = {};
	outMessage.type = MessageType::IntroducePlayer;
	outMessage.data.writeInt32(getNumLocalPlayers());
	outMessage.isReliable = true;
	
	//for (auto player : m_localPlayers)
	//{
	//	// Write player info
	//}

	queueMessage(outMessage);
}

void Client::onAcceptPlayer(IncomingMessage& msg)
{
	if (m_localPlayers[0].playerID < 0 == false)
	{
		ensure(false);
		return;
	}
	
	for (uint32_t i = 0; i < getNumLocalPlayers(); i++)
	{
		m_localPlayers[i].playerID = msg.data.readInt32();
		LOG_INFO("Received player id %i", m_localPlayers[i].playerID);
	}
}

void Client::onSpawnEntity(IncomingMessage& msg)
{
	ReadStream readStream(static_cast<int32_t>(msg.data.getLength()));
	msg.data.readBytes((char*)readStream.getBuffer(), msg.data.getLength() - msg.data.getReadTotalBytes());

	Entity* entity = Entity::instantiate(readStream);

#ifdef _DEBUG
	LOG_DEBUG("Client: Spawned entity ID: %d netID: %d", entity->getID(), entity->getNetworkID());
#endif // _DEBUG
}

void Client::onAcceptEntity(IncomingMessage& msg)
{
	LOG_DEBUG("OnAcceptEntity 1");
	int32_t localID  = msg.data.readInt32();
	int32_t remoteID = msg.data.readInt32();
	Entity* entity   = nullptr;

	if (int32_t index = m_recentlyPredictedSpawns.find(localID) != INDEX_NONE)
	{
		m_recentlyPredictedSpawns[index] = 0;
	}
	LOG_DEBUG("OnAcceptEntity 2");
	auto entities = Entity::getList();
	if (Entity* entity = findPtrByPredicate(entities.begin(), entities.end(),
		[localID](Entity* it) { return it->getNetworkID() == localID; } ))
	{
		if (m_recentlyDestroyedEntities.contains(remoteID))
		{
			entity->kill();
			return;
		}

		entity->setNetworkID(remoteID);
	}
	else
	{
		LOG_DEBUG("Client::onAcceptEntity: Unknown netID (%i)", localID);
	}
	LOG_DEBUG("OnAcceptEntity 3");
}

void Client::onDestroyEntity(IncomingMessage& msg)
{
	int32_t networkID = msg.data.readInt32();

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

	// Not found, but remember it in case a spawn message comes in delayed
	m_recentlyDestroyedEntities.insert(networkID); // TODO Remove once messages are ordered
} 

void Client::onGameState(IncomingMessage& msg)
{
	std::vector<Entity*>& entityList = Entity::getList();
	ReadStream readStream(512);

	msg.data.readBytes((char*)readStream.getBuffer(),
        msg.data.getLength() - msg.data.getReadTotalBytes());

	for (uint32_t netID = 0; netID < s_maxNetworkedEntities; netID++)
	{
		bool readEntityData = false;
		serializeBit(readStream, readEntityData);
		if (readEntityData)
		{
			if (Entity* netEntity = findPtrByPredicate(entityList.begin(), entityList.end(), 
				[netID](Entity* entity) -> bool { return entity->getNetworkID() == netID; }))
			{
				Entity::serialize(netEntity, readStream);
			}
		}
	}
}

void Client::processIncomingMessages(float deltaTime)
{
	using IncomingMessages = std::queue<IncomingMessage>;
		
	IncomingMessages& orderedMessages = m_networkInterface.getOrderedMessages();
	IncomingMessages& unorderedMessages = m_networkInterface.getMessages();

	if (orderedMessages.size() >= 2048)
	{
		LOG_WARNING("Client: orderedMessages is very big!");
	}

	if (unorderedMessages.size() >= 2048)
	{
		LOG_WARNING("Client: unorderedMessages is very big!");
	}

	// Process ordered queue
	while (!orderedMessages.empty())
	{
		IncomingMessage& incomingMessage = orderedMessages.front();
		if (incomingMessage.sequence > (int32_t)m_lastOrderedMessaged)
		{
			m_lastOrderedMessaged = incomingMessage.sequence;
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

	const float currentTime = m_gameTime.getSeconds();

	/** Send all queued messages */
	if (m_messageSentTime >= m_maxMessageSentTime)
	{
		sendMessages();
	}
}

void Client::processMessage(IncomingMessage& msg)
{
	// Disregard duplicate messages
	if (m_recentlyProcessed.contains(msg.sequence))
	{
		return;
	}

	m_recentlyProcessed.insert(msg.sequence);

	switch (msg.type)
	{
		case MessageType::Gamestate:
		{
			if (msg.sequence <= m_lastReceivedState)
			{ // discard packet
				break;
			}
			
			m_lastReceivedState = msg.sequence;
			onGameState(msg);	
			break;
		}
		case MessageType::SpawnEntity:
		{
			onSpawnEntity(msg);
			break;
		}
		case MessageType::AcceptEntity:
		{
			onAcceptEntity(msg);
			break;
		}
		case MessageType::DestroyEntity:
		{
			onDestroyEntity(msg);
			break;
		}

		case MessageType::AcceptClient:
		{
			onHandshake(msg);
			break;
		}
		case MessageType::AcceptPlayer:
		{
			onAcceptPlayer(msg);
			break;
		}

		case MessageType::RequestEntity:
		case MessageType::IntroducePlayer:
		case MessageType::PlayerInput:
		case MessageType::None:
		case MessageType::Ping:
		case MessageType::RequestConnection:
		case MessageType::Disconnect:
		case MessageType::GameEvent:
		case MessageType::NUM_MESSAGE_TYPES:
		{
			break;
		}
	}
}

void Client::sendInput()
{
}

void Client::setState(State state)
{
	switch (state)
	{
		case State::Disconnected:
		{
			m_connectionAttempt = 0;
		};
		case State::Connected:
		case State::Connecting:
		case State::Disconnecting:
		{
			break;
		}
	}

	m_state = state;
	m_stateTimer = 0.0f;
}

void Client::requeueReliableMessages()
{
	for (OutgoingMessage& message : m_session.outgoingReliableMessages)
	{
		SequenceBuffer<SentMessage>& acks = m_networkInterface.getAcks();

		if (acks.exists(message.sequence))
		{
			if (acks.getEntry(message.sequence)->acked)
			{
				message.type = MessageType::None;
				continue;
			}
		}

		if (m_gameTime.getSeconds() - message.timestamp >= 1.0f)
		{
			//queueMessage(message);
			message.type = MessageType::None;
		}
	}
}

void Client::sendMessages()
{
	if (m_session.pendingMessages > 60)
	{
		LOG_DEBUG("Client: Warning: pendingMessages is very big");
	}

	m_networkInterface.sendMessages(m_session.outgoingMessages.data(), 
		                            s_maxPendingMessages,
		                            m_session.serverAddress);
	m_session.pendingMessages = 0;
}

void Client::clearSession()
{
	m_session.isActive        = false;
	m_session.pendingMessages = 0;
	m_session.nextMessageID   = 0;

	m_recentlyProcessed.fill(INDEX_NONE);
	m_recentlyDestroyedEntities.fill(INDEX_NONE);
	m_recentlyPredictedSpawns.fill(0);
}

int32_t Client::getNextTempNetworkID()
{
	int32_t tempNetworkID = s_nextTempNetworkID--;
	if (s_nextTempNetworkID <= -s_maxSpawnPredictedEntities - 1)
	{
		s_nextTempNetworkID = s_firstTempNetworkID;
	}

	return tempNetworkID;
}
