
#pragma once

#include <network/address.h>

#include <cstdint>

static const uint16_t s_defaultServerPort = 4320;
static const uint16_t s_defaultClientPort = 0;

namespace network {
	class Client;
	class Server;
}; // namespace network

class Network
{
public:
	static bool isClient();
	static bool isServer();

	static void generateNetworkId(class Entity* entity);

	static void addLocalPlayer(int32_t controllerId);
	static uint32_t getNumLocalPlayers();
	static bool isLocalPlayer(int16_t playerId);

	static void destroyEntity(int32_t networkId);

protected:
	static void setClient(network::Client* client);
	static void setServer(network::Server* server);

	static class network::Server* getLocalServer();

public:
	//friend class Core;
	friend class Game;
	friend class network::Client;
	friend class network::Server;
};