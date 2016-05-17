
#pragma once

#include <game_time.h>
#include <network/network_interface.h>
#include <network/remote_client.h>

#include <array>
#include <cstdint>

class Game;

namespace network
{
	static const uint32_t s_maxConnectedClients = 32;

	//==========================================================================

	class Server
	{
	public:
		Server(Time& time, Game* game);
		~Server();

		bool initialize();
		void update();
		void fixedUpdate();

		void host(uint32_t port);

		void setReliableSendRate(float timesPerSecond);

	private:
		void onClientConnect(const IncomingMessage& msg);
		void onClientDisconnect(const IncomingMessage& msg);

		void onAckMessage(const IncomingMessage& msg);
		void onPlayerIntroduction(const IncomingMessage& msg);

		void processMessage(const IncomingMessage& msg);

		void processIncomingMessages(float deltaTime);
		void processOutgoingMessages(float deltaTime);

		void sendMessages();

		/** Finds unused remote client slot */
		RemoteClient* FindUnusedClient();

		/** Gets remote client by address */
		RemoteClient* getClient(const Address& address);

		NetworkInterface m_networkInterface;
		Game*            m_game;
		bool             m_isInitialized;
		Time&            m_gameTime;
		uint32_t         m_clientIDCounter;
		uint32_t         m_numConnectedClients;
		uint32_t         m_lastOrderedMessaged;
		uint64_t         m_simulatedTime;

		/* Time since last snapshot */
		float m_snapshotTime;

		/** Time since last reliable message release */
		float m_reliableMessageTime;
		
		/** Max reliable message wait time */
		float m_maxReliableMessageTime;

		/** Remote client buffer */
		std::array<RemoteClient, s_maxConnectedClients> m_clients;
	};

}; // namespace network