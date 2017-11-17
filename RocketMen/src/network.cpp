
#include <network.h>

#include <core/debug.h>
#include <network/client.h>
#include <network/server.h>

using namespace network;

static Client* s_client;
static Server* s_server;

bool Network::isClient()
{
	return (s_client && s_client->isInitialized());
}

bool Network::isServer()
{
	return (s_server != nullptr);
}

void Network::connect(const network::Address& address)
{
	assert(s_client != nullptr);
	LOG_INFO("Network: connecting to %s...", address.toString().c_str());
	s_client->connect(address);
}

void Network::disconnect()
{
	if (isClient())
	{
		s_client->disconnect();
	}
}

void Network::generateNetworkId(Entity* entity)
{
	assert(entity != nullptr);
	if (isServer())
	{
		s_server->generateNetworkId(entity);
	}
	else if (isClient())
	{
		if(!requestEntity(entity))
		{
			LOG_ERROR("Couldn't Request Entity!");
			entity->kill();
		}		
	}
}

void Network::addLocalPlayer(int32_t controllerId)
{
	assert(s_client != nullptr);
	assert(s_client->isInitialized());
	assert(controllerId >= 0);
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
	assert(s_client != nullptr);
	assert(s_client->isInitialized());

	return s_client->isLocalPlayer(playerId);
}

bool Network::requestEntity(Entity* entity)
{
	assert(s_client != nullptr);
	assert(s_client->isInitialized());

	return s_client->requestEntity(entity);
}

void Network::destroyEntity(int32_t networkId)
{
	if (s_server)
	{
		s_server->destroyEntity(networkId);
	}
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
