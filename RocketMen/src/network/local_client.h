
#pragma once

#include <core/entity.h>
#include <core/keys.h>
#include <core/entity_manager.h>
#include <network/client_history.h>
#include <network/common_network.h>
#include <network/connection.h>
#include <network/connection_callback.h>
#include <network/message.h>
#include <network/client/message_factory_client.h>
#include <network/server/message_factory_server.h>
#include <network/session.h>
#include <utility/buffer.h>
#include <utility/circular_buffer.h>
#include <utility/id_manager.h>

#include <array>
#include <cstdint>
#include <functional>

class Game;
class Time;
class ActionBuffer;
//=============================================================================

namespace network 
{
	class PacketReceiver;
	struct LocalPlayer
	{
		LocalPlayer() : 
			playerId(INDEX_NONE),
			controllerId(INDEX_NONE),
			listenMouseKB(false) {}

		int16_t playerId;
		input::ControllerId controllerId;
		bool listenMouseKB;
	};

	class LocalClient
	{
	private:
		static const uint32_t s_sequenceMemorySize    = 256;
		static const uint32_t s_recentlyDestroyedSize = 32;

	public:
		LocalClient(Game* game);
		~LocalClient();

		enum class State
		{
			Disconnected,
			Connecting,
			Connected,
			Disconnecting
		};

	public:
		void setPort(uint16_t port);
		void update(const Time& time);
		void tick(Sequence frameId);

		void requestServerTime(const Time& localTime);
		void readInput();
		void connect(const Address& address, 
			std::function<void(Game*, JoinSessionResult)> callback);

		bool canConnect()    const { return m_state == LocalClient::State::Disconnected; }
		bool canDisconnect() const { return m_state != State::Connected && m_state != State::Connecting;}

		void disconnect();


		LocalPlayer& addLocalPlayer(int32_t controllerId, bool listenMouseKB = false);
		void requestEntity(int32_t netId);

		uint32_t getNumLocalPlayers() const;
		bool isLocalPlayer(int16_t playerId) const;
		LocalPlayer* getLocalPlayer(int16_t playerId) const;
		State getState() const;

	private:
		void sendPlayerActions();
		void readMessage(const Message& message, const Time& localTime);
		void onConnectionAccepted(const message::AcceptConnection& inMessage);
		void onAcceptPlayer(const message::AcceptPlayer& inMessage);
		void onSpawnEntity(const message::SpawnEntity& inMessage);
		void onDestroyEntity(const message::DestroyEntity& inMessage);
		void onSnapshot(const message::Snapshot& inMessage);
		void onServerTime(const message::ServerTime& inMessage, const Time& localTime);
		void onDisconnected();

		void sendMessage(Message* message);
		void sendPendingMessages(const Time& localTime);
		void setState(State state);
		void clearSession();

		void receivePackets();
		void readMessages(const Time& localTime);
		void onConnectionCallback(ConnectionCallback type, Connection* connection);
		bool shouldSendInput() const;

		Socket*         m_socket;
		Game*           m_game;
		Connection*     m_connection;
		Sequence        m_lastReceivedSnapshotId;
		Sequence        m_lastFrameSent;
		Sequence        m_lastFrameSimulated;
		uint32_t        m_lastOrderedMessaged;
		State           m_state;
		float           m_timeSinceLastInputMessage;
		float           m_maxInputMessageSentTime;
		float           m_timeSinceLastClockSync;
		float           m_clockResyncTime;
		uint16_t        m_port;
		PacketReceiver* m_packetReceiver;

		CircularBuffer<int32_t> m_requestedEntities;
		Buffer<LocalPlayer>	m_localPlayers;	

		ClientHistory m_clientHistory;
		std::function<void(Game*, JoinSessionResult)> m_sessionCallback;

		MessageFactoryClient m_messageFactory;
		MessageFactoryServer m_receiveMessageFactory;

		IdManager m_tempNetworkIdManager;
	};
}; //namespace network