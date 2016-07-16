
#include <network.h>

#include <network/client.h>
#include <network/server.h>

using namespace network;

static Client* s_client;
static Server* s_server;

bool Network::isClient()
{
	if(s_client)
		return s_client->isInitialized();
	return false;
}

bool Network::isServer()
{
	if(s_server)
		return s_server->isInitialized();
	return false;
}

void Network::generateNetworkID(Entity* entity)
{
	if(s_server)
		s_server->generateNetworkID(entity);
}

void Network::setLocalPlayers(uint32_t numPlayers)
{
	if (s_client)
		s_client->setLocalPlayers(numPlayers);
}

uint32_t Network::getNumLocalPlayers()
{
	if(s_client)
		return s_client->getNumLocalPlayers();

	return 0;
}

bool Network::isLocalPlayer(int32_t playerID)
{
	if (s_client)
	{
		return s_client->isLocalPlayer(playerID);
	}
	return false;
}

bool Network::requestEntity(Entity* entity)
{
	if (s_client)
		return s_client->requestEntity(entity);

	return false;
}

void Network::destroyEntity(int32_t networkID)
{
	if (s_server)
		s_server->destroyEntity(networkID);
}

void Network::setClient(Client* client)
{
	s_client = client;
}

void Network::setServer(Server* server)
{
	s_server = server;
}

Server* Network::getLocalServer()
{
	return s_server;
}
