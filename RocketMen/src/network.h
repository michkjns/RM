
#pragma once

#include <network/address.h>

#include <cstdint>

namespace network {
	class Client;
	class Server;
}; // namespace network

class Network
{
public:
	static bool isClient();
	static bool isServer();

	static void connect(const network::Address& address);
	static void disconnect();


	static void generateNetworkID(class Entity* entity);

	static bool addLocalPlayer(int32_t controllerID);
	static uint32_t getNumLocalPlayers();
	static bool isLocalPlayer(int32_t playerID);

	static bool requestEntity(class Entity* entity);
	static void destroyEntity(int32_t networkID);

protected:
	static void setClient(network::Client* client);
	static void setServer(network::Server* server);

	static class network::Server* getLocalServer();

public:
	friend class Core;
	friend class network::Client;
	friend class network::Server;
};