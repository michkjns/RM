
#include "server.h"

#include <assert.h>

using namespace network;

class Server_impl : public Server, public Networker
{
public:
	Server_impl();
	~Server_impl();
	
	bool initialize() override;
	void tick(const Time& time) override;

	uint32_t getNumClients() const override;

private:
	void onClientConnect();
	void onClientDisconnect();
	void handlePacket(const Packet& packet) override;

	uint32_t m_numClients;
};

Server_impl::Server_impl()
	: m_numClients(0)
{
}

Server_impl::~Server_impl()
{
}

Server* Server::create()
{
	return new Server_impl();
}

bool Server_impl::initialize()
{
	if (!Networker::initialize())
	{
		return false;
	}

	return true;
}

void Server_impl::tick(const Time& time)
{
	Networker::tick();
}

uint32_t Server_impl::getNumClients() const
{
	return m_numClients;
}

void Server_impl::onClientConnect()
{
	m_numClients++;
}

void Server_impl::onClientDisconnect()
{
	m_numClients--;
	assert(m_numClients >= 0);
}

void Server_impl::handlePacket(const Packet& packet)
{
	switch (packet.header.type)
	{	
		/* Client to server */
		case EPacketType::PLAYER_INTRO:
		{
			break;
		}
		case EPacketType::PLAYER_INPUT:
		{
			break;
		}

		/* Connection */
		case EPacketType::CONNECTION_CONNECT:
		{
			break;
		}
		case EPacketType::CONNECTION_DISCONNECT:
		{
			break;
		}

		default: break;
	}
}
