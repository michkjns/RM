
#pragma once

#include <game/game_time.h>
#include <network/connection_callback.h>
#include <network/remote_client.h>

#include <array>

class Game;
class Entity;

namespace network
{
	static const uint32_t s_maxConnectedClients = 32;
	//=========================================================================

	struct IncomingMessage;
	class  Packet;
	class  PacketReceiver;
	class  Socket;

	class Server 
	{
	public:
		Server(Time& time, Game* game);
		~Server();
		void reset();

		void update();
		void fixedUpdate();

		void host(uint16_t port);

		void generateNetworkId(Entity* entity);
		void registerLocalClientId(int32_t clientId);
		void destroyEntity(int32_t networkId);

	private:
		RemoteClient* addClient(const Address& address, Connection* connection);
		
		void onPlayerIntroduction(IncomingMessage& inMessage);
		void onPlayerInput(IncomingMessage& inMessage);
		void onEntityRequest(IncomingMessage& inMessage);
		void onClientDisconnect(IncomingMessage& inMessage);
		void onClientGameState(IncomingMessage& inMessage);
		void onKeepAliveMessage(IncomingMessage& inMessage);

		void sendEntitySpawn(Entity* entity, RemoteClient* client);
		void sendEntitySpawn(Entity* entity);
		void acknowledgeEntitySpawn(IncomingMessage& inMessage, const int32_t tempId, RemoteClient* client);

		void onClientPing(IncomingMessage& message);

		void readMessage(IncomingMessage& message);
		void createSnapshots(float deltaTime);
		void writeSnapshot(RemoteClient& client);

		void updateConnections();
		void receivePackets();
		void readMessages();
		void sendMessages();
		void onConnectionCallback(ConnectionCallback type, 
			Connection* connection);

		void onConnectionRequest(const Address& address, Packet& packet);

		/** Finds unused remote client slot */
		RemoteClient* findUnusedClient();

		/** Gets remote client by address */
		RemoteClient* getClient(const Address& address);

		/** Gets remote client by connection*/
		RemoteClient* getClient(const Connection* connection);

		Socket* m_socket;
		Game*   m_game;
		bool    m_isInitialized;
		Time&   m_gameTime;
		int32_t m_clientIdCounter;
		int16_t m_playerIdCounter;
		int32_t m_networkIdCounter;
		int32_t m_localClientId;
		int32_t m_numClients;

		/* Time since last snapshot */
		float m_snapshotTime;

		/** Remote client buffer */
		std::array<RemoteClient, s_maxConnectedClients> m_clients;
		PacketReceiver* m_packetReceiver;
	};

}; // namespace network