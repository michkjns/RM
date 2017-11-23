
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
	m_networkIdManager.reset();
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
	int32_t networkId = m_networkIdManager.next();
	LOG_DEBUG("netId: %d", networkId);
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
	Message message   = {};
	message.type      = MessageType::DestroyEntity;
	message.data.writeInt32(networkId);

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

	Message replyMessage = {};
	replyMessage.type = MessageType::Disconnect;
	client->getConnection()->sendMessage(replyMessage);
	client->getConnection()->close();
}

void Server::onPlayerIntroduction(IncomingMessage& inMessage)
{
	RemoteClient* client = m_clients.getClient(inMessage.address);
	assert(client != nullptr);
	if (client->getNumPlayers() > 0)
		return;

	const uint32_t numPlayers = inMessage.data.readInt32();

	assert(numPlayers > 0);
	assert(numPlayers <= 4);
	if (numPlayers < 1 || numPlayers > s_maxPlayersPerClient)
	{
		LOG_WARNING("Illegal number of players received from client %i", client->getNumPlayers())
		return;
	}
	
	// Introduce Players
	Message outMessage = {};
	outMessage.type    = MessageType::AcceptPlayer;

	for (uint32_t i = 0; i < numPlayers; i++)
	{
		LOG_INFO("Giving player id %i", m_playerIdCounter);
		client->addPlayer(m_playerIdCounter);
		m_game->onPlayerJoin(m_playerIdCounter);
		outMessage.data.writeInt16(m_playerIdCounter++);
	}

	client->getConnection()->sendMessage(outMessage);
}

void Server::onPlayerInput(IncomingMessage& message)
{
	RemoteClient* client = m_clients.getClient(message.address);
	if (client == nullptr)
	{
		LOG_WARNING("Server: onPlayerInput: Client non-existent (%s)", message.address.toString());
		return;
	}

	ActionBuffer playerActions;
	const int16_t numFrames = message.data.readInt16();
	const int16_t startFrame = message.data.readInt16();
	const int32_t numPlayers = client->getNumPlayers();
	for (int16_t i = 0; i < numFrames; i++)
	{
		const Sequence frameId = startFrame + i;
		for (int32_t j = 0; j < numPlayers; j++)
		{
			const int16_t playerId = message.data.readInt16();
			if (playerId <= INDEX_NONE)
			{
				return;
			}
			playerActions.readFromMessage(message);
			m_game->processPlayerActions(playerActions, playerId);
		}
	}
}

void Server::onEntityRequest(IncomingMessage& inMessage)
{
	RemoteClient* client = m_clients.getClient(inMessage.address);
	if (client == nullptr)
	{
		LOG_WARNING("Server: onEntityRequest: Client non-existent (%s)", inMessage.address.toString());
		return;
	}

	const int32_t requestId = inMessage.data.readInt32();
	if (requestId > INDEX_NONE)
	{
		Entity* entity = findPtrByPredicate(EntityManager::getEntities().begin(), EntityManager::getEntities().end(),
			[requestId](Entity* entity) -> bool { return entity->getNetworkId() == requestId; });
		if (entity != nullptr)
		{
			sendEntitySpawn(entity, client);
		}
	}
	else if(requestId < INDEX_NONE)
	{
		acknowledgeEntitySpawn(inMessage, requestId, client);
	}
}

void Server::onClientGameState(IncomingMessage& inMessage)
{
	RemoteClient* client = m_clients.getClient(inMessage.address);
	if (client == nullptr)
	{
		LOG_WARNING("Server: onClientGameState: unknown client (%s)", inMessage.address.toString());
		return;
	}

	int32_t numReceivedEntities = 0;
	ReadStream stream(32);
	inMessage.data.readToStream(stream);

	serializeInt(stream, numReceivedEntities);
	std::vector<Entity*> entities = EntityManager::getEntities();
	for (int32_t i = 0; i < numReceivedEntities; i++)
	{
		int32_t networkId = INDEX_NONE;
		serializeInt(stream, networkId);
		assert(networkId != INDEX_NONE);
		Entity* entity = findPtrByPredicate(entities.begin(), entities.end(), [networkId](Entity* entity)
		{
			return entity->getNetworkId() == networkId;
		});
		if (entity != nullptr)
		{
			assert(client->ownsPlayer(entity->getOwnerPlayerId()));
			EntityManager::serializeClientVars(entity, stream);
		}
	}

}

void Server::onKeepAliveMessage(IncomingMessage& inMessage)
{
	if (RemoteClient* client = m_clients.getClient(inMessage.address))
	{
		Message message = {};
		message.type = MessageType::KeepAlive;
		client->getConnection()->sendMessage(message);
	}
}

void Server::sendEntitySpawn(Entity* entity, RemoteClient* client)
{
	assert(entity != nullptr);
	assert(entity->getNetworkId() > INDEX_NONE);

	Message spawnMessage = {};
	spawnMessage.type = MessageType::SpawnEntity;

	WriteStream stream(32);
	int32_t networkId = entity->getNetworkId();
	serializeInt(stream, networkId);
	if (!EntityManager::serializeFullEntity(entity, stream))
	{
		assert(false);
		return;
	}
	spawnMessage.data.writeFromStream(stream);
	client->getConnection()->sendMessage(spawnMessage);
}

void Server::sendEntitySpawn(Entity* entity)
{
	assert(entity != nullptr);
	assert(entity->getNetworkId() > INDEX_NONE);

	Message message = {};
	message.type = MessageType::SpawnEntity;

	WriteStream stream(32);
	int32_t networkId = entity->getNetworkId();
	serializeInt(stream, networkId);
	if(!EntityManager::serializeFullEntity(entity, stream))
	{
		assert(false);
		return;
	}
	message.data.writeFromStream(stream);
	m_clients.sendMessage(message, true);
#ifdef _DEBUG
	LOG_DEBUG("Server: spawning Entity id: %d netId: %d", entity->getId(), entity->getNetworkId());
#endif // _DEBUG
}

void Server::acknowledgeEntitySpawn(IncomingMessage& inMessage, const int32_t tempId, RemoteClient* client)
{
	const int32_t bufferLength = int32_t(inMessage.data.getLength()) - inMessage.data.getReadTotalBytes();

	LOG_DEBUG("Server::acknowledgeEntitySpawn tempId_%d", tempId);

	ReadStream readStream(bufferLength);
	inMessage.data.readBytes((char*)readStream.getBuffer(), bufferLength);

	Entity* entity = EntityManager::instantiateEntity(readStream);
	if (entity == nullptr)
	{
		return;
	}

	const int32_t networkId = m_networkIdManager.next();
	entity->setNetworkId(networkId);

	Message outMessage = {};
	outMessage.type = MessageType::AcceptEntity;
	outMessage.data.writeInt32(tempId);
	outMessage.data.writeInt32(networkId); // TODO Compress in range (0, s_maxNetworkedEntities)

	WriteStream writeStream(128);
	if (!EntityManager::serializeFullEntity(entity, writeStream))
	{
		entity->kill();
		assert(false);
		return;
	}

	outMessage.data.writeFromStream(writeStream);
	client->getConnection()->sendMessage(outMessage);

	Message spawnMessage = {};
	spawnMessage.type = MessageType::SpawnEntity;

	spawnMessage.data.writeInt32(networkId);
	spawnMessage.data.writeFromStream(writeStream);

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
	const uint64_t clientTimestamp = message.data.readInt64();

	Message pongMessage = {};
	pongMessage.type = MessageType::ClockSync;
	pongMessage.data.writeInt64(clientTimestamp);
	pongMessage.data.writeInt64(time.getMilliSeconds());

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
		case MessageType::Gamestate:
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
	std::vector<Entity*>& entityList = EntityManager::getEntities();
	Snapshot snapshot(entityList);

	Message message = {};
	message.type    = MessageType::Gamestate;
	message.data.writeBuffer(snapshot.getBuffer(), snapshot.getSize());

	client.getConnection()->sendMessage(message);
}

void Server::receivePackets()
{
	if (m_type == GameSessionType::Offline)
	{
		return;
	}

	m_packetReceiver->receivePackets(m_socket);

	Buffer<Packet>&  packets   = m_packetReceiver->getPackets();
	Buffer<Address>& addresses = m_packetReceiver->getAddresses();

	const int32_t numClients = m_clients.count();
	const int32_t numPackets = packets.getCount();
	for (int32_t i = 0; i < numPackets; i++)
	{
		Packet& packet = packets[i];
		Address& address = addresses[i];

		if (m_type == GameSessionType::LAN && !address.isFromLAN())
		{
			continue;
		}

		bool newConnection = true;
		if(RemoteClient* client = m_clients.getClient(address))
		{
			newConnection = false;
			client->getConnection()->receivePacket(packet);
		}

		if (newConnection && numClients < s_maxConnectedClients)
		{
			IncomingMessage* message = packet.readNextMessage();
			if (message->type == MessageType::RequestConnection)
			{
				onConnectionRequest(address, packet);
			}
			delete message;
		}
	}

	packets.clear();
	addresses.clear();
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

void Server::onConnectionRequest(const Address& address, Packet& packet)
{
	if (m_clients.count() < s_maxConnectedClients)
	{
		Connection* connection = new Connection(m_socket, address, 
			std::bind(&Server::onConnectionCallback, this, std::placeholders::_1, std::placeholders::_2));
		
		if (RemoteClient* client = m_clients.add(connection))
		{
			Message message = {};
			message.type = MessageType::AcceptClient;
			message.data.writeInt32(client->getId());
			client->sendMessage(message);
			LOG_DEBUG("Sent clientId  %d", client->getId());

			connection->setState(Connection::State::Connected);
			packet.resetReading();
			connection->receivePacket(packet);
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
}
