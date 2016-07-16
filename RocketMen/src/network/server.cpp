
#include <network/server.h>

#include <core/entity.h>
#include <core/game.h>
#include <core/debug.h>
#include <network/remote_client.h>

using namespace network;

Server::Server(Time& gameTime, Game* game) :
	m_isInitialized(false),
	m_gameTime(gameTime),
	m_game(game),
	m_clientIDCounter(0),
	m_playerIDCounter(0),
	m_objectIDCounter(0),
	m_numConnectedClients(0),
	m_lastOrderedMessaged(0),
	m_localClientID(-1),
	m_snapshotTime(0.0f),
	m_reliableMessageTime(0.0f),
	m_maxReliableMessageTime(0.20f)
{
}

Server::~Server()
{
}

bool Server::initialize()
{
	m_isInitialized = true;
	return true;
}

bool Server::isInitialized() const
{
	return m_isInitialized;
}

void Server::update()
{
	const float deltaTime   = m_gameTime.getDeltaSeconds();
	const float currentTime = m_gameTime.getSeconds();

	processIncomingMessages(deltaTime);

	for (auto& client : m_clients) 
	{
		if (client.m_timeFromLastMsg - currentTime >= 10.0f)
		{
			LOG_INFO("Server: Client %i has timed out");
			client.m_id = 0; // TODO Properly disconnect the client here
		}
	}

	processOutgoingMessages(deltaTime);
}

void Server::fixedUpdate()
{
}

void Server::host(uint32_t port)
{
	m_networkInterface.host(port);
	LOG_INFO("Server: Listening on port %d", port);
}

void Server::setReliableSendRate(float timesPerSecond)
{
	m_maxReliableMessageTime = 1.0f / timesPerSecond;
}

void Server::generateNetworkID(Entity* entity)
{
	int32_t id = m_objectIDCounter++;
	entity->setNetworkID(id);

	NetworkMessage msg = {};
	msg.type           = MessageType::SPAWN_ENTITY;
	msg.isReliable     = true;
	msg.isOrdered      = true;

	WriteStream stream = {};
	stream.m_bufferLength = 32;
	stream.m_buffer = new uint32_t[32];
	Entity::serializeFull(entity, stream);
	msg.data.writeData((char*)stream.m_buffer, stream.getLength());

	delete[] stream.m_buffer;

	for (auto& client : m_clients)
	{
		if (client.isUsed())
			client.queueMessage(msg);
	}
}

void Server::registerLocalClient(int32_t clientID)
{
	m_localClientID = clientID;
}

void Server::destroyEntity(int32_t networkID)
{
	NetworkMessage msg = {};
	msg.type           = MessageType::DESTROY_ENTITY;
	msg.isReliable     = true;
	msg.isOrdered      = true;
	msg.data.writeInt32(networkID);
	for (auto& client : m_clients)
	{
		if(client.isUsed())
			client.queueMessage(msg);
	}
}

void Server::onClientConnect(const IncomingMessage& msg)
{
	LOG_DEBUG("A client is attempting to connect");

	uint32_t duplicatePeers = 0;
	for (auto& client : m_clients)
	{
		if (client.m_id < 0) continue;

		if (client.m_address == msg.address)
		{
			if (client.m_duplicatePeers >= s_maxDuplicatePeers)
			{
				LOG_INFO("Server: Client connection refused: Too many connections from the same machine.");
				return;
			}
			else
			{
				duplicatePeers = ++client.m_duplicatePeers;
				break;
			}
		}
	}

	if (m_numConnectedClients < s_maxConnectedClients)
	{
		// Accept the client
		RemoteClient* client = FindUnusedClient();
		assert(client != nullptr);
		{
			client->m_address          = msg.address;
			client->m_id               = m_clientIDCounter++;
			client->m_duplicatePeers   = duplicatePeers;
			client->m_numPlayers       = 0;
			client->m_timeFromLastMsg = 0.0f;
			m_numConnectedClients++;
		}
		
		NetworkMessage message = {};
			message.type = MessageType::CLIENT_CONNECT_ACCEPT;		
			message.data.writeInt32(client->m_id);
			message.isReliable = true;
		client->queueMessage(message);
	}
	else
	{
		// Disconnect the client
		__debugbreak();
	}
}

void Server::onClientDisconnect(const IncomingMessage& msg)
{
	__debugbreak();
}

void Server::onAckMessage(const IncomingMessage& msg)
{
}

void Server::onPlayerIntroduction(IncomingMessage& msg)
{
	// Read player count
	RemoteClient* client = getClient(msg.address);

	if (client->m_numPlayers > 0)
		return; // Already initialized, dismiss duplicate packet

	uint32_t numPlayers = msg.data.readInt32();

	assert(numPlayers > 0);
	assert(numPlayers <= 4);
	if (numPlayers < 1 || numPlayers > s_maxPlayersPerClient)
	{
		LOG_WARNING("Illegal number of players received from client %i",
					client->m_id)
		return;
	}

	client->m_numPlayers = numPlayers;
	
	// Introduce Players
	NetworkMessage outMessage = {};
	outMessage.type = MessageType::PLAYER_JOIN_ACCEPT;
	outMessage.isReliable = true;

	for (uint32_t i = 0; i < numPlayers; i++)
	{
		LOG_INFO("Giving player id %i", m_playerIDCounter);
		outMessage.data.writeInt32(m_playerIDCounter++);
	}

	client->queueMessage(outMessage);
	m_game->onPlayerJoin();
}

void Server::onEntityRequest(IncomingMessage& msg)
{
	RemoteClient* client = getClient(msg.address);
	int32_t receivedID   = msg.data.readInt16();
	int32_t generatedID  = m_objectIDCounter++;
	
	ReadStream stream = {};
	int32_t buflen = int32_t(msg.data.getLength()) - msg.data.getReadTotalBytes();
	stream.m_bufferLength = buflen;
	stream.m_buffer = new uint32_t[stream.m_bufferLength];
	msg.data.readBytes((char*)stream.m_buffer, buflen);

	Entity* entity = Entity::instantiate(stream);
	delete[] stream.m_buffer;

	if (!entity)
	{
		LOG_WARNING("Failed to instantiate entity");
		return;
	}
	entity->setNetworkID(generatedID);

	NetworkMessage outMsg = {};
	outMsg.isOrdered      = false;
	outMsg.isReliable     = true;
	outMsg.type           = MessageType::APPROVE_ENTITY;
	outMsg.data.writeInt32(receivedID);
	outMsg.data.writeInt32(generatedID);

	WriteStream ws = {};
	ws.m_bufferLength = 128;
	ws.m_buffer = new uint32_t[128];
	if (!Entity::serializeFull(entity, ws))
	{
		entity->kill();
		delete[] ws.m_buffer;
		assert(false);
		return;
	}
	outMsg.data.writeData((char*)ws.m_buffer, ws.getLength());
	client->queueMessage(outMsg);

	NetworkMessage spawnMsg = {};
	spawnMsg.isOrdered      = false;
	spawnMsg.isReliable     = true;
	spawnMsg.type           = MessageType::SPAWN_ENTITY;

	spawnMsg.data.writeInt32(generatedID);
	spawnMsg.data.writeData((char*)ws.m_buffer, ws.getLength());
	delete[] ws.m_buffer;
	for (auto& c : m_clients)
	{
		if (c.isUsed())
		{
			if (c == *client)
				continue;

			c.queueMessage(spawnMsg);
		}
	}
}

void Server::processMessage(IncomingMessage& msg)
{
	RemoteClient* client = getClient(msg.address);
	if (client)
	{
		for (auto i : client->m_recentlyProcessed)
		{
			if (i == msg.sequenceNr)
				return; // Disregard duplicate message
			client->m_timeFromLastMsg = m_gameTime.getSeconds();
		}
	}

	switch (msg.type)
	{	
		/* Client to server */
		case MessageType::PLAYER_INTRO:
		{
			onPlayerIntroduction(msg);
			break;
		}
		case MessageType::PLAYER_INPUT:
		{
			break;
		}
		case MessageType::ENTITY_REQUEST:
		{
			onEntityRequest(msg);
			break;
		};

		/* Connection */
		case MessageType::CLIENT_CONNECT_REQUEST:
		{
			onClientConnect(msg);
			break;
		}
		case MessageType::CLIENT_DISCONNECT:
		{
			break;
		}

		default: break;
	}
	if (client != nullptr)
	{
		client->m_recentlyProcessed[client->m_nextProcessed++] = msg.sequenceNr;
		if (client->m_nextProcessed >= 32) client->m_nextProcessed = 0;
	}
}

void Server::processIncomingMessages(float deltaTime)
{
	m_networkInterface.update(deltaTime);
	std::queue<IncomingMessage>& orderedMessages   = m_networkInterface.getOrderedMessages();
	std::queue<IncomingMessage>& unorderedMessages = m_networkInterface.getMessages();

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

void Server::processOutgoingMessages(float deltaTime)
{
	m_snapshotTime        += deltaTime;
	m_reliableMessageTime += deltaTime;

	if (m_snapshotTime >= s_snapshotCreationRate)
	{
		m_snapshotTime -= s_snapshotCreationRate;
		writeSnapshot();
	}

	if (m_reliableMessageTime >= m_maxReliableMessageTime)
	{
		sendMessages();
	}
}

void Server::writeSnapshot()
{
	for (auto& client : m_clients)
	{
		if (!client.isUsed())
			continue;
		if (client.m_id == m_localClientID)
			continue;

		writeSnapshot(client);
	}
}

void Server::writeSnapshot(RemoteClient& client)
{
	std::vector<Entity*>& entityList = Entity::getList();
	WriteStream ws = {};
	ws.m_bufferLength = 256;
	ws.m_buffer = new uint32_t[256];

	for (uint32_t netID = 0; netID < s_maxNetworkedEntities; netID++)
	{
		bool written = false;
		for (auto& entity : entityList)
		{
			if (entity->getNetworkID() != netID)
				continue;

			written = true;
			serializeBit(ws, written);
			Entity::serialize(entity, ws);
		}
		if (!written)
			serializeBit(ws, written);
	}

	NetworkMessage msg = {};
	msg.type       = MessageType::SERVER_GAMESTATE;
	msg.data.writeBuffer((char*)ws.m_buffer, ws.getLength());
	delete[] ws.m_buffer;
	msg.isOrdered  = true;
	msg.isReliable = false;

	client.queueMessage(msg);
}

void Server::sendMessages()
{
	for (RemoteClient& client : m_clients)
	{
		if (client.isUsed())
		{
			static std::vector<NetworkMessage> messages;
			for (NetworkMessage& msg : client.m_messageBuffer)
			{
				if (msg.type == MessageType::MESSAGE_CLEAR) 
					continue;

				messages.push_back(msg);
				msg.type = MessageType::MESSAGE_CLEAR;
			}
			m_networkInterface.sendMessages(client.m_address, messages);
			messages.clear();
		}
	}
}

RemoteClient* Server::FindUnusedClient()
{
	for (RemoteClient& client : m_clients)
	{
		if (client.m_id < 0)
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
			if (client.m_address == address)
			{
				return &client;
			}
		}
	}
	return nullptr;
}
