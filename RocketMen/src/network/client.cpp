
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
#include <network/socket.h>
#include <utility.h>

extern "C" unsigned long crcFast(unsigned char const message[], int nBytes);

using namespace network;

static const int16_t s_firstTempNetworkId = -2; // Reserve -1 for INDEX_NONE
static int16_t s_nextTempNetworkId = s_firstTempNetworkId;

Client::Client(Time& time, Game* game) :
	m_gameTime(time),
	m_game(game),
	m_connection(nullptr),
	m_lastReceivedState((Sequence)-1),
	m_lastOrderedMessaged(0),
	m_state(State::Disconnected),
	m_stateTimer(0.0f),
	m_messageSentTime(0.0f),
	m_maxMessageSentTime(0.05f),
	m_timeSinceLastClockSync(0.f),
	m_isInitialized(false),
	m_numLocalPlayers(0)
{
	clearSession();
	m_socket = Socket::create();
	assert(m_socket);
}

Client::~Client()
{
	delete m_socket;
}

bool Client::initialize(uint16_t port)
{
	clearSession();
	clearLocalPlayers();

	m_port = port;
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
		assert(m_connection != nullptr);

		m_connection->update(m_gameTime);
		receivePackets();
		readMessages();
		
		m_messageSentTime += deltaTime;

		if (m_timeSinceLastClockSync > 5.f)
		{
			requestServerTime();
		}

		if (m_messageSentTime >= m_maxMessageSentTime)
		{
			sendPendingMessages();
			m_messageSentTime = 0.f;
		}
	}

	m_stateTimer += deltaTime;
}

void Client::requestServerTime()
{
	OutgoingMessage pingMessage = {};
	pingMessage.type = MessageType::ClockSync;
	pingMessage.data.writeInt64(m_gameTime.getMilliSeconds());
	sendMessage(pingMessage);
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
	if (!ensure(m_state == Client::State::Disconnected))
	{
		LOG_ERROR("You must disconnect first before connecting!");
		return;
	}

	assert(m_connection == nullptr);
	assert(m_socket->isInitialized() == false);

	if (m_socket->initialize(m_port))
	{
		m_connection = new Connection(m_socket, address, 
			std::bind(&Client::onConnectionCallback, this, std::placeholders::_1, std::placeholders::_2));

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
	if (m_state != State::Connected || m_state != State::Connecting)
	{
		ensure(false);
		return;
	}

	setState(State::Disconnecting);
}

void Client::clearLocalPlayers()
{
	for (LocalPlayer& localPlayer : m_localPlayers)
	{
		localPlayer.controllerId = INDEX_NONE;
		localPlayer.playerId     = INDEX_NONE;
	}

	m_numLocalPlayers = 0;
}

bool Client::addLocalPlayer(int32_t controllerId)
{
	if (m_numLocalPlayers < s_maxPlayersPerClient)
	{
		m_localPlayers[m_numLocalPlayers++].controllerId = controllerId;
		return true;
	}

	return false;
}

bool Client::requestEntity(Entity* entity)
{
	const int16_t tempId = getNextTempNetworkId();
	entity->setNetworkId(static_cast<int32_t>(tempId));

	if (m_recentlyPredictedSpawns.contains(tempId) == false)
	{
		m_recentlyPredictedSpawns.insert(tempId);

		Message message = {};
		message.type = MessageType::RequestEntity;
		message.data.writeInt16(tempId);

		WriteStream stream(128);
		Entity::serializeFull(entity, stream);
		message.data.writeData((char*)stream.getBuffer(), stream.getLength());
	
		m_connection->sendMessage(message);
		return true;
	}

	return false;
}

uint32_t Client::getNumLocalPlayers() const
{
	return m_numLocalPlayers;
}

bool Client::isLocalPlayer(int32_t playerId) const
{
	for (auto player : m_localPlayers)
	{
		if (player.playerId == playerId)
		{
			return true;
		}
	}

	return false;
}

void Client::readMessage(IncomingMessage& message)
{
	//// Disregard duplicate messages
	//if (m_recentlyProcessed.contains(message.id))
	//{
	//	return;
	//}
	//
	//m_recentlyProcessed.insert(message.id);

	switch (message.type)
	{
		case MessageType::Gamestate:
		{
			if (sequenceLessThan(message.sequence, m_lastReceivedState))
			{ // discard packet
				break;
			}
			
			m_lastReceivedState = message.sequence;
			onGameState(message);
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
			onHandshake(message);
			break;
		}
		case MessageType::AcceptPlayer:
		{
			onAcceptPlayer(message);
			break;
		}
		case MessageType::ClockSync:
		{
			onReceiveServerTime(message);
			break;
		}

		case MessageType::RequestEntity:
		case MessageType::IntroducePlayer:
		case MessageType::PlayerInput:
		case MessageType::None:
		case MessageType::RequestConnection:
		case MessageType::Disconnect:
		case MessageType::GameEvent:
		case MessageType::KeepAlive:
		case MessageType::NUM_MESSAGE_TYPES:
		{
			break;
		}
	}
}

void Client::onHandshake(IncomingMessage& msg)
{
	if (!ensure(m_state == State::Connecting))
		return;

	int32_t id = msg.data.readInt32();
	LOG_INFO("Client: Received handshake from the server! I have received ID  %d\n", id);
	setState(State::Connected);

	if (Network::isServer())
	{
		Network::getLocalServer()->registerLocalClientId(id);
	}

	// Introduce Players
	Message outMessage   = {};
	outMessage.type      = MessageType::IntroducePlayer;
	outMessage.data.writeInt32(getNumLocalPlayers());
	
	//for (auto player : m_localPlayers)
	//{
	//	// Write player names etc
	//}

	sendMessage(outMessage);
}

void Client::onAcceptPlayer(IncomingMessage& msg)
{
	if (m_localPlayers[0].playerId < 0 == false)
	{
		ensure(false);
		return;
	}
	
	for (uint32_t i = 0; i < getNumLocalPlayers(); i++)
	{
		m_localPlayers[i].playerId = msg.data.readInt32();
		LOG_INFO("Received player id %i", m_localPlayers[i].playerId);
	}
}

void Client::onSpawnEntity(IncomingMessage& msg)
{
	ReadStream readStream(static_cast<int32_t>(msg.data.getLength()));
	msg.data.readBytes((char*)readStream.getBuffer(), msg.data.getLength() - msg.data.getReadTotalBytes());

#ifdef _DEBUG
	Entity* entity = Entity::instantiate(readStream);
	LOG_DEBUG("Client: Spawned entity ID: %d netID: %d", entity->getId(), entity->getNetworkId());
#else
	Entity::instantiate(readStream);
#endif // _DEBUG
}

void Client::onAcceptEntity(IncomingMessage& msg)
{
	int32_t localId  = msg.data.readInt32();
	int32_t remoteId = msg.data.readInt32();

	int32_t index = m_recentlyPredictedSpawns.find(localId);
	if (index != INDEX_NONE)
	{
		m_recentlyPredictedSpawns[index] = 0;
	}
	auto entities = Entity::getList();
	if (Entity* entity = findPtrByPredicate(entities.begin(), entities.end(),
		[localId](Entity* it) { return it->getNetworkId() == localId; } ))
	{
		if (m_recentlyDestroyedEntities.contains(remoteId))
		{
			entity->kill();
			return;
		}

		entity->setNetworkId(remoteId);
	}
	else
	{
		LOG_DEBUG("Client::onAcceptEntity: Unknown netID (%i)", localId);
	}
}

void Client::onDestroyEntity(IncomingMessage& msg)
{
	int32_t networkId = msg.data.readInt32();

	if (networkId < -s_maxSpawnPredictedEntities
		|| networkId > s_maxNetworkedEntities)
	{
		return;
	}

	for (auto& entity : Entity::getList())
	{
		if (entity->getNetworkId() == networkId)
		{
			entity->kill();
			return;
		}
	}

	// Not found, but remember it in case a spawn message comes in delayed
	//m_recentlyDestroyedEntities.insert(networkID); // TODO Remove once messages are ordered
} 

void Client::onGameState(IncomingMessage& msg)
{
	std::vector<Entity*>& entityList = Entity::getList();
	ReadStream readStream(512);

	msg.data.readBytes((char*)readStream.getBuffer(),
        msg.data.getLength() - msg.data.getReadTotalBytes());

	for (int32_t netId = 0; netId < s_maxNetworkedEntities; netId++)
	{
		bool readEntityData = false;
		serializeBit(readStream, readEntityData);
		if (readEntityData)
		{
			if (Entity* netEntity = findPtrByPredicate(entityList.begin(), entityList.end(), 
				[netId](Entity* entity) -> bool { return entity->getNetworkId() == netId; }))
			{
				Entity::serialize(netEntity, readStream);
			}
		}
	}
}

void Client::onReceiveServerTime(IncomingMessage& message)
{
	const uint64_t pingSentTime    = message.data.readInt64();
	const uint64_t pongReceiveTime = message.data.readInt64();
	const uint64_t currentTime     = m_gameTime.getMilliSeconds();
	const uint64_t latency         = currentTime - pingSentTime;

	// TODO resync game clock
	// http://www.mine-control.com/zack/timesync/timesync.html
}

void Client::sendMessage(Message& message)
{
	assert(m_connection != nullptr);
	m_connection->sendMessage(message);
}

void Client::sendPendingMessages()
{
	assert(m_connection != nullptr);
	m_connection->sendPendingMessages(m_gameTime);
}

void Client::sendInput()
{
}

void Client::setState(State state)
{
	m_state      = state;
	m_stateTimer = 0.0f;
}

void Client::clearSession()
{
	m_recentlyProcessed.fill(INDEX_NONE);
	m_recentlyDestroyedEntities.fill(INDEX_NONE);
	m_recentlyPredictedSpawns.fill(0);
	
	if (m_connection)
	{
		delete m_connection;
	}
}

int16_t Client::getNextTempNetworkId()
{
	int16_t tempNetworkId = s_nextTempNetworkId--;
	if (s_nextTempNetworkId <= -s_maxSpawnPredictedEntities - 1)
	{
		s_nextTempNetworkId = s_firstTempNetworkId;
	}

	return tempNetworkId;
}

void Client::receivePackets()
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

		ChannelType channel = (packetHeader.sequence    == (Sequence)-1 &&
			                   packetHeader.ackBits     == (uint32_t)-1 &&
			                   packetHeader.ackSequence == (Sequence)-1) ?
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
			if (address == m_connection->getAddress())
			{
				m_connection->receivePacket(packet);
			}
		}
		else
		{
			LOG_DEBUG("PacketReceiver::receivePackets: Checksum mismatched, packet discarded.");
		}
	}
}
void Client::readMessages()
{
	while (IncomingMessage* message = m_connection->getNextMessage())
	{
		readMessage(*message);
		message->type = MessageType::None;
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
		case ConnectionCallback::ConnectionLost:
		{
			LOG_INFO(type == ConnectionCallback::ConnectionLost ? 
				"Client: Lost connection to the server" : "Client: Failed to connect to the server");
			connection->close();
			setState(State::Disconnected);
			break;
		}
		case ConnectionCallback::ConnectionReceived:
		{
			assert(false);
			break;
		}
	}
}
