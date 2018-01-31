
#include <network/local_client.h>

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
#include <network/server/message_factory_server.h>
#include <utility/utility.h>

using namespace network;

static ActionBuffer s_playerActions[s_maxPlayersPerClient];

//=============================================================================

LocalClient::LocalClient(Game* game) :
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
	ASSERT(m_socket != nullptr, "Failed to create valid socket instance");
}

LocalClient::~LocalClient()
{
	delete m_socket;
	delete m_packetReceiver;
}

void LocalClient::setPort(uint16_t port)
{
	m_port = port;
}

void LocalClient::update(const Time& time)
{
	const float deltaTime = time.getDeltaSeconds();
	
	if (m_state == State::Disconnected)
	{
		return;
	}

	ASSERT(m_connection != nullptr);
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

			if (shouldSendInput())
			{
				sendPlayerActions();
			}
		}

		if (m_timeSinceLastClockSync > m_clockResyncTime)
		{
			requestServerTime(time);
		}		
	}

	sendPendingMessages(time);

	if (m_game->getSessionType() != GameSessionType::Offline)
	{
		m_connection->update(time);
	}

	if (m_state == State::Disconnected)
	{
		clearSession();
	}
}

void LocalClient::tick(Sequence frameCounter)
{
	Frame* currentFrame = m_clientHistory.insertFrame(frameCounter);
	ASSERT(currentFrame != nullptr);

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

	m_lastFrameSimulated = frameCounter;
}

void LocalClient::sendPlayerActions()
{
	const int32_t numFramesToSend = sequenceDifference(m_lastFrameSimulated, m_lastFrameSent);
	if (numFramesToSend > 0)
	{
		message::PlayerInput* message = dynamic_cast<message::PlayerInput*>(m_messageFactory.createMessage(MessageType::PlayerInput));
		const int32_t startFromFrame = static_cast<int32_t>(m_lastFrameSent + 1);

		message->numFrames = numFramesToSend;
		message->startFrame = startFromFrame;
		message->numPlayers = m_localPlayers.getCount();

		WriteStream stream(message::PlayerInput::maxDataLength);
		for (int16_t i = 0; i < numFramesToSend; i++)
		{
			const Sequence frameId = static_cast<Sequence>(startFromFrame) + i;
			Frame* frame = m_clientHistory.getFrame(frameId);
			ASSERT(frame != nullptr);
			for (int32_t j = 0; j < message->numPlayers; j++)
			{
				frame->actions[j].serialize(stream);
				frame->actions[j].clear();
			}
			m_lastFrameSent = frameId;
			if (stream.getBufferSize() - stream.getDataLength() < 32)
			{
				break;
			}
		}

		stream.flush();
		ASSERT(stream.getDataLength() < message::PlayerInput::maxDataLength);
		memcpy(message->data, stream.getData(), stream.getDataLength());
		message->dataLength = stream.getDataLength();

		sendMessage(message);
	}
}

void LocalClient::requestServerTime(const Time& localTime)
{
	message::RequestTime* message = dynamic_cast<message::RequestTime*>(m_messageFactory.createMessage(MessageType::RequestTime));
	message->clientTimestamp = localTime.getMilliSeconds();
	sendMessage(message);
}

void LocalClient::readInput()
{
	for (const LocalPlayer& player : m_localPlayers)
	{
		ActionBuffer& inputActions = s_playerActions[player.playerId];
		input::getActions(player.controllerId, inputActions, player.listenMouseKB);
	}
}

void LocalClient::connect(const Address& address, std::function<void(Game*, JoinSessionResult)> callback)
{
	using namespace std::placeholders;
	ASSERT(canConnect(), "LocalClient must be disconnected before calling connect()");
	ASSERT(m_connection == nullptr);
	ASSERT(m_socket->isInitialized() == false);

	if (m_socket->initialize(m_port))
	{
		m_sessionCallback = callback;
		m_connection = new Connection(m_socket, address, 
			std::bind(&LocalClient::onConnectionCallback, this, _1, _2), m_messageFactory);

		m_connection->tryConnect();
		m_state = LocalClient::State::Connecting;
	}
	else
	{
		LOG_ERROR("Client: Failed to initialize socket");
	}
}

void LocalClient::disconnect()
{
	ASSERT(canDisconnect(), "LocalCLient must be connected before able to disconnect");
	ASSERT(m_connection != nullptr);

	m_connection->sendMessage(m_messageFactory.createMessage(MessageType::Disconnect));
	setState(State::Disconnecting);
	m_connection->close();
}

LocalPlayer& LocalClient::addLocalPlayer(int32_t controllerId, bool enableMouseKB)
{
	ASSERT(m_localPlayers.getCount() < s_maxPlayersPerClient, "Too many local players were added");

	LocalPlayer& player = m_localPlayers.insert();
	player.controllerId = controllerId;
	player.listenMouseKB = enableMouseKB;
	return player;
}

void LocalClient::requestEntity(int32_t netId)
{
	ASSERT(netId > INDEX_NONE && netId < s_maxNetworkedEntities, "Invalid NetworkId provided");
	if (!m_requestedEntities.contains(netId))
	{
		m_requestedEntities.insert(netId);
		LOG_DEBUG("Client::requestEntity netID %d", netId);

		message::RequestEntity* message = dynamic_cast<message::RequestEntity*>(m_messageFactory.createMessage(MessageType::RequestEntity));
		message->entityNetworkId = netId;
		m_connection->sendMessage(message);
	}
}

uint32_t LocalClient::getNumLocalPlayers() const
{
	return m_localPlayers.getCount();
}

bool LocalClient::isLocalPlayer(int16_t playerId) const
{
	return getLocalPlayer(playerId) != nullptr;
}

LocalPlayer* LocalClient::getLocalPlayer(int16_t playerId) const
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

LocalClient::State LocalClient::getState() const
{
	return m_state;
}

void LocalClient::readMessage(const Message& message, const Time& localTime)
{
	switch (message.getType())
	{
		case MessageType::Snapshot:
		{
			if (sequenceLessThan(message.getId(), m_lastReceivedSnapshotId))
			{ // old, discard
				break;
			}
			
			m_lastReceivedSnapshotId = message.getId();
			onSnapshot(static_cast<const message::Snapshot&>(message));
			break;
		}

		case MessageType::SpawnEntity:
		{
			onSpawnEntity(static_cast<const message::SpawnEntity&>(message));
			break;
		}
		case MessageType::DestroyEntity:
		{
			onDestroyEntity(static_cast<const message::DestroyEntity&>(message));
			break;
		}
		case MessageType::AcceptConnection:
		{
			onConnectionAccepted(static_cast<const message::AcceptConnection&>(message));
			break;
		}
		case MessageType::AcceptPlayer:
		{
			onAcceptPlayer(static_cast<const message::AcceptPlayer&>(message));
			break;
		}
		case MessageType::ServerTime:
		{
			onServerTime(static_cast<const message::ServerTime&>(message), localTime);
			break;
		}
		case MessageType::Disconnect:
		{
			onDisconnected();
			break;
		}
		case MessageType::KeepAlive:
		{
			break;
		}

		case MessageType::RequestEntity:
		case MessageType::IntroducePlayer:
		case MessageType::PlayerInput:
		case MessageType::None:
		case MessageType::RequestConnection:
		case MessageType::GameEvent:
		case MessageType::RequestTime:
		case MessageType::NUM_MESSAGE_TYPES:
		{
			ASSERT(false, "Illegal MessageType received");
			break;
		}
	}
}

void LocalClient::onConnectionAccepted(const message::AcceptConnection& inMessage)
{
	ASSERT(m_state == State::Connecting);

	setState(State::Connected);
	LOG_INFO("Client: Connection established with the server. My ID: %d", inMessage.clientId);
	m_lastFrameSent = m_lastFrameSimulated;

	if (Server* localServer = Network::getLocalServer())
	{
		localServer->registerLocalClientId(inMessage.clientId);
	}

	m_sessionCallback(m_game, JoinSessionResult::Joined);

	message::IntroducePlayer* outMessage = dynamic_cast<message::IntroducePlayer*>(m_messageFactory.createMessage(MessageType::IntroducePlayer));
	outMessage->numPlayers = getNumLocalPlayers();

	sendMessage(outMessage);
}

void LocalClient::onAcceptPlayer(const message::AcceptPlayer& inMessage)
{
	const int32_t numPlayers = (int32_t)m_localPlayers.getCount();
	if (m_localPlayers[0].playerId != INDEX_NONE)
	{
		return;
	}
	
	if (inMessage.numPlayers != numPlayers)
	{
		return;
	}
	
	for (int32_t i = 0; i < numPlayers; i++)
	{
		m_localPlayers[i].playerId = inMessage.playerIds[i];
		LOG_INFO("Received player id %i", m_localPlayers[i].playerId);
	}
}

void LocalClient::onSpawnEntity(const message::SpawnEntity& inMessage)
{
	ASSERT(inMessage.entity != nullptr, "No valid entity provided by the message");

	const int32_t networkId = inMessage.entity->getNetworkId();
	LOG_DEBUG("Client: Received entity %d", networkId);

	int32_t index = m_requestedEntities.find(networkId);
	if (index != INDEX_NONE)
	{
		m_requestedEntities[index] = INDEX_NONE;
	}

	/*Entity* entity =*/ 
	//LOG_DEBUG("Client::onSpawnEntity ID: %d netID: %d", entity->getId(), entity->getNetworkId());
}

void LocalClient::onDestroyEntity(const message::DestroyEntity& inMessage)
{
	const int32_t networkId = inMessage.entityNetworkId;
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

void LocalClient::onSnapshot(const message::Snapshot& inMessage)
{
	if(inMessage.numMissingEntities > 0)
	{
		for (int32_t i = 0; i < inMessage.numMissingEntities; i++)
		{
			ASSERT(inMessage.missingEntityIds[i] != INDEX_NONE, "Entity must have a valid NetworkId");
			requestEntity(inMessage.missingEntityIds[i]);
		}
	}
}

void LocalClient::onServerTime(const message::ServerTime& inMessage, const Time& localTime)
{
	const uint64_t originalTime = inMessage.clientTimestamp;
	const uint64_t serverTime = inMessage.serverTimestamp;
	const uint64_t currentTime = localTime.getMilliSeconds();
	const uint64_t latency = currentTime - originalTime;

	// http://www.mine-control.com/zack/timesync/timesync.html
}

void LocalClient::onDisconnected()
{
	ASSERT(m_connection != nullptr);
	setState(LocalClient::State::Disconnected);
}

void LocalClient::sendMessage(Message* message)
{
	ASSERT(m_connection != nullptr);
	m_connection->sendMessage(message);
}

void LocalClient::sendPendingMessages(const Time& localTime)
{
	ASSERT(m_connection != nullptr);

	m_connection->sendPendingMessages(localTime);
}

void LocalClient::setState(State state)
{
	m_state = state;
}

void LocalClient::clearSession()
{
	m_localPlayers.clear();
	m_requestedEntities.fill(INDEX_NONE);
	
	delete m_connection;
	m_connection = nullptr;

	EntityManager::killEntities();
}

void LocalClient::receivePackets()
{
	m_packetReceiver->receivePackets(m_socket, &m_receiveMessageFactory);

	Buffer<Packet*>&  packets = m_packetReceiver->getPackets();

	const int32_t packetCount = packets.getCount();
	for (int32_t i = 0; i < packetCount; i++)
	{
		Packet* packet = packets[i];
		if (packet->address == m_connection->getAddress())
		{
			m_connection->receivePacket(*packet);
		}
	}

	m_packetReceiver->clearPackets();
}

void LocalClient::readMessages(const Time& localTime)
{
	while (Message* message = m_connection->getNextMessage())
	{
		readMessage(*message, localTime);
		ASSERT(message->getRefCount() == 1, "Message had dangling references at the end of their lifetime");
		message->releaseRef();
	}
}

void LocalClient::onConnectionCallback(ConnectionCallback type, Connection* connection)
{
	ASSERT(connection != nullptr);
	ASSERT(connection == m_connection);

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
			m_sessionCallback(m_game, JoinSessionResult::Failed);
			break;
		}
		case ConnectionCallback::ConnectionLost:
		{
			setState(State::Connecting);
			break;
		}
		case ConnectionCallback::ConnectionReceived:
		{
			ASSERT(false, "LocalClient is not allowed to receive incoming connections");
			break;
		}
	}
}

bool network::LocalClient::shouldSendInput() const
{
	return sequenceLessThan(m_lastFrameSimulated, m_lastFrameSent)
		&& m_game->getSessionType() != GameSessionType::Offline
		&& m_localPlayers.getCount() > 0
		&& m_localPlayers[0].playerId != INDEX_NONE;
}
