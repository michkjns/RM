
#pragma once

#include <game_time.h>
#include <network/network_interface.h>
#include <network/remote_client.h>

#include <array>

class Game;
class Entity;

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
		bool isInitialized() const;

		void update();
		void fixedUpdate();

		void host(uint32_t port);

		void setReliableSendRate(float timesPerSecond);

		void generateNetworkID(Entity* entity);
		void registerLocalClientID(int32_t clientID);
		void destroyEntity(int32_t networkID);

	private:
		void onClientConnect(IncomingMessage& msg);
		void onClientDisconnect(const IncomingMessage& msg);

		void onAckMessage(IncomingMessage& msg, RemoteClient* client);
		void onPlayerIntroduction(IncomingMessage& msg);
		void onEntityRequest(IncomingMessage& msg);

		void processMessage(IncomingMessage& msg);

		void processIncomingMessages(float deltaTime);
		void processOutgoingMessages(float deltaTime);

		void writeSnapshot();
		void writeSnapshot(RemoteClient& client);
		void sendMessages();

		/** Finds unused remote client slot */
		RemoteClient* findUnusedClient();

		/** Gets remote client by address */
		RemoteClient* getClient(const Address& address);

		NetworkInterface m_networkInterface;
		Game*            m_game;
		bool             m_isInitialized;
		Time&            m_gameTime;
		int32_t          m_clientIDCounter;
		int32_t          m_playerIDCounter;
		int32_t          m_networkIDCounter;
		uint32_t         m_numConnectedClients;
		uint32_t         m_lastOrderedMessaged;
		int32_t          m_localClientID;
		uint32_t		 m_sequenceCounter;

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