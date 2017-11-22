
#pragma once

#include <core/entity.h>
#include <core/keys.h>
#include <game/game_session.h>
#include <network/common_network.h>
#include <network/connection.h>
#include <network/connection_callback.h>
#include <network/network_message.h>
#include <network/client_history.h>
#include <utility/buffer.h>
#include <utility/circular_buffer.h>

#include <array>
#include <cstdint>

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

	class Client
	{
	private:
		static const uint32_t s_sequenceMemorySize    = 256;
		static const uint32_t s_recentlyDestroyedSize = 32;

	public:
		Client(Game* game);
		~Client();

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

		void requestServerTime(const Time& localTime);
		void readInput();
		void tick(Sequence frameId);
		void connect(const Address& address, 
			std::function<void(SessionResult)> callback);
		void disconnect();

		LocalPlayer& addLocalPlayer(int32_t controllerId, bool listenMouseKB = false);
		bool requestEntity(Entity* entity);
		void requestEntity(int32_t netId);

		uint32_t getNumLocalPlayers() const;
		bool isLocalPlayer(int16_t playerId) const;
		LocalPlayer* getLocalPlayer(int16_t playerId) const;
		State getState() const;

	private:
		void sendPlayerActions();
		void syncOwnedEntities(int16_t playerId);
		void readMessage(IncomingMessage& message, const Time& localTime);
		void onConnectionEstablished(IncomingMessage& message);
		void onAcceptPlayer(IncomingMessage& message);
		void onSpawnEntity(IncomingMessage& message);
		void onAcceptEntity(IncomingMessage& message);
		void onDestroyEntity(IncomingMessage& message);
		void onGameState(IncomingMessage& message);
		void onReceiveServerTime(IncomingMessage& message, const Time& localTime);
		void onDisconnected();

		void sendMessage(Message& message);
		void sendPendingMessages(const Time& localTime);
		void setState(State state);
		void clearSession();
		int32_t getNextTempNetworkId();

		void receivePackets();
		void readMessages(const Time& localTime);
		void onConnectionCallback(ConnectionCallback type, Connection* connection);

		Socket*         m_socket;
		Game*           m_game;
		Connection*     m_connection;
		Sequence        m_lastReceivedState;
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

		CircularBuffer<int32_t, s_sequenceMemorySize> 
			m_recentlyProcessed;

		CircularBuffer<int32_t, s_recentlyDestroyedSize>
			m_recentlyDestroyedEntities;

		CircularBuffer<int32_t, s_maxSpawnPredictedEntities>
			m_requestedEntities;

		Buffer<LocalPlayer>	m_localPlayers;	

		ClientHistory m_clientHistory;
		std::function<void(SessionResult)> m_sessionCallback;
	};
}; //namespace network