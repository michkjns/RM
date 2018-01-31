
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
#include <network/client/message_factory_client.h>
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
	m_networkIdManager.clear();
	m_clients.clear();

	EntityManager::killEntities();
	
	delete m_socket;
	m_socket = Socket::create();
	ASSERT(m_socket != nullptr);
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
	ASSERT(m_socket != nullptr);
	ASSERT(type != GameSessionType::None);
	ASSERT(m_socket->isInitialized() == false, "Cannot host if a socket is already active");

	if (m_socket->initialize(port))
	{
		LOG_INFO("Server: Listening on port %d", port);
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
	ASSERT(networkId < s_maxNetworkedEntities);
	message::DestroyEntity* message = dynamic_cast<message::DestroyEntity*>(m_messageFactory.createMessage(MessageType::DestroyEntity));
	message->entityNetworkId = networkId;
	m_clients.sendMessage(message, true);
	m_networkIdManager.remove(networkId);
}

int32_t network::Server::getNumClients() const
{
	return m_clients.count();
}

void Server::onClientDisconnect(RemoteClient& client)
{
	for (auto playerId : client.getPlayerIds())
	{
		m_game->onPlayerLeave(playerId);
	}

	client.sendMessage(m_messageFactory.createMessage(MessageType::Disconnect));
	client.getConnection()->close();
}

void Server::onIntroducePlayer(const message::IntroducePlayer& inMessage, RemoteClient& client)
{
	if (client.getNumPlayers() > 0)
	{
		return;
	}

	if (inMessage.numPlayers < 1 || inMessage.numPlayers > s_maxPlayersPerClient)
	{
		LOG_WARNING("Illegal number of players received from client %i", client.getId())
		return;
	}
	
	// Introduce Players
	message::AcceptPlayer* outMessage = dynamic_cast<message::AcceptPlayer*>(m_messageFactory.createMessage(MessageType::AcceptPlayer));
	outMessage->numPlayers = inMessage.numPlayers;

	for (int32_t i = 0; i < inMessage.numPlayers; i++)
	{
		const int16_t newPlayerId = static_cast<int32_t>(m_playerIdCounter++);

		LOG_INFO("Giving player id %i", newPlayerId);
		client.addPlayer(newPlayerId);
		m_game->onPlayerJoin(newPlayerId);
		outMessage->playerIds[i] = newPlayerId;
	}

	client.sendMessage(outMessage);
}

void Server::onPlayerInput(const message::PlayerInput& inMessage, RemoteClient& client)
{
	ActionBuffer playerActions;

	const int32_t numPlayers = client.getNumPlayers();
	if (numPlayers != inMessage.numPlayers || inMessage.dataLength == 0)
	{
		return;
	}

	ReadStream stream(inMessage.data, inMessage.dataLength);

	for (int32_t i = 0; i < inMessage.numFrames; i++)
	{
		const Sequence frameId = static_cast<Sequence>(inMessage.startFrame + i);
		for (int32_t j = 0; j < numPlayers; j++)
		{
			playerActions.serialize(stream);
			m_game->processPlayerActions(playerActions, client.getPlayerIds()[j]);
			playerActions.clear();
		}
	}
}

void Server::onRequestTime(const message::RequestTime& inMessage, RemoteClient& client, const Time& time)
{	
	message::ServerTime* outMessage = dynamic_cast<message::ServerTime*>(m_messageFactory.createMessage(MessageType::ServerTime));
	outMessage->clientTimestamp = inMessage.clientTimestamp;
	outMessage->serverTimestamp = time.getMilliSeconds();

	client.getConnection()->sendMessage(outMessage);
}


void Server::onRequestEntity(const message::RequestEntity& inMessage, RemoteClient& client)
{
	const int32_t requestId = inMessage.entityNetworkId;

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

void Server::onKeepAlive(RemoteClient& client)
{
	client.sendMessage(m_messageFactory.createMessage(MessageType::KeepAlive));
}

void Server::sendEntitySpawn(Entity* entity, RemoteClient& client)
{
	ASSERT(entity != nullptr);
	ASSERT(entity->getNetworkId() > INDEX_NONE);

	message::SpawnEntity* spawnMessage = 
		dynamic_cast<message::SpawnEntity*>(m_messageFactory.createMessage(MessageType::SpawnEntity));

	spawnMessage->entity = entity;

	LOG_DEBUG("Server::sendEntitySpawn id: %d netId: %d", entity->getId(), entity->getNetworkId());
	client.sendMessage(spawnMessage);
}

void Server::sendEntitySpawn(Entity* entity)
{
	ASSERT(entity != nullptr);
	ASSERT(entity->getNetworkId() > INDEX_NONE);

	message::SpawnEntity* message = dynamic_cast<message::SpawnEntity*>(m_messageFactory.createMessage(MessageType::SpawnEntity));
	message->entity = entity;
	
	m_clients.sendMessage(message, true);
	//LOG_DEBUG("Server: spawning Entity id: %d netId: %d", entity->getId(), entity->getNetworkId());
}

void Server::readMessage(const Message& message, RemoteClient& client, const Time& time)
{
	switch (message.getType())
	{	
		case MessageType::IntroducePlayer:
		{
			onIntroducePlayer(static_cast<const message::IntroducePlayer&>(message), client);
			break;
		}
		case MessageType::PlayerInput:
		{
			onPlayerInput(static_cast<const message::PlayerInput&>(message), client);
			break;
		}
		case MessageType::RequestEntity:
		{
			onRequestEntity(static_cast<const message::RequestEntity&>(message), client);
			break;
		}
		case MessageType::Disconnect:
		{
			onClientDisconnect(client);
			break;
		}
		case MessageType::RequestTime:
		{
			onRequestTime(static_cast<const message::RequestTime&>(message), client, time);
			break;
		}
		case MessageType::KeepAlive:
		{
			onKeepAlive(client);
			break;
		}
		case MessageType::Snapshot:
		case MessageType::RequestConnection:
		case MessageType::None:
		case MessageType::AcceptConnection:
		case MessageType::AcceptPlayer:
		case MessageType::SpawnEntity:
		case MessageType::DestroyEntity:
		case MessageType::GameEvent:
		case MessageType::ServerTime:
		case MessageType::NUM_MESSAGE_TYPES:
		{
			break;
		}
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
				client.sendMessage(m_messageFactory.createMessage(MessageType::Snapshot));
			}
		}
	}
}

void Server::receivePackets()
{
	ASSERT(m_game->getSessionType() != GameSessionType::Offline);

	m_packetReceiver->receivePackets(m_socket, &m_clientMessageFactory);

	Buffer<Packet*>& packets = m_packetReceiver->getPackets();
	const int32_t numPackets = packets.getCount();

	for (int32_t i = 0; i < numPackets; i++)
	{
		Packet* packet = packets[i];

		bool newConnection = true;
		if(RemoteClient* client = m_clients.getClient(packet->address))
		{
			newConnection = false;
			client->getConnection()->receivePacket(*packet);
		}

		if (newConnection && packet->header.numMessages > 0)
		{
			Message* message = packet->messages[0];
			if (message->getType() == MessageType::RequestConnection)
			{
				if (Connection* connection = addConnection(packet->address))
				{
					connection->receivePacket(*packet);
				}
			}
		}
	}

	m_packetReceiver->clearPackets();
}

void Server::readMessages(const Time& time)
{
	for (RemoteClient& client : m_clients)
	{
		if (client.isUsed())
		{
			Connection* connection = client.getConnection();
			ASSERT(connection != nullptr);
			while (Message* message = connection->getNextMessage())
			{
				readMessage(*message, client, time);
				ASSERT(message->releaseRef(), "Message has unexpected dangling references");
			}
		}
	}
}

void Server::onConnectionCallback(ConnectionCallback type, Connection* connection)
{
	ASSERT(connection != nullptr);

	switch (type)
	{
		case ConnectionCallback::ConnectionLost:
		{
			RemoteClient* client = m_clients.getClient(connection);
			ASSERT(client != nullptr);

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

	if (m_clients.count() < s_maxConnectedClients)
	{
		Connection* connection = new Connection(m_socket, address, 
			std::bind(&Server::onConnectionCallback, this, _1, _2),
			m_messageFactory);
		
		if (RemoteClient* client = m_clients.add(connection))
		{
			message::AcceptConnection* message = 
				dynamic_cast<message::AcceptConnection*>(m_messageFactory.createMessage(MessageType::AcceptConnection));

			ASSERT(message != nullptr);

			message->clientId = client->getId();
			ASSERT(message->clientId >= 0 && message->clientId < s_maxConnectedClients, "ClientId out of range");
			LOG_INFO("Server::addConnection: New ClientId: %d", message->clientId);

			client->sendMessage(message);

			connection->setState(Connection::State::Connected);
			return connection;
		}
		else
		{
			delete connection;
			ASSERT(false, "Unexpected error adding new client");
		}
	}
	else
	{
		LOG_WARNING("connection attempt dropped, client limit reached");
	}

	return nullptr;
}
