
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
	return (s_server && s_server->isInitialized());
}

void Network::connect(const network::Address& address)
{
	assert(s_client != nullptr);
	LOG_INFO("Client: Attempting to connect to %s", address.toString().c_str());
	s_client->connect(address);
}

void Network::disconnect()
{
	if (isClient())
	{
		s_client->disconnect();
	}
}

void Network::generateNetworkID(Entity* entity)
{
	assert(entity != nullptr);
	if (isServer())
	{
		s_server->generateNetworkID(entity);
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

bool Network::addLocalPlayer(int32_t controllerID) // TODO Create enum for controller IDs / input devices
{
	assert(s_client);
	assert(s_client->isInitialized());
	if (ensure(controllerID >= 0))
	{
		return s_client->addLocalPlayer(controllerID);
	}

	return false;
}

uint32_t Network::getNumLocalPlayers()
{
	if (s_client)
	{
		return s_client->getNumLocalPlayers();
	}

	return 0;
}

bool Network::isLocalPlayer(int32_t playerID)
{
	assert(s_client);
	assert(s_client->isInitialized());

	return s_client->isLocalPlayer(playerID);
}

bool Network::requestEntity(Entity* entity)
{
	assert(s_client);
	assert(s_client->isInitialized());

	return s_client->requestEntity(entity);
}

void Network::destroyEntity(int32_t networkID)
{
	if (s_server)
	{
		s_server->destroyEntity(networkID);
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
