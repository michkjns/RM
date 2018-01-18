
#include "server.h"

#include <core/entity.h>
#include <core/entity_manager.h>
#include <core/game.h>
#include <core/debug.h>
#include <core/action_buffer.h>
#include <network/common_network.h>
#include <network/connection.h>
#include <network/packet_receiver.h>
#include <network/remote_client.h>
#include <network/snapshot.h>
#include <network/socket.h>

#include <utility/utility.h>

extern "C" unsigned long crcFast(unsigned char const message[], int nBytes);

using namespace network;

Server::Server(Game* game) :
	m_game(game),
	m_type(GameSessionType::Offline),
	m_packetReceiver(new PacketReceiver(128)),
	m_networkIdManager(s_maxNetworkedEntities),
	m_clients(s_maxConnectedClients)
{
	m_socket = nullptr;
	reset();
}

Server::~Server()
{
	delete m_socket;
	delete m_packetReceiver;
}

void Server::reset()
{
	m_isInitialized    = false;
	m_playerIdCounter  = 0;
	m_snapshotTime     = 0.0f;
	m_networkIdManager.clear();
	m_clients.clear();

	EntityManager::killEntities();
	
	delete m_socket;
	m_socket = Socket::create();
	assert(m_socket != nullptr);
}

void Server::update(const Time& time)
{
	if (m_socket->isInitialized())
	{
		receivePackets();
		readMessages(time);
		createSnapshots(time.getDeltaSeconds());
		m_clients.sendPendingMessages(time);
		m_clients.updateConnections(time);
	}
}

void Server::fixedUpdate()
{
}

bool Server::host(uint16_t port, GameSessionType type)
{
	assert(m_socket != nullptr);

	if (!ensure(m_socket->isInitialized() == false))
	{
		LOG_WARNING("Server: Already listening to connections on port %d", m_socket->getPort());
		return false;
	}

	if (m_socket->initialize(port))
	{
		LOG_INFO("Server: Listening on port %d", port);
		m_type = type;
		return true;
	}
	else
	{
		LOG_WARNING("Server: Failed to listen on port %d", port);
	}
	return false;
}

void Server::generateNetworkId(Entity* entity)
{
	const int32_t networkId = m_networkIdManager.getNext();
	entity->setNetworkId(networkId);
	sendEntitySpawn(entity);
}

void Server::registerLocalClientId(int32_t clientId)
{
	m_clients.setLocalClientId(clientId);
}

void Server::destroyEntity(int32_t networkId)
{
	assert(networkId < s_maxNetworkedEntities);
	Message* message = new Message(MessageType::DestroyEntity);
	serializeInt(message->data, networkId);
	m_clients.sendMessage(message, true);
	m_networkIdManager.remove(networkId);
}

void Server::onClientDisconnect(IncomingMessage& inMessage)
{
	RemoteClient* client = m_clients.getClient(inMessage.address);
	if (client == nullptr)
	{
		return;
	}

	for (auto playerId : client->getPlayerIds())
	{
		m_game->onPlayerLeave(playerId);
	}

	client->sendMessage(new Message(MessageType::Disconnect));
	client->getConnection()->close();
}

void Server::onPlayerIntroduction(IncomingMessage& inMessage)
{
	RemoteClient* client = m_clients.getClient(inMessage.address);
	assert(client != nullptr);
	if (client->getNumPlayers() > 0)
	{
		return;
	}
	uint32_t numPlayers = 0;
	const uint32_t min = 1;
	serializeInt(inMessage.data, numPlayers, min, s_maxPlayersPerClient);

	if (numPlayers <= 0)
	{
		return;
	}

	assert(numPlayers <= 4);
	if (numPlayers < 1 || numPlayers > s_maxPlayersPerClient)
	{
		LOG_WARNING("Illegal number of players received from client %i", client->getNumPlayers())
		return;
	}
	
	// Introduce Players
	Message* outMessage = new Message(MessageType::AcceptPlayer);

	for (uint32_t i = 0; i < numPlayers; i++)
	{
		LOG_INFO("Giving player id %i", m_playerIdCounter);
		client->addPlayer(m_playerIdCounter);
		m_game->onPlayerJoin(m_playerIdCounter);
		int32_t playerId = static_cast<int32_t>(m_playerIdCounter);
		serializeInt(outMessage->data, playerId, 0, s_maxPlayersPerClient);
		++m_playerIdCounter;
	}

	client->sendMessage(outMessage);
}

void Server::onPlayerInput(IncomingMessage& message)
{
	RemoteClient* client = m_clients.getClient(message.address);
	if (client == nullptr)
	{
		LOG_WARNING("Server: onPlayerInput: Client non-existent (%s)", message.address.toString().c_str());
		return;
	}

	ActionBuffer playerActions;
	int32_t numFrames = 0;
	serializeInt(message.data, numFrames);

	int32_t startFrame = INDEX_NONE;
	serializeInt(message.data, startFrame);

	const int32_t numPlayers = client->getNumPlayers();
	for (int16_t i = 0; i < numFrames; i++)
	{
		const Sequence frameId = static_cast<Sequence>(startFrame) + i;
		for (int32_t j = 0; j < numPlayers; j++)
		{
			int32_t playerId = INDEX_NONE;
			serializeInt(message.data, playerId, 0, s_maxPlayersPerClient);
			if (playerId <= INDEX_NONE)
			{
				return;
			}
			playerActions.readFromMessage(message);
			m_game->processPlayerActions(playerActions, static_cast<int16_t>(playerId));
		}
	}
}

void Server::onEntityRequest(IncomingMessage& inMessage)
{
	RemoteClient* client = m_clients.getClient(inMessage.address);
	if (client == nullptr)
	{
		LOG_WARNING("Server: onEntityRequest: Client non-existent (%s)", inMessage.address.toString().c_str());
		return;
	}

	int32_t requestId = INDEX_NONE;
	serializeInt(inMessage.data, requestId, 0, s_maxNetworkedEntities);
	Entity* entity = findPtrByPredicate(EntityManager::getEntities().begin(), EntityManager::getEntities().end(),
		[requestId](Entity* entity) -> bool { return entity->getNetworkId() == requestId; });

	if (entity != nullptr)
	{
		sendEntitySpawn(entity, client);
	}
	else
	{
		LOG_DEBUG("Server::onEntityRequest: Entity netId %d not found", requestId);
	}
}

void Server::onEntitySpawnRequest(IncomingMessage& inMessage)
{
	RemoteClient* client = m_clients.getClient(inMessage.address);
	if (client == nullptr)
	{
		LOG_WARNING("Server: onEntityRequest: Client non-existent (%s)", inMessage.address.toString().c_str());
		return;
	}

	int32_t requestId = INDEX_NONE;
	serializeInt(inMessage.data, requestId, -s_maxSpawnPredictedEntities - 2, -2);
	acknowledgeEntitySpawn(inMessage, requestId, client);
}

void Server::onClientGameState(IncomingMessage& inMessage)
{
	RemoteClient* client = m_clients.getClient(inMessage.address);
	if (client == nullptr)
	{
		LOG_WARNING("Server: onClientGameState: unknown client (%s)", inMessage.address.toString().c_str());
		return;
	}

	int32_t numReceivedEntities = 0;

	serializeInt(inMessage.data, numReceivedEntities);
	std::vector<Entity*> entities = EntityManager::getEntities();
	for (int32_t i = 0; i < numReceivedEntities; i++)
	{
		int32_t networkId = INDEX_NONE;
		serializeInt(inMessage.data, networkId);
		assert(networkId != INDEX_NONE);
		Entity* entity = findPtrByPredicate(entities.begin(), entities.end(), [networkId](Entity* entity)
		{
			return entity->getNetworkId() == networkId;
		});
		if (entity != nullptr)
		{
			assert(client->ownsPlayer(entity->getOwnerPlayerId()));
			EntityManager::serializeClientVars(entity, inMessage.data);
		}
	}
}

void Server::onKeepAliveMessage(IncomingMessage& inMessage)
{
	if (RemoteClient* client = m_clients.getClient(inMessage.address))
	{
		client->sendMessage(new Message(MessageType::KeepAlive));
	}
}

void Server::sendEntitySpawn(Entity* entity, RemoteClient* client)
{
	assert(entity != nullptr);
	assert(entity->getNetworkId() > INDEX_NONE);

	Message* spawnMessage = new Message(MessageType::SpawnEntity);
	int32_t networkId = entity->getNetworkId();
	serializeInt(spawnMessage->data, networkId);

	if (!EntityManager::serializeFullEntity(entity, spawnMessage->data))
	{
		assert(false);
		return;
	}

	LOG_DEBUG("Server::sendEntitySpawn id: %d netId: %d", entity->getId(), entity->getNetworkId());
	client->sendMessage(spawnMessage);
}

void Server::sendEntitySpawn(Entity* entity)
{
	assert(entity != nullptr);
	assert(entity->getNetworkId() > INDEX_NONE);

	Message* message = new Message(MessageType::SpawnEntity);
	int32_t networkId = entity->getNetworkId();
	serializeInt(message->data, networkId);

	if(!EntityManager::serializeFullEntity(entity, message->data))
	{
		assert(false);
		return;
	}

	m_clients.sendMessage(message, true);
	//LOG_DEBUG("Server: spawning Entity id: %d netId: %d", entity->getId(), entity->getNetworkId());
}

void Server::acknowledgeEntitySpawn(IncomingMessage& inMessage, const int32_t tempId, RemoteClient* client)
{
	assert(client != nullptr);

	Entity* entity = nullptr;
	if (client->getId() == m_clients.getLocalClientId())
	{
		entity = findPtrByPredicate(EntityManager::getEntities().begin(), EntityManager::getEntities().end(), [tempId](Entity* ent)
		{
			return ent->getNetworkId() == tempId;
		});
		entity->setNetworkId(m_networkIdManager.getNext());
	}
	else
	{
		entity = EntityManager::instantiateEntity(inMessage.data, m_networkIdManager.getNext());
	}
	if (entity == nullptr)
	{
		return;
	}

	Message* outMessage = new Message(MessageType::AcceptEntity);

	int32_t remoteId = tempId;
	serializeInt(outMessage->data, remoteId);

	int32_t networkId = entity->getNetworkId();
	serializeInt(outMessage->data, networkId);

	//LOG_DEBUG("Server::acknowledgeEntitySpawn tempId_%d newId %d", remoteId, networkId);

	if (!EntityManager::serializeFullEntity(entity, outMessage->data))
	{
		entity->kill();
		assert(false);
		return;
	}

	client->sendMessage(outMessage);

	Message* spawnMessage = new Message(MessageType::SpawnEntity);
	//LOG_DEBUG("Server::acknowledgeEntitySpawn id: %d netId: %d", entity->getId(), entity->getNetworkId());
	serializeInt(spawnMessage->data, networkId);
	if (!EntityManager::serializeFullEntity(entity, spawnMessage->data))
	{
		entity->kill();
		assert(false);
		return;
	}

	for (auto& otherClient : m_clients)
	{
		if (otherClient.isUsed() && otherClient != *client
			&& otherClient.getId() != m_clients.getLocalClientId())
		{
			otherClient.getConnection()->sendMessage(spawnMessage);
		}
	}
}

void Server::onClientPing(IncomingMessage& message, const Time& time)
{
	uint64_t clientTimestamp = 0;
	message.data.serializeData(reinterpret_cast<char*>(&clientTimestamp), sizeof(uint64_t));

	Message* pongMessage = new Message(MessageType::ClockSync);
	pongMessage->data.serializeData(reinterpret_cast<const char*>(&clientTimestamp), sizeof(uint64_t));

	uint64_t serverTimestamp = time.getMilliSeconds();
	pongMessage->data.serializeData(reinterpret_cast<const char*>(&serverTimestamp), sizeof(uint64_t));

	RemoteClient* client = m_clients.getClient(message.address);
	client->getConnection()->sendMessage(pongMessage);
}

void Server::readMessage(IncomingMessage& message, const Time& time)
{
	switch (message.type)
	{	
		case MessageType::IntroducePlayer:
		{
			onPlayerIntroduction(message);
			break;
		}
		case MessageType::PlayerInput:
		{
			onPlayerInput(message);
			break;
		}
		case MessageType::RequestEntity:
		{
			onEntityRequest(message);
			break;
		}
		case MessageType::RequestEntitySpawn:
		{
			onEntitySpawnRequest(message);
			break;
		}
		case MessageType::Disconnect:
		{
			onClientDisconnect(message);
			break;
		}
		case MessageType::ClockSync:
		{
			onClientPing(message, time);
			break;
		}
		case MessageType::Snapshot:
		{
			onClientGameState(message);
			break;
		}
		case MessageType::KeepAlive:
		{
			onKeepAliveMessage(message);			
			break;
		}
		case MessageType::RequestConnection:
		case MessageType::None:
		case MessageType::AcceptClient:
		case MessageType::AcceptPlayer:
		case MessageType::SpawnEntity:
		case MessageType::AcceptEntity:
		case MessageType::DestroyEntity:
		case MessageType::GameEvent:
		case MessageType::NUM_MESSAGE_TYPES:
			break;
	}
}

void Server::createSnapshots(float deltaTime)
{
	m_snapshotTime += deltaTime;

	if (m_snapshotTime >= s_snapshotCreationRate)
	{
		m_snapshotTime -= s_snapshotCreationRate;
		const int32_t localClientId = m_clients.getLocalClientId();
		for (auto& client : m_clients)
		{
			if (client.isUsed() && client.getId() != localClientId)
			{
				writeSnapshot(client);
			}
		}
	}
}

void Server::writeSnapshot(RemoteClient& client)
{
	client.sendMessage(Snapshot::createMessage(EntityManager::getEntities()));
}

void Server::receivePackets()
{
	if (m_type == GameSessionType::Offline)
	{
		return;
	}

	m_packetReceiver->receivePackets(m_socket);

	Buffer<Packet*>&  packets = m_packetReceiver->getPackets();

	const int32_t numPackets = packets.getCount();

	for (int32_t i = 0; i < numPackets; i++)
	{
		Packet* packet = packets[i];

		if (m_type == GameSessionType::LAN && !packet->address.isFromLAN())
		{
			continue;
		}

		bool newConnection = true;
		if(RemoteClient* client = m_clients.getClient(packet->address))
		{
			newConnection = false;
			client->getConnection()->receivePacket(*packet);
		}

		if (newConnection && packet->header.numMessages > 0)
		{
			Message* message = packet->messages[0];
			if (message->type == MessageType::RequestConnection)
			{
				if (Connection* connection = addConnection(packet->address))
				{
					connection->receivePacket(*packet);
				}
			}
		}
	}

	packets.clear();
}

void Server::readMessages(const Time& time)
{
	for (RemoteClient& client : m_clients)
	{
		if (client.isUsed())
		{
			Connection* connection = client.getConnection();
			while (IncomingMessage* message = connection->getNextMessage())
			{
				readMessage(*message, time);
				message->type = MessageType::None;
			}
		}
	}
}

void Server::onConnectionCallback(ConnectionCallback type, Connection* connection)
{
	assert(connection != nullptr);

	switch (type)
	{
		case ConnectionCallback::ConnectionLost:
		{
			RemoteClient* client = m_clients.getClient(connection);
			assert(client != nullptr);

			for (auto playerId : client->getPlayerIds())
			{
				m_game->onPlayerLeave(playerId);
			}

			m_clients.remove(client);
			break;
		}
		case ConnectionCallback::ConnectionEstablished:
		case ConnectionCallback::ConnectionFailed:
		case ConnectionCallback::ConnectionReceived:
		{
			break;
		}
	}
}

Connection* Server::addConnection(const Address& address)
{
	using namespace std::placeholders;
	LOG_INFO("Server::addConnection");
	if (m_clients.count() < s_maxConnectedClients)
	{
		Connection* connection = new Connection(m_socket, address, 
			std::bind(&Server::onConnectionCallback, this, _1, _2));
		
		if (RemoteClient* client = m_clients.add(connection))
		{
			Message* message = new Message(MessageType::AcceptClient);
			int32_t clientId = client->getId();
			serializeInt(message->data, clientId, 0, s_maxConnectedClients);
			LOG_INFO("New ClientID: %d", clientId);

			client->sendMessage(message);

			connection->setState(Connection::State::Connected);
			return connection;
		}
		else
		{
			delete connection;
			assert(false);
		}
	}
	else
	{
		LOG_WARNING("connection attempt dropped, client limit reached");
	}

	return nullptr;
}
