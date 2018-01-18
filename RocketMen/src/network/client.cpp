
#include <network/client.h>

#include <core/action_buffer.h>
#include <core/entity.h>
#include <core/entity_manager.h>
#include <core/game.h>
#include <core/input.h>
#include <core/debug.h>
#include <core/game_time.h>
#include <network/network.h>
#include <network/address.h>
#include <network/packet_receiver.h>
#include <network/server.h>
#include <network/socket.h>
#include <utility/utility.h>

using namespace network;

static const int16_t s_firstTempNetworkId = -2; // Reserve -1 for INDEX_NONE

static ActionBuffer s_playerActions[s_maxPlayersPerClient];

//=============================================================================

Client::Client(Game* game) :
	m_game(game),
	m_connection(nullptr),
	m_lastReceivedSnapshotId((Sequence)INDEX_NONE),
	m_lastFrameSent(0),
	m_lastFrameSimulated(0),
	m_lastOrderedMessaged(0),
	m_state(State::Disconnected),
	m_timeSinceLastInputMessage(0.0f),
	m_maxInputMessageSentTime(0.05f),
	m_timeSinceLastClockSync(0.f),
	m_clockResyncTime(5.f),
	m_packetReceiver(new PacketReceiver(64)),
	m_requestedEntities(s_maxSpawnPredictedEntities),
	m_localPlayers(s_maxPlayersPerClient),
	m_tempNetworkIdManager(s_maxSpawnPredictedEntities)
{
	clearSession();
	m_socket = Socket::create();
	assert(m_socket != nullptr);
}

Client::~Client()
{
	delete m_socket;
	delete m_packetReceiver;
}

void Client::setPort(uint16_t port)
{
	m_port = port;
}

void Client::update(const Time& time)
{
	const float deltaTime = time.getDeltaSeconds();
	
	if (m_state == State::Disconnected)
		return;
	
	assert(m_connection != nullptr);
	m_timeSinceLastInputMessage += deltaTime;

	const State prevState = m_state;
	receivePackets();
	if (m_state != prevState)
	{
		return;
	}

	readMessages(time);

	if (m_state == State::Connected)
	{
		readInput();
		if (m_timeSinceLastInputMessage >= m_maxInputMessageSentTime)
		{
			m_timeSinceLastInputMessage = 0.f;
			sendPlayerActions();
			for (LocalPlayer& player : m_localPlayers)
			{
				syncOwnedEntities(player.playerId);
			}
		}

		if (m_timeSinceLastClockSync > m_clockResyncTime)
		{
			requestServerTime(time);
		}		
	}

	sendPendingMessages(time);
	if (!Network::isServer())
	{
		m_connection->update(time);
	}

	if (m_state == State::Disconnected)
	{
		clearSession();
	}
}

void Client::tick(Sequence frameId)
{
	if (m_state != Client::State::Connected)
		return;

	Frame* currentFrame = m_clientHistory.insertFrame(frameId);
	for (LocalPlayer& player : m_localPlayers)
	{
		ActionBuffer& playerActions = s_playerActions[player.playerId];
		if (!playerActions.isEmpty())
		{
			m_game->processPlayerActions(playerActions, player.playerId);
			currentFrame->actions[player.playerId].insert(playerActions);
			playerActions.clear();
		}
	}
	m_lastFrameSimulated = frameId;
}

void Client::sendPlayerActions()
{
	if (sequenceLessThan(m_lastFrameSimulated, m_lastFrameSent)
		|| Network::isServer()
		|| m_localPlayers.getCount() < 1
		|| m_localPlayers[0].playerId <= INDEX_NONE)
	{
		return;
	}

	Message* message = new Message(MessageType::PlayerInput);

	int32_t numFramesToSend = sequenceDifference(m_lastFrameSimulated, m_lastFrameSent);
	serializeInt(message->data, numFramesToSend);

	int32_t startFromFrame = static_cast<int32_t>(m_lastFrameSent + 1);
	serializeInt(message->data, startFromFrame);

	for (int16_t i = 0; i < numFramesToSend; i++)
	{
		const Sequence frameId = static_cast<Sequence>(startFromFrame) + i;
		if (Frame* frame = m_clientHistory.getFrame(frameId))
		{
			for (LocalPlayer& player : m_localPlayers)
			{
				int32_t playerId = static_cast<int32_t>(player.playerId);
				serializeInt(message->data, playerId, 0, s_maxPlayersPerClient);
				frame->actions[player.playerId].writeToMessage(message);
				frame->actions[player.playerId].clear();
			}
			m_lastFrameSent = frameId;
		}
		else
		{
			assert(false);
			break;
		}
	}

	sendMessage(message);
}

void Client::requestServerTime(const Time& localTime)
{
	Message* pingMessage = new Message(MessageType::ClockSync);
	uint64_t timeMilliseconds = localTime.getMilliSeconds();
	pingMessage->data.serializeData(reinterpret_cast<const char*>(&timeMilliseconds), sizeof(uint64_t));
	sendMessage(pingMessage);
}

void Client::readInput()
{
	for (const LocalPlayer& player : m_localPlayers)
	{
		ActionBuffer& inputActions = s_playerActions[player.playerId];
		input::getActions(player.controllerId, inputActions, player.listenMouseKB);
	}
}

void Client::connect(const Address& address, std::function<void(SessionResult)> callback)
{
	using namespace std::placeholders;

	if (!ensure(m_state == Client::State::Disconnected))
	{
		LOG_ERROR("Client::connect: Already connected");
		return;
	}

	assert(m_connection == nullptr);
	assert(m_socket->isInitialized() == false);

	if (m_socket->initialize(m_port))
	{
		m_sessionCallback = callback;
		m_connection = new Connection(m_socket, address, 
			std::bind(&Client::onConnectionCallback, this, _1, _2));

		m_connection->tryConnect();
		m_state = Client::State::Connecting;
	}
	else
	{
		LOG_ERROR("Client: Failed to initialize socket");
	}
}

void Client::disconnect()
{
	if (m_state != State::Connected && m_state != State::Connecting)
	{
		return;
	}

	m_connection->sendMessage(new Message(MessageType::Disconnect));

	setState(State::Disconnecting);
	m_connection->close();
}

LocalPlayer& Client::addLocalPlayer(int32_t controllerId, bool enableMouseKB)
{
	assert(m_localPlayers.getCount() < s_maxPlayersPerClient);

	LocalPlayer& player = m_localPlayers.insert();
	player.controllerId = controllerId;
	player.listenMouseKB = enableMouseKB;
	return player;
}

bool Client::requestEntitySpawn(Entity* entity)
{
	assert(entity != nullptr);
	assert(entity->getNetworkId() == INDEX_NONE);
	if (!m_tempNetworkIdManager.hasAvailable())
	{
		LOG_WARNING("Client::requestEntity: too many entities requested!");
		return false;
	}

	int32_t tempId = -m_tempNetworkIdManager.getNext() + s_firstTempNetworkId;
	entity->setNetworkId(tempId);

	Message* message = new Message(MessageType::RequestEntitySpawn);
	serializeInt(message->data, tempId, -s_maxSpawnPredictedEntities + s_firstTempNetworkId , -2);

	EntityManager::serializeFullEntity(entity, message->data);
		
	m_connection->sendMessage(message);
	//LOG_DEBUG("Client::requestEntitySpawn temp_%d", tempId);
	return true;
}

void Client::requestEntity(int32_t netId)
{
	assert(netId > INDEX_NONE && netId < s_maxNetworkedEntities);
	if (!m_requestedEntities.contains(netId))
	{
		m_requestedEntities.insert(netId);
	//	LOG_DEBUG("Client::requestEntity netID %d", netId);

		Message* message = new Message(MessageType::RequestEntity);
		serializeInt(message->data, netId, 0, s_maxNetworkedEntities);
		m_connection->sendMessage(message);
	}
}

uint32_t Client::getNumLocalPlayers() const
{
	return m_localPlayers.getCount();
}

bool Client::isLocalPlayer(int16_t playerId) const
{
	return getLocalPlayer(playerId) != nullptr;
}

LocalPlayer* Client::getLocalPlayer(int16_t playerId) const
{
	for (auto& localPlayer : m_localPlayers)
	{
		if (localPlayer.playerId == playerId)
		{
			return &localPlayer;
		}
	}

	return nullptr;
}

Client::State Client::getState() const
{
	return m_state;
}

void Client::syncOwnedEntities(int16_t playerId)
{
	std::vector<Entity*> entities = EntityManager::getEntities();
	std::vector<Entity*> ownedEntities;
	std::copy_if(entities.begin(), entities.end(), std::back_inserter(ownedEntities), [playerId](Entity* entity)
	{
		return entity->getOwnerPlayerId() == playerId;
	});
	
	int32_t entityCount = static_cast<int32_t>(ownedEntities.size());

	if (entityCount > 0)
	{
		Message* message = new Message(MessageType::Snapshot);

		serializeInt(message->data, entityCount);
		for (Entity* entity : ownedEntities)
		{
			int32_t networkId = entity->getNetworkId();
			serializeInt(message->data, networkId);
			EntityManager::serializeClientVars(entity, message->data);
		}
		sendMessage(message);
	}
}

void Client::readMessage(IncomingMessage& message, const Time& localTime)
{
	switch (message.type)
	{
		case MessageType::Snapshot:
		{
			if (sequenceLessThan(message.id, m_lastReceivedSnapshotId))
			{ // old, discard
				break;
			}
			
			m_lastReceivedSnapshotId = message.id;
			onSnapshot(message);
			break;
		}

		case MessageType::SpawnEntity:
		{
			onSpawnEntity(message);
			break;
		}
		case MessageType::AcceptEntity:
		{
			onAcceptEntity(message);
			break;
		}
		case MessageType::DestroyEntity:
		{
			onDestroyEntity(message);
			break;
		}

		case MessageType::AcceptClient:
		{
			onConnectionEstablished(message);
			break;
		}
		case MessageType::AcceptPlayer:
		{
			onAcceptPlayer(message);
			break;
		}
		case MessageType::ClockSync:
		{
			onReceiveServerTime(message, localTime);
			break;
		}
		case MessageType::Disconnect:
		{
			onDisconnected();
			break;
		}

		case MessageType::KeepAlive:
		case MessageType::RequestEntity:
		case MessageType::IntroducePlayer:
		case MessageType::PlayerInput:
		case MessageType::None:
		case MessageType::RequestConnection:
		case MessageType::GameEvent:
		case MessageType::RequestEntitySpawn:
		case MessageType::NUM_MESSAGE_TYPES:
		{
			break;
		}
	}

	message.markAsRead();
}

void Client::onConnectionEstablished(IncomingMessage& inMessage)
{
	if (!ensure(m_state == State::Connecting))
		return;

	int32_t receivedClientId = INDEX_NONE;
	serializeInt(inMessage.data, receivedClientId, 0, s_maxConnectedClients);

	LOG_INFO("Client: Connection established with the server. My ID: %d", receivedClientId);
	setState(State::Connected);

	m_lastFrameSent = m_lastFrameSimulated;

	if (Network::isServer())
	{
		Network::getLocalServer()->registerLocalClientId(receivedClientId);
	}

	m_sessionCallback(SessionResult::Joined);

	// Introduce Players
	Message* outMessage = new Message(MessageType::IntroducePlayer);

	int32_t numPlayers = getNumLocalPlayers();
	serializeInt(outMessage->data, numPlayers, 1, s_maxPlayersPerClient);

	sendMessage(outMessage);
}

void Client::onAcceptPlayer(IncomingMessage& msg)
{
	if (m_localPlayers[0].playerId != INDEX_NONE)
	{
		assert(false);
		return;
	}
	
	for (auto& player : m_localPlayers)
	{
		int32_t playerId = INDEX_NONE;
		serializeInt(msg.data, playerId, 0, s_maxPlayersPerClient);

		player.playerId = static_cast<int16_t>(playerId);
		LOG_INFO("Received player id %i", player.playerId);
	}
}

void Client::onSpawnEntity(IncomingMessage& message)
{
	int32_t networkId = INDEX_NONE;
	serializeInt(message.data, networkId);

	if (networkId >= 0 && networkId < s_maxNetworkedEntities)
	{
		int32_t index = m_requestedEntities.find(networkId);
		if (index != INDEX_NONE)
		{
			m_requestedEntities[index] = INDEX_NONE;
		}

		auto& entities = EntityManager::getEntities();
		if (Entity* entity = findPtrByPredicate(entities.begin(), entities.end(),
			[networkId](Entity* it) { return it->getNetworkId() == networkId; }))
		{
			return;
		}

		/*Entity* entity =*/ EntityManager::instantiateEntity(message.data, networkId);
		//LOG_DEBUG("Client::onSpawnEntity ID: %d netID: %d", entity->getId(), entity->getNetworkId());
	}
	else
	{
		assert(false);
	}
}

void Client::onAcceptEntity(IncomingMessage& inMessage)
{
	int32_t localId = INDEX_NONE; 
	serializeInt(inMessage.data, localId);
	if (localId > s_firstTempNetworkId)
	{
		return;
	}

	int32_t remoteId = INDEX_NONE;
	serializeInt(inMessage.data, remoteId);
	if (remoteId < 0 || remoteId >= s_maxNetworkedEntities)
	{
		return;
	}

	const int32_t index = m_requestedEntities.find(localId);
	if (index != INDEX_NONE)
	{
		m_requestedEntities[index] = INDEX_NONE;
	}
	
	m_tempNetworkIdManager.remove(-localId + s_firstTempNetworkId);

	auto& entities = EntityManager::getEntities();
	if (Entity* entity = findPtrByPredicate(entities.begin(), entities.end(),
		[localId](Entity* it) { return it->getNetworkId() == localId; } ))
	{
		entity->setNetworkId(remoteId);
#ifdef _DEBUG
		LOG_DEBUG("Client: Accepted entity ID: %d netID: %d", entity->getId(), entity->getNetworkId());
#endif
	}
	else
	{
		LOG_DEBUG("Client::onAcceptEntity: Unknown netID (%i)", localId);
	}
}

void Client::onDestroyEntity(IncomingMessage& inMessage)
{
	int32_t networkId = INDEX_NONE;
	serializeInt(inMessage.data, networkId);

	if (networkId < -s_maxSpawnPredictedEntities
		|| networkId > s_maxNetworkedEntities)
	{
		return;
	}

	auto entityList = EntityManager::getEntities();
	if (Entity* netEntity = findPtrByPredicate(entityList.begin(), entityList.end(),
		[networkId](Entity* entity) -> bool { return entity->getNetworkId() == networkId; }))
	{
		netEntity->kill();
	}
} 

void Client::onSnapshot(IncomingMessage& inMessage)
{
	std::vector<Entity*>& entities = EntityManager::getEntities();
	std::vector<Entity*> replicatedEntities;
	for (Entity* entity : entities)
	{
		if (entity->isReplicated())
		{
			replicatedEntities.push_back(entity);
		}
	}

	int32_t numReceivedEntities = 0;
	serializeInt(inMessage.data, numReceivedEntities);
	if (numReceivedEntities <= 0 || numReceivedEntities > s_maxNetworkedEntities)
		return;

	int32_t numEntitiesRead = 0;

	for (int32_t networkId = 0; networkId < s_maxNetworkedEntities; networkId++)
	{
		bool isEntityWritten = false;
		serializeBool(inMessage.data, isEntityWritten);
		if (isEntityWritten)
		{
			numEntitiesRead++;

			if (Entity* netEntity = findPtrByPredicate(replicatedEntities.begin(), replicatedEntities.end(),
				[networkId](Entity* entity) -> bool { return entity->getNetworkId() == networkId; }))
			{
				EntityManager::serializeEntity(netEntity, inMessage.data);
			}
			else
			{
				requestEntity(networkId);
			}
		}
		if (numEntitiesRead >= numReceivedEntities)
		{
			break;
		}
	}	
}

void Client::onReceiveServerTime(IncomingMessage& message, const Time& localTime)
{
	uint64_t originalTime = 0;
	message.data.serializeData(reinterpret_cast<char*>(&originalTime), sizeof(uint64_t));

	uint64_t serverTime = 0;
	message.data.serializeData(reinterpret_cast<char*>(&serverTime), sizeof(uint64_t));

	const uint64_t currentTime = localTime.getMilliSeconds();
	const uint64_t latency = currentTime - originalTime;

	// http://www.mine-control.com/zack/timesync/timesync.html
}

void Client::onDisconnected()
{
	assert(m_connection != nullptr);
	setState(Client::State::Disconnected);
}

void Client::sendMessage(Message* message)
{
	assert(m_connection != nullptr);
	m_connection->sendMessage(message);
}

void Client::sendPendingMessages(const Time& localTime)
{
	assert(m_connection != nullptr);

	m_connection->sendPendingMessages(localTime);
}

void Client::setState(State state)
{
	m_state = state;
}

void Client::clearSession()
{
	m_localPlayers.clear();
	m_requestedEntities.fill(INDEX_NONE);
	
	delete m_connection;
	m_connection = nullptr;

	EntityManager::killEntities();
}

void Client::receivePackets()
{
	m_packetReceiver->receivePackets(m_socket);

	Buffer<Packet*>&  packets = m_packetReceiver->getPackets();

	const int32_t packetCount = packets.getCount();
	for (int32_t i = 0; i < packetCount; i++)
	{
		Packet* packet = packets[i];
		if (packet->address == m_connection->getAddress())
		{
			m_connection->receivePacket(*packet);
		}
		
		delete packet;
	}

	packets.clear();
}

void Client::readMessages(const Time& localTime)
{
	while (IncomingMessage* message = m_connection->getNextMessage())
	{
		readMessage(*message, localTime);
	}
}

void Client::onConnectionCallback(ConnectionCallback type, Connection* connection)
{
	assert(connection != nullptr);
	assert(connection == m_connection);

	switch (type)
	{
		case ConnectionCallback::ConnectionEstablished:
		{
			setState(State::Connected);
			break;
		}
		case ConnectionCallback::ConnectionFailed:
		{
			LOG_INFO("Client: Failed to connect to the server");
			setState(State::Disconnected);
			connection->close();
			clearSession();
			m_sessionCallback(SessionResult::Failed);
			break;
		}
		case ConnectionCallback::ConnectionLost:
		{
			setState(State::Connecting);
			break;
		}
		case ConnectionCallback::ConnectionReceived:
		{
			assert(false);
			break;
		}
	}
}
