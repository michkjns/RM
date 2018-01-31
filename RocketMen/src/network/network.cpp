
#include <network/network.h>

#include <core/debug.h>
#include <core/game.h>
#include <core/input.h>
#include <network/local_client.h>
#include <network/server.h>

using namespace network;

static LocalClient* s_client;
static Server* s_server;

bool Network::isClient()
{
	return s_client != nullptr;
}

bool Network::isServer()
{
	return s_server != nullptr;
}

void Network::generateNetworkId(Entity* entity)
{
	ASSERT(entity != nullptr);
	if (isServer())
	{
		s_server->generateNetworkId(entity);
	}
}

void Network::addLocalPlayer(int32_t controllerId)
{
	ASSERT(s_client != nullptr);
	ASSERT(controllerId >= 0 && controllerId <= input::NumSupportedControllers, "Invalid controllerId");
	s_client->addLocalPlayer(controllerId);
}

uint32_t Network::getNumLocalPlayers()
{
	if (s_client)
	{
		return s_client->getNumLocalPlayers();
	}

	return 0;
}

bool Network::isLocalPlayer(int16_t playerId)
{
	if (s_client != nullptr)
	{
		return s_client->isLocalPlayer(playerId);
	}

	return false;
}

void Network::destroyEntity(int32_t networkId)
{
	if (s_server && networkId > INDEX_NONE)
	{
		s_server->destroyEntity(networkId);
	}
}

void Network::setClient(LocalClient* client)
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
