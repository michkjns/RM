
#pragma once

#include <core/game_time.h>
#include <network/connection_callback.h>
#include <network/remote_client_manager.h>
#include <network/server/message_factory_server.h>
#include <network/client/message_factory_client.h>
#include <network/session.h>
#include <utility/id_manager.h>

#include <array>

class Game;
class Entity;

namespace network
{
	static const uint32_t s_maxConnectedClients = 32;
	//=========================================================================

	struct Packet;
	class  PacketReceiver;
	class  Socket;

	class Server 
	{
	public:
		Server(Game* game);
		~Server();
		void reset();

		void update(const Time& time);
		void fixedUpdate();

		bool host(uint16_t port, GameSessionType type);

		void generateNetworkId(Entity* entity);
		void registerLocalClientId(int32_t clientId);
		void destroyEntity(int32_t networkId);

		int32_t getNumClients() const;

	private:
		void onIntroducePlayer(const message::IntroducePlayer& inMessage, RemoteClient& client);
		void onPlayerInput(const message::PlayerInput& inMessage, RemoteClient& client);
		void onRequestTime(const message::RequestTime& inMessage, RemoteClient& client, const Time& time);
		void onRequestEntity(const message::RequestEntity& inMessagem, RemoteClient& client);
		void onClientDisconnect(RemoteClient& client);
		void onKeepAlive(RemoteClient& client);

		void sendEntitySpawn(Entity* entity, RemoteClient& client);
		void sendEntitySpawn(Entity* entity);

		void readMessage(const Message& message, RemoteClient& client, const Time& time);
		void createSnapshots(float deltaTime);

		void receivePackets();
		void readMessages(const Time& time);
		void onConnectionCallback(ConnectionCallback type, 
			Connection* connection);

		Connection* addConnection(const Address& address);

		Socket* m_socket;
		Game*   m_game;
		bool    m_isInitialized;
		int16_t m_playerIdCounter;

		/* Time since last snapshot */
		float m_snapshotTime;

		PacketReceiver* m_packetReceiver;
		IdManager m_networkIdManager;
		RemoteClientManager m_clients;

		MessageFactoryServer m_messageFactory;
		MessageFactoryClient m_clientMessageFactory;
	};

}; // namespace network