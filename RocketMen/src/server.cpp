
#include "server.h"

#include"debug.h"

#include <assert.h>

using namespace network;

class Server_impl : public Server, public Networker
{
public:
	Server_impl();
	~Server_impl();
	
	bool initialize() override;
	void tick(const Time& time) override;

	void host(uint32_t port) override;

	uint32_t getNumClients() const override;

private:
	void onClientConnect(const Packet& packet);
	void onClientDisconnect(const Packet& packet);
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
	Networker::tick(time.getDeltaSeconds());
}

void Server_impl::host(uint32_t port)
{
	m_networkInterface.host(port);
	LOG_INFO("Server: Listening on port %d", port);
}

uint32_t Server_impl::getNumClients() const
{
	return m_numClients;
}

void Server_impl::onClientConnect(const Packet& packet)
{
	m_numClients++;
	LOG_DEBUG("A client has joined with ID %d", packet.header.senderID);

	// Accept client
	Packet responsePacket = createPacket(ECommand::SERVER_HANDSHAKE, nullptr, packet.header.senderID, EBroadcast::BROADCAST_SINGLE);
	responsePacket.data = BitStream::create();
	responsePacket.data->writeInt32(packet.header.senderID);
	queuePacket(responsePacket);
}

void Server_impl::onClientDisconnect(const Packet& packet)
{
	m_numClients--;
	assert(m_numClients >= 0);
}

void Server_impl::handlePacket(const Packet& packet)
{
	switch (packet.header.type)
	{	
		/* Client to server */
		case ECommand::PLAYER_INTRO:
		{
			break;
		}
		case ECommand::PLAYER_INPUT:
		{
			break;
		}

		/* Connection */
		case ECommand::CLIENT_CONNECT:
		{
			onClientConnect(packet);
			break;
		}
		case ECommand::CLIENT_DISCONNECT:
		{
			break;
		}

		default: break;
	}
}
