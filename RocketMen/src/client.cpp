
#include "client.h"

#include <assert.h>

using namespace network;

class Client_impl : public Client, public Networker
{
public:
	Client_impl();
	~Client_impl();


	bool initialize() override;
	void tick() override;

	unsigned int getNumClients() const override;

private:
	void onClientConnect();
	void onClientDisconnect();

	void handlePacket(const Packet& packet) override;

	unsigned int m_numClients;
};

Client_impl::Client_impl()
	: m_numClients(0)
{
}

Client_impl::~Client_impl()
{
}


bool Client_impl::initialize()
{
	if (!Networker::initialize())
	{
		return false;
	}

	return true;
}

void Client_impl::tick()
{
	Networker::tick();
}

unsigned int Client_impl::getNumClients() const
{
	return m_numClients;
}

void Client_impl::onClientConnect()
{
	m_numClients++;
}

void Client_impl::onClientDisconnect()
{
	m_numClients--;
	assert(m_numClients >= 0);
}

Client* Client::create()
{
	return new Client_impl();
}

void Client_impl::handlePacket(const Packet& packet)
{
	switch (packet.header.type)
	{
		switch (packet.header.type)
		{
			/** Server to client */
			case EPacketType::SERVER_GAMESTATE:
			{
				break;
			}
			case EPacketType::SERVER_HANDSHAKE:
			{
				break;
			}

			/** Connection */
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
}
