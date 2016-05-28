
#pragma once

#include <cstdint>

#include <network/address.h>

class Entity;
struct EntityInitializer;

namespace network {
	class Client;
	class Server;
};

class Network
{
public:
	static bool isClient();

	static void joinGame(const network::Address& address,
					 uint32_t numPlayers = 1);

	static void leaveGame();

	static void generateNetworkID(Entity* entity);

	static void setLocalPlayers(uint32_t numPlayers);
	static uint32_t getNumLocalPlayers();
	static bool isLocalPlayer(int32_t playerID);

	static void requestEntity(Entity* entity);

public:
	static bool isServer();
	static void createGame();

protected:
	static void setClient(network::Client* client);
	static void setServer(network::Server* server);

	static network::Server* getLocalServer();

public:
	friend class Core;
	friend class network::Client;
	friend class network::Server;
};