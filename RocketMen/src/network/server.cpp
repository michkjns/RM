
#include "server.h"

#include <core/entity.h>
#include <core/entity_manager.h>
#include <core/game.h>
#include <core/debug.h>
#include <core/input_buffer.h>
#include <network/common_network.h>
#include <network/connection.h>
#include <network/remote_client.h>
#include <network/snapshot.h>
#include <network/socket.h>

#include <utility.h>

extern "C" unsigned long crcFast(unsigned char const message[], int nBytes);

using namespace network;

Server::Server(Time& gameTime, Game* game) :
	m_isInitialized(false),
	m_gameTime(gameTime),
	m_game(game),
	m_clientIdCounter(0),
	m_playerIdCounter(0),
	m_networkIdCounter(0),
	m_localClientId(INDEX_NONE),
	m_numClients(0),
	m_snapshotTime(0.0f)
{
	m_socket = Socket::create();
	assert(m_socket != nullptr);
}

Server::~Server()
{
	delete m_socket;
}

void Server::update()
{
	if (m_socket->isInitialized())
	{
		receivePackets();
		readMessages();
		createSnapshots(m_gameTime.getDeltaSeconds());
		sendMessages();
		updateConnections();
	}
}

void Server::fixedUpdate()
{
}

void Server::host(uint16_t port)
{
	assert(m_socket != nullptr);

	if (!ensure(m_socket->isInitialized() == false))
	{
		LOG_WARNING("Server: Already listening to connections on port %d", m_socket->getPort());
		return;
	}

	if (m_socket->initialize(port))
	{
		LOG_INFO("Server: Listening on port %d", port);
	}
	else
	{
		LOG_WARNING("Server: Failed to listen on port %d", port);
	}
}

void Server::generateNetworkId(Entity* entity)
{
	int32_t networkId = m_networkIdCounter++;
	entity->setNetworkId(networkId);

	Message message = {};
	message.type    = MessageType::SpawnEntity;

	WriteStream stream(32);
	EntityManager::serializeFullEntity(entity, stream);
	message.data.writeData((char*)stream.getBuffer(), stream.getLength());

	for (auto& client : m_clients)
	{
		if (client.isUsed() && client.getId() != m_localClientId)
		{
			client.getConnection()->sendMessage(message);
#ifdef _DEBUG
				LOG_DEBUG("Server: spawning Entity id: %d netId: %d", entity->getId(), networkId);
#endif // _DEBUG
		}
	}
}

void Server::registerLocalClientId(int32_t clientId)
{
	m_localClientId = clientId;
}

void Server::destroyEntity(int32_t networkId)
{
	assert(networkId < s_maxNetworkedEntities);
	Message message   = {};
	message.type      = MessageType::DestroyEntity;
	message.data.writeInt32(networkId);
	for (auto& client : m_clients)
	{
		if (client.isUsed() && client.getId() != m_localClientId)
		{
			client.getConnection()->sendMessage(message);
		}
	}
}

RemoteClient* Server::addClient(const Address& address, Connection* connection)
{
	assert(connection != nullptr);
	LOG_DEBUG("Server: Adding new client");
	
	if (RemoteClient* existingClient = getClient(address))
	{
		LOG_DEBUG("Server: Adding new client error: address duplicate");
		return nullptr;
	}
	
	// Accept the client
	RemoteClient* client = findUnusedClient();
	assert(client != nullptr);
	client->initialize(m_clientIdCounter++, connection);
	
	Message message = {};
	message.type = MessageType::AcceptClient;
	message.data.writeInt32(client->getId());
	client->getConnection()->sendMessage(message);
	LOG_DEBUG("Sent clientId  %d", client->getId());
	return client;
}

void Server::onClientDisconnect(const IncomingMessage& /*inMessage*/)
{
}

void Server::onPlayerIntroduction(IncomingMessage& inMessage)
{
	RemoteClient* client = getClient(inMessage.address);
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

	client->setNumPlayers(numPlayers);
	
	// Introduce Players
	Message outMessage = {};
	outMessage.type    = MessageType::AcceptPlayer;

	for (uint32_t i = 0; i < numPlayers; i++)
	{
		LOG_INFO("Giving player id %i", m_playerIdCounter);
		m_game->onPlayerJoin(m_playerIdCounter);
		outMessage.data.writeInt32(m_playerIdCounter++);
	}

	client->getConnection()->sendMessage(outMessage);
}

void Server::onPlayerInput(IncomingMessage& message)
{
	RemoteClient* client = getClient(message.address);
	if (client == nullptr)
	{
		LOG_WARNING("Server: onPlayerInput: Client non-existent (%s)", message.address.toString());
		return;
	}

	ActionBuffer inputEvents;
	const int32_t playerId = inputEvents.readFromMessage(message);
	if (playerId != INDEX_NONE)
	{
		m_game->processInputEvents(inputEvents, playerId);
	}
}

void Server::onEntityRequest(IncomingMessage& inMessage)
{
	RemoteClient* client = getClient(inMessage.address);
	if (client == nullptr)
	{
		LOG_WARNING("Server: onEntityRequest: Client non-existent (%s)", inMessage.address.toString());
		return;
	}
	const int32_t tempNetworkId  = inMessage.data.readInt16();
	
	const int32_t bufferLength = int32_t(inMessage.data.getLength()) - inMessage.data.getReadTotalBytes();
	ReadStream readStream(bufferLength);
	inMessage.data.readBytes((char*)readStream.getBuffer(), bufferLength);

	Entity* entity = EntityManager::instantiateEntity(readStream);
	if (entity == nullptr)
	{
		return;
	}

	const int32_t networkId = m_networkIdCounter++;
	entity->setNetworkId(networkId);

	Message outMessage = {};
	outMessage.type    = MessageType::AcceptEntity;
	outMessage.data.writeInt32(tempNetworkId);
	outMessage.data.writeInt32(networkId); // TODO Compress in range (0, s_maxNetworkedEntities)

	WriteStream ws(128);
	if (!EntityManager::serializeFullEntity(entity, ws))
	{
		entity->kill();
		assert(false);
		return;
	}

	outMessage.data.writeData((char*)ws.getBuffer(), ws.getLength());
	client->getConnection()->sendMessage(outMessage);

	Message spawnMessage = {};
	spawnMessage.type    = MessageType::SpawnEntity;

	spawnMessage.data.writeInt32(networkId);
	spawnMessage.data.writeData((char*)ws.getBuffer(), ws.getLength());

	for (auto& otherClient : m_clients)
	{
		if (otherClient.isUsed() && otherClient != *client && otherClient.getId() != m_localClientId)
		{
			otherClient.getConnection()->sendMessage(spawnMessage);
		}
	}
}

void Server::onClientPing(IncomingMessage& message)
{
	const uint64_t clientTimestamp = message.data.readInt64();

	Message pongMessage = {};
	pongMessage.type = MessageType::ClockSync;
	pongMessage.data.writeInt64(clientTimestamp);
	pongMessage.data.writeInt64(m_gameTime.getMilliSeconds());

	RemoteClient* client = getClient(message.address);
	client->getConnection()->sendMessage(pongMessage);
}

void Server::readMessage(IncomingMessage& message)
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
			break;
		}
		case MessageType::ClockSync:
		{
			onClientPing(message);
			break;
		}
		case MessageType::RequestConnection:
		case MessageType::None:
		case MessageType::AcceptClient:
		case MessageType::AcceptPlayer:
		case MessageType::Gamestate:
		case MessageType::SpawnEntity:
		case MessageType::AcceptEntity:
		case MessageType::DestroyEntity:
		case MessageType::KeepAlive:
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
		for (auto& client : m_clients)
		{
			if (client.isUsed() && client.getId() != m_localClientId)
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

void Server::updateConnections()
{
	for (auto& client : m_clients)
	{
		if (client.isUsed() && client.getId() != m_localClientId)
		{
			client.getConnection()->update(m_gameTime);
		}
	}
}
void Server::receivePackets()
{
	assert(m_socket != nullptr);
	assert(m_socket->isInitialized());

	Address address;
	char    buffer[g_maxPacketSize];
	int32_t length = 0;

	while (m_socket->receive(address, buffer, length))
	{
		assert(length <= g_maxPacketSize);

		// Reconstruct packet
		BitStream stream;
		stream.writeBuffer(buffer, length);

		const uint32_t checksum = stream.readInt32();

		PacketHeader packetHeader;
		stream.readBytes((char*)&packetHeader, sizeof(PacketHeader));

		ChannelType channel = (packetHeader.sequence == -1 &&
			packetHeader.ackBits == -1 &&
			packetHeader.ackSequence == -1) ?
			ChannelType::Unreliable :
			ChannelType::ReliableOrdered;

		Packet packet(channel);
		packet.header = packetHeader;
		stream.readBytes(packet.getData(), packet.header.dataLength);

		// Write protocol ID after packet to include in the checksum
		memcpy(packet.getData() + packet.header.dataLength, &g_protocolId, sizeof(g_protocolId));

		if (checksum == crcFast((const unsigned char*)packet.getData(),
			packet.header.dataLength + sizeof(uint32_t)))
		{
			bool newConnection = true;
			for (auto& client : m_clients)
			{
				if (client.isUsed())
				{
					if (client.getConnection()->getAddress() == address)
					{
						newConnection = false;
						client.getConnection()->receivePacket(packet);
						break;
					}
				}
			}
			if (newConnection && m_numClients < s_maxConnectedClients)
			{
				IncomingMessage* message = packet.readNextMessage();
				if (message->type == MessageType::RequestConnection)
				{
					onConnectionRequest(address, packet);
				}
				delete message;
			}
		}
		else
		{
			LOG_DEBUG("PacketReceiver::receivePackets: Checksum mismatched, packet discarded.");
		}
	}
}

void Server::readMessages()
{
	for (RemoteClient& client : m_clients)
	{
		if (client.isUsed())
		{
			Connection* connection = client.getConnection();
			while (IncomingMessage* message = connection->getNextMessage())
			{
				readMessage(*message);
				message->type = MessageType::None;
			}
		}
	}
}

void Server::sendMessages()
{
	for (RemoteClient& client : m_clients)
	{
		if (client.isUsed())
		{
			client.getConnection()->sendPendingMessages(m_gameTime);
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
			if (RemoteClient* client = getClient(connection->getAddress()))
			{
				LOG_INFO("Server: Client %i has timed out", client->getId());
				client->clear();
			}
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
	if (m_numClients < s_maxConnectedClients)
	{
		Connection* connection = new Connection(m_socket, address, 
			std::bind(&Server::onConnectionCallback, this, std::placeholders::_1, std::placeholders::_2),
			false);
		
		if (addClient(address, connection))
		{
			connection->setState(Connection::State::Connected);
			packet.resetReading();
			connection->receivePacket(packet);
		}
		else
		{
			delete connection;
		}
	}
	else
	{
		LOG_WARNING("connection attempt dropped, client limit reached");
	}
}

RemoteClient* Server::findUnusedClient()
{
	for (RemoteClient& client : m_clients)
	{
		if (client.isAvailable())
		{
			return &client;
		}
	}
	return nullptr;
}

RemoteClient* Server::getClient(const Address& address)
{
	for (RemoteClient& client : m_clients)
	{
		if (client.isUsed())
		{
			if (client.getConnection()->getAddress() == address)
			{
				return &client;
			}
		}
	}
	return nullptr;
}
