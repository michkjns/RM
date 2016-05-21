
#include <server.h>

#include <core/entity.h>
#include <core/game.h>
#include <debug.h>
#include <network/remote_client.h>

#include <assert.h>

using namespace network;

Server::Server(Time& gameTime, Game* game) :
	m_isInitialized(false),
	m_gameTime(gameTime),
	m_game(game),
	m_clientIDCounter(0),
	m_numConnectedClients(0),
	m_lastOrderedMessaged(0),
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

void Server::onClientConnect(const IncomingMessage& msg)
{
	LOG_DEBUG("A client is attempting to connect");

	uint32_t duplicatePeers = 0;
	for (auto client : m_clients)
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
			m_numConnectedClients++;
		}
		
		NetworkMessage message = {};
			message.type = MessageType::CLIENT_CONNECT_ACCEPT;		
			message.data = BitStream::create();
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
				LOG_DEBUG("Server: Message %i has been ACK'd", seq);
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
	client->m_numPlayers = msg.data->readInt32();
}

void Server::processMessage(const IncomingMessage& msg)
{
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

	if (m_reliableMessageTime >= m_maxReliableMessageTime)
	{
		sendMessages();
	}
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
				ackMsg.data = BitStream::create();
				ackMsg.data->writeInt32(static_cast<int32_t>(
					client.m_reliableAckList.size())); // write length
				for (auto seq : client.m_reliableAckList)
				{
					ackMsg.data->writeInt32(seq); // write seq
					LOG_DEBUG("Sending ack %i", seq);
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
				LOG_DEBUG("Sending msg %i", msg.type);
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
