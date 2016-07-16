
#pragma once

#include <core/entity.h>
#include <circular_buffer.h>
#include <network/network_interface.h>
#include <network/network_message.h>

#include <array>
#include <cstdint>

class Game;
class Time;
class ActionBuffer;

//==============================================================================

namespace network 
{
	class Address;

	struct LocalPlayer
	{
		int32_t playerID;
		int32_t controllerID;
		bool    isUsed;
	};

	struct NetworkSession
	{
		Address  serverAddress;
		uint32_t sequenceCounter;
		uint32_t pendingMessages;
		bool     isActive;
		std::array<NetworkMessage, 64> messageBuffer;
	};

	class Client
	{
	private:
		static const uint32_t s_sequenceMemorySize = 256;
		static const uint32_t s_recentlyDestroyedSize = 32;
	public:
		Client(Time& time, Game* game);
		~Client();

		enum class State
		{
			STATE_DISCONNECTED,
			STATE_CONNECTING,
			STATE_CONNECTED,
			STATE_DISCONNECTING
		};

	public:
		bool initialize();
		bool isInitialized() const;
		void update();
		void fixedUpdate(ActionBuffer& actions);
		void connect(const Address& address);
		void queueMessage(const NetworkMessage& message);
		void setLocalPlayers(uint32_t numPlayers);
		bool requestEntity(Entity* entity);

		uint32_t getNumLocalPlayers() const;
		bool     isLocalPlayer(int32_t playerID) const;

	private:
		void onHandshake(IncomingMessage& msg);
		void onAckMessage(const IncomingMessage& msg);
		void onPlayerAccepted(IncomingMessage& msg);
		void onSpawnEntity(IncomingMessage& msg);
		void onApproveEntity(IncomingMessage& msg);
		void onDestroyEntity(IncomingMessage& msg);
		void onGameState(IncomingMessage& msg);

		void processIncomingMessages(float deltaTime);
		void processOutgoingMessages(float deltaTime);
		void processMessage(IncomingMessage& msg);
		void sendInput();
		void setState(State state);
		void sendMessages();
		void clearSession();

		NetworkInterface m_networkInterface;
		Game*            m_game;
		Time&            m_gameTime;
		int32_t          m_lastReceivedState;
		uint32_t         m_lastOrderedMessaged;
		NetworkSession   m_session;
		State            m_state;
		float            m_stateTimer;
		float            m_messageSentTime;
		float            m_maxMessageSentTime;
		uint32_t         m_connectionAttempt;
		bool             m_isInitialized;

		CircularBuffer<int32_t, s_sequenceMemorySize> m_recentlyProcessed;

		CircularBuffer<int32_t, s_recentlyDestroyedSize>
		m_recentlyDestroyedEntities;

		CircularBuffer<int32_t, s_maxSpawnPredictedEntities>
		m_recentlyPredictedSpawns;

		std::vector<int32_t> m_reliableAckList;
		std::array<LocalPlayer, s_maxPlayersPerClient> m_localPlayers;	
	};
}; //namespace network