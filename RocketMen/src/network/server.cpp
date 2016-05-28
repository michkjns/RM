
#include <network/server.h>

#include <core/entity.h>
#include <core/game.h>
#include <core/debug.h>
#include <network/remote_client.h>
#include <core/serialize.h>

#include <assert.h>

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
	const float deltaTime = m_gameTime.getDeltaSeconds();
	processIncomingMessages(deltaTime);
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
	msg.data           = new BitStream();

	entity->serializeFull(*msg.data);

	for (auto& client : m_clients)
	{
		client.queueMessage(msg);
	}	
}

void Server::registerLocalClient(int32_t clientID)
{
	m_localClientID = clientID;
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
			client->m_address        = msg.address;
			client->m_id             = m_clientIDCounter++;
			client->m_duplicatePeers = duplicatePeers;
			client->m_numPlayers     = 0;
			m_numConnectedClients++;
		}
		
		NetworkMessage message = {};
			message.type = MessageType::CLIENT_CONNECT_ACCEPT;		
			message.data = new BitStream();
			message.data->writeInt32(client->m_id);
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
	RemoteClient* client = getClient(msg.address);
	assert(msg.data != nullptr);
	assert(client != nullptr);

	int32_t count = msg.data->readInt32();
	for (int32_t i = 0; i < count; i++)
	{
		uint32_t seq = msg.data->readInt32();
		for (NetworkMessage& outMsg : client->m_messageBuffer)
		{
			if (outMsg.sequenceNr == seq)
			{
			//	LOG_DEBUG("Server: Message %i has been ACK'd", seq);
				outMsg.type = MessageType::MESSAGE_CLEAR;
				delete outMsg.data;
				outMsg.data = nullptr;
				break;
			}
		}
	}
}

void Server::onPlayerIntroduction(const IncomingMessage& msg)
{
	assert(msg.data != nullptr);
	assert(msg.data->getLength() == sizeof(int32_t));

	// Read player count
	RemoteClient* client = getClient(msg.address);

	if (client->m_numPlayers > 0)
		return; // Already initialized, dismiss duplicate packet

	uint32_t numPlayers = msg.data->readInt32();

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
	outMessage.data = new BitStream();
	for (uint32_t i = 0; i < numPlayers; i++)
	{
		LOG_INFO("Giving player id %i", m_playerIDCounter);
		outMessage.data->writeInt32(m_playerIDCounter++);
	}

	client->queueMessage(outMessage);
	m_game->onPlayerJoin();
}

void Server::onEntityRequest(const IncomingMessage& msg)
{
	//static int32_t nextID = 0;

	RemoteClient* client = getClient(msg.address);

	int32_t size = msg.data->readInt32();
	EntityInitializer* init = (EntityInitializer*)malloc(size);
	msg.data->readBytes((char*)init, size);

	//for (auto id : client->m_recentNetworkIDs)
	//{
	//	if (id == init->networkID)
	//	{
	//		// drop duplicate packet..
	//		free(init);
	//		return;
	//	}
	//}
	//client->m_recentNetworkIDs[nextID++] = init->networkID;
//	if (nextID >= 15) nextID = 0;
	//LOG_DEBUG("Send ent apprv %i", client->m_recentNetworkIDs[nextID - 1]);
	int32_t receivedID = init->networkID;
	LOG_DEBUG("Rec ent req %i", receivedID);

	init->networkID = m_objectIDCounter++;
	Entity* entity = Entity::instantiate(init);
	free(init);

	NetworkMessage outMsg = {};
	outMsg.isOrdered      = false;
	outMsg.isReliable     = true;
	outMsg.type           = MessageType::APPROVE_ENTITY;
	outMsg.data           = new BitStream();
	outMsg.data->writeInt32(receivedID);
	entity->serializeFull(*outMsg.data);
	client->queueMessage(outMsg);
}

void Server::processMessage(const IncomingMessage& msg)
{
	RemoteClient* client = getClient(msg.address);
	if (client)
	{
		for (auto i : client->m_recentlyProcessed)
		{
			if (i == msg.sequenceNr)
				return; // Disregard duplicate message
		}
	}

	switch (msg.type)
	{	
		case MessageType::MESSAGE_ACK:
		{
			onAckMessage(msg);
			break;
		}
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
	if (msg.isReliable)
	{
		LOG_DEBUG("Received reliable msg %i", msg.sequenceNr);
		RemoteClient* client = getClient(msg.address);
		if(client) client->m_reliableAckList.push_back(msg.sequenceNr);
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
	BitStream* stream = new BitStream();

	stream->writeInt32(m_objectIDCounter);

	for (auto& entity : entityList)
	{
		if (entity->getNetworkID() < 0)
			continue;

		stream->writeInt32(entity->getNetworkID());
		Serialize::serializePosition(stream, 
									 entity->getTransform().getLocalPosition());
		Serialize::serializeAngle(stream, 
								  entity->getTransform().getLocalRotation());
	}

	NetworkMessage msg = {};
	msg.type       = MessageType::SERVER_GAMESTATE;
	msg.data       = stream;
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

			// Create ACK message
			NetworkMessage ackMsg = {};
			ackMsg.type = MessageType::MESSAGE_ACK;
			ackMsg.isOrdered = true;
			if (client.m_reliableAckList.size() > 0)
			{
				ackMsg.data = new BitStream();
				ackMsg.data->writeInt32(static_cast<int32_t>(
					client.m_reliableAckList.size())); // write length
				for (auto seq : client.m_reliableAckList)
				{
					ackMsg.data->writeInt32(seq); // write seq
					//LOG_DEBUG("Sending ack %i", seq);
				}
				client.m_reliableAckList.clear();
				client.queueMessage(ackMsg);
			}

			// Other messages
			for (NetworkMessage& msg : client.m_messageBuffer)
			{
				if (msg.type == MessageType::MESSAGE_CLEAR) 
					continue;

				messages.push_back(msg);
				if (!msg.isReliable)
				{
					msg.type = MessageType::MESSAGE_CLEAR;
				}
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
