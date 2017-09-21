
#pragma once

#include <circular_buffer.h>
#include <core/entity.h>
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
	using NetworkMessageBuffer = 
		std::array<OutgoingMessage, s_maxPendingMessages>;
	
	struct LocalPlayer
	{
		int32_t playerID;
		int32_t controllerID;
	};

	struct NetworkSession
	{
		Address  serverAddress;
		uint32_t pendingMessages;
		uint32_t pendingReliable;
		uint32_t nextMessageID;
		bool     isActive;
		
		NetworkMessageBuffer outgoingMessages;
		NetworkMessageBuffer outgoingReliableMessages;
	};

	class Client
	{
	private:
		static const uint32_t s_sequenceMemorySize    = 256;
		static const uint32_t s_recentlyDestroyedSize = 32;

	public:
		Client(Time& time, Game* game);
		~Client();

		enum class State
		{
			Disconnected,
			Connecting,
			Connected,
			Disconnecting
		};

	public:
		bool initialize();
		bool isInitialized() const;
		void update();
		void fixedUpdate(ActionBuffer& actions);
		void connect(const Address& address);
		void disconnect();
		void queueMessage(const NetworkMessage& message);
		void queueMessage(const OutgoingMessage& message);
		void clearLocalPlayers();
		bool addLocalPlayer(int32_t controllerID);
		bool requestEntity(Entity* entity);

		uint32_t getNumLocalPlayers() const;
		bool isLocalPlayer(int32_t playerID) const;

	private:
		void onHandshake(IncomingMessage& msg);
		void onAcceptPlayer(IncomingMessage& msg);
		void onSpawnEntity(IncomingMessage& msg);
		void onAcceptEntity(IncomingMessage& msg);
		void onDestroyEntity(IncomingMessage& msg);
		void onGameState(IncomingMessage& msg);

		void processIncomingMessages(float deltaTime);
		void processOutgoingMessages(float deltaTime);
		void processMessage(IncomingMessage& msg);
		void sendInput();
		void setState(State state);
		void requeueReliableMessages();
		void sendMessages();
		void clearSession();
		int32_t getNextTempNetworkID();

		NetworkInterface m_networkInterface;
		Game*            m_game;
		Time&            m_gameTime;
		uint32_t         m_lastReceivedState;
		uint32_t         m_lastOrderedMessaged;
		NetworkSession   m_session;
		State            m_state;
		float            m_stateTimer;
		float            m_messageSentTime;
		float            m_maxMessageSentTime;
		uint32_t         m_connectionAttempt;
		bool             m_isInitialized;
		int32_t          m_numLocalPlayers;	

		CircularBuffer<int32_t, s_sequenceMemorySize> 
			m_recentlyProcessed;

		CircularBuffer<int32_t, s_recentlyDestroyedSize>
			m_recentlyDestroyedEntities;

		CircularBuffer<int32_t, s_maxSpawnPredictedEntities>
			m_recentlyPredictedSpawns;

		std::array<LocalPlayer, s_maxPlayersPerClient> 
			m_localPlayers;	
	};
}; //namespace network