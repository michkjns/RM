
#pragma once

#include <network/network_interface.h>
#include <network/network_message.h>

#include <array>
#include <cstdint>

class Game;
class Time;
class ActionBuffer;

namespace network 
{
	class Address;

	struct LocalPlayer
	{
		uint32_t playerID;
		uint32_t controllerID;
		bool     isUsed;
	};

	struct NetworkSession
	{
		Address serverAddress;
		bool isActive;
		uint32_t sequenceCounter;
		std::array<NetworkMessage, 64> messageBuffer;
	};

	class Client
	{
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
		void update();
		void fixedUpdate(ActionBuffer& actions);
		void connect(const Address& address);
		void queueMessage(const NetworkMessage& message);

		uint32_t getNumLocalPlayers() const;

	private:
		void onHandshake(const IncomingMessage& msg);
		void onAckMessage(const IncomingMessage& msg);

		void processIncomingMessages(float deltaTime);
		void processOutgoingMessages(float deltaTime);
		void processMessage(const IncomingMessage& msg);
		void sendInput();
		void setState(State state);
		void sendMessages();

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

		std::vector<int32_t> m_reliableAckList;
		std::array<LocalPlayer, s_maxPlayersPerClient> m_localPlayers;
	
	};
}; //namespace network