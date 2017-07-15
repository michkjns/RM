
#include "server.h"

#include <core/entity.h>
#include <core/game.h>
#include <core/debug.h>
#include <network/remote_client.h>
#include <utility.h>

using namespace network;

Server::Server(Time& gameTime, Game* game) :
	m_isInitialized(false),
	m_gameTime(gameTime),
	m_game(game),
	m_clientIDCounter(0),
	m_playerIDCounter(0),
	m_networkIDCounter(0),
	m_numConnectedClients(0),
	m_lastOrderedMessaged(0),
	m_localClientID(INDEX_NONE),
	m_snapshotTime(0.0f),
	m_reliableMessageTime(0.0f),
	m_maxReliableMessageTime(0.10f),
	m_sequenceCounter(0)
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
		if (client.m_timeFromLastMessage - currentTime >= 10.0f)
		{
			LOG_INFO("Server: Client %i has timed out");
			client.m_id = INDEX_NONE; // TODO Properly disconnect the client here
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
	int32_t networkID = m_networkIDCounter++;
	entity->setNetworkID(networkID);

	NetworkMessage msg = {};
	msg.type           = MessageType::SpawnEntity;
	msg.isReliable     = true;
	msg.isOrdered      = true;

	WriteStream stream(32);
	Entity::serializeFull(entity, stream);
	msg.data.writeData((char*)stream.getBuffer(), stream.getLength());

	DEBUG_ONLY (
		int32_t entityID = entity->getID();
		ReadStream rs(32);
		memcpy(rs.getBuffer(), stream.getBuffer(), stream.getLength());
		Entity::serializeFull(entity, rs);
		assert(networkID == entity->getNetworkID());
		assert(entityID == entity->getID());
	)

	for (auto& client : m_clients)
	{
		if (client.isUsed())
		{
			client.queueMessage(msg, m_gameTime.getSeconds());
			DEBUG_ONLY(
				LOG_DEBUG("Server: spawning Entity ID: %d netID: %d", entity->getID(), networkID);
			)
		}
	}
}

void Server::registerLocalClientID(int32_t clientID)
{
	m_localClientID = clientID;
}

void Server::destroyEntity(int32_t networkID)
{
	NetworkMessage msg = {};
	msg.type           = MessageType::DestroyEntity;
	msg.isReliable     = true;
	msg.isOrdered      = true;
	msg.data.writeInt32(networkID);
	for (auto& client : m_clients)
	{
		if (client.isUsed())
		{
			client.queueMessage(msg, m_gameTime.getSeconds());
		}
	}
}

void Server::onClientConnect(IncomingMessage& msg)
{
	LOG_DEBUG("A client is attempting to connect");

	if (m_numConnectedClients >= s_maxConnectedClients)
	{
		return; // TODO: Send disconnect message
	}

	int32_t duplicatePeers = 0;
	if (RemoteClient* existingClient = getClient(msg.address))
	{
		if (existingClient->m_duplicatePeers >= s_maxDuplicatePeers)
		{
			LOG_INFO("Server: Client connection refused: Too many connections from the same machine.");
			return; // TODO: Send disconnect message
		}

		duplicatePeers = ++existingClient->m_duplicatePeers;
		//TODO: Fix duplicate client connection
	}
	
	// Accept the client
	RemoteClient* client = findUnusedClient();
	assert(client != nullptr);
	{
		client->m_address          = msg.address;
		client->m_id               = m_clientIDCounter++;
		client->m_duplicatePeers   = duplicatePeers;
		client->m_numPlayers       = 0;
		client->m_timeFromLastMessage  = 0.0f;
		m_numConnectedClients++;
		LOG_DEBUG("Number of clients increased, new value: %d", m_numConnectedClients);
	}
	
	NetworkMessage message = {};
		message.type = MessageType::AcceptClient;		
		message.data.writeInt32(client->m_id);
		message.isReliable = true;
	client->queueMessage(message, m_gameTime.getSeconds());
	LOG_DEBUG("Sent client ID  %d", client->m_id);
}

void Server::onClientDisconnect(const IncomingMessage& msg)
{
	__debugbreak();
}

void Server::onAckMessage(IncomingMessage& ackMessage, RemoteClient* client)
{
	assert(client != nullptr);

	const int32_t remoteSequence = ackMessage.sequence;
	const int32_t ackSeq = ackMessage.data.readInt32();
	const int32_t ackList = ackMessage.data.readInt32();
}

void Server::onPlayerIntroduction(IncomingMessage& msg)
{
	RemoteClient* client = getClient(msg.address);

	if (client->m_numPlayers > 0)
		return; // Already initialized

	uint32_t numPlayers = msg.data.readInt32();

	assert(numPlayers > 0);
	assert(numPlayers <= 4);
	if (numPlayers < 1 || numPlayers > s_maxPlayersPerClient)
	{
		LOG_WARNING("Illegal number of players received from client %i", client->m_id)
		return;
	}

	client->m_numPlayers = numPlayers;
	
	// Introduce Players
	NetworkMessage outMessage = {};
	outMessage.type = MessageType::AcceptPlayer;
	outMessage.isReliable = true;

	for (uint32_t i = 0; i < numPlayers; i++)
	{
		LOG_INFO("Giving player id %i", m_playerIDCounter);
		outMessage.data.writeInt32(m_playerIDCounter++);
	}

	client->queueMessage(outMessage, m_gameTime.getSeconds());
	m_game->onPlayerJoin();
}

void Server::onEntityRequest(IncomingMessage& msg)
{
	RemoteClient* client = getClient(msg.address);
	int32_t receivedID   = msg.data.readInt16();
	int32_t generatedID  = m_networkIDCounter++;
	
	const int32_t buflen = int32_t(msg.data.getLength()) - msg.data.getReadTotalBytes();
	ReadStream readStream(buflen);
	msg.data.readBytes((char*)readStream.getBuffer(), buflen);

	Entity* entity = Entity::instantiate(readStream);

	if (!entity)
	{
		LOG_WARNING("Failed to instantiate entity");
		return;
	}
	entity->setNetworkID(generatedID);

	NetworkMessage outMsg = {};
	outMsg.isOrdered      = false;
	outMsg.isReliable     = true;
	outMsg.type           = MessageType::AcceptEntity;
	outMsg.data.writeInt32(receivedID);
	outMsg.data.writeInt32(generatedID);

	WriteStream ws(128);
	if (!Entity::serializeFull(entity, ws))
	{
		entity->kill();
		assert(false);
		return;
	}

	outMsg.data.writeData((char*)ws.getBuffer(), ws.getLength());
	client->queueMessage(outMsg, m_gameTime.getSeconds());
	LOG_DEBUG("spawnMessage %d", generatedID);

	NetworkMessage spawnMsg = {};
	spawnMsg.isOrdered      = false;
	spawnMsg.isReliable     = true;
	spawnMsg.type           = MessageType::SpawnEntity;

	spawnMsg.data.writeInt32(generatedID);
	spawnMsg.data.writeData((char*)ws.getBuffer(), ws.getLength());

	for (auto& c : m_clients)
	{
		if (c.isUsed())
		{
			if (c == *client)
				continue;

			c.queueMessage(spawnMsg, m_gameTime.getSeconds());			
		}
	}
}

void Server::processMessage(IncomingMessage& msg)
{
	RemoteClient* client = getClient(msg.address);
	if (client && msg.type != MessageType::Ack)
	{
		for (auto i : client->m_recentlyProcessed)
		{
			if (i == msg.sequence)
				return; // Disregard duplicate message
			client->m_timeFromLastMessage = m_gameTime.getSeconds();
		}
	}

	switch (msg.type)
	{	
		/* Client to server */
		case MessageType::IntroducePlayer:
		{
			onPlayerIntroduction(msg);
			break;
		}
		case MessageType::PlayerInput:
		{
			break;
		}
		case MessageType::RequestEntity:
		{
			onEntityRequest(msg);
			break;
		}
		case MessageType::Ack:
		{
			LOG_DEBUG("onAckMessage");
			onAckMessage(msg, client);
			break;
		}
		/* Connection */
		case MessageType::ClientConnectRequest:
		{
			onClientConnect(msg);
			break;
		}
		case MessageType::ClientDisconnect:
		{
			break;
		}
		case MessageType::Clear:
		case MessageType::Ping:
		case MessageType::AcceptClient:
		case MessageType::AcceptPlayer:
		case MessageType::Gamestate:
		case MessageType::SpawnEntity:
		case MessageType::AcceptEntity:
		case MessageType::DestroyEntity:
		case MessageType::GameEvent:
		case MessageType::NUM_MESSAGE_TYPES:
			break;
	}
	//if (client != nullptr)
	//{
	//	client->m_recentlyProcessed[client->m_nextProcessed++] = msg.sequenceNr;
	//	if (client->m_nextProcessed >= 32) client->m_nextProcessed = 0;
	//}
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

void Server::processOutgoingMessages(float deltaTime)
{
	const float currentTime = m_gameTime.getSeconds();

	m_snapshotTime        += deltaTime;
	m_reliableMessageTime += deltaTime;

	if (m_snapshotTime >= s_snapshotCreationRate)
	{
		m_snapshotTime -= s_snapshotCreationRate;
		writeSnapshot();
	}
	
	/* Requeue reliable messages older than 1.0s */
	for (auto& client : m_clients)
	{
		for (NetworkMessage& msg : client.m_reliableBuffer)
		{
			if (msg.type != MessageType::Clear)
			{
				if (currentTime - msg.timeOfCreation >= 1.0f)
				{
					//	client.queueMessage(msg, currentTime);
					msg.type = MessageType::Clear;
				}
			}
		}
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
		if(client.isUsed() && client.m_id != m_localClientID)
		{
			writeSnapshot(client);
		}
	}
}

void Server::writeSnapshot(RemoteClient& client)
{
	std::vector<Entity*>& entityList = Entity::getList();
	WriteStream writeStream(512);  // TODO Make a constant variable for snapshot size, maybe a snapshot class/struct?

	for (uint32_t netID = 0; netID < s_maxNetworkedEntities; netID++)
	{
		Entity* netEntity = findPtrByPredicate(entityList.begin(), entityList.end(),
			[netID](Entity* entity) -> bool { return entity->getNetworkID() == netID; });

		bool writeEntity = netEntity != nullptr;
		serializeBit(writeStream, writeEntity);
		if (writeEntity)
		{
			Entity::serialize(netEntity, writeStream);
		}
	}

	NetworkMessage msg = {};
	msg.type           = MessageType::Gamestate;
	msg.isOrdered      = true;
	msg.isReliable     = false;
	msg.data.writeBuffer((char*)writeStream.getBuffer(), writeStream.getLength());

	client.queueMessage(msg, m_gameTime.getSeconds());
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
				if (msg.type == MessageType::Clear) 
					continue;

				messages.push_back(msg);
				msg.type = MessageType::Clear;
			}
			if (!messages.empty())
			{
				Packet* packet = new Packet;
				packet->header = {};
				packet->header.sequence = m_sequenceCounter++;

				for (auto& msg : messages)
				{
					packet->writeMessage(msg);
					msg.sequenceNr = packet->header.sequence;
				}
				
				m_networkInterface.sendPacket(client.m_address, packet);
				delete packet;
			}
			messages.clear();
		}
	}
}

RemoteClient* Server::findUnusedClient()
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
