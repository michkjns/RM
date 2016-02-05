
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

private:
	void onClientConnect();
	void onClientDisconnect();

	void handlePacket(const Packet& packet) override;
};

Client_impl::Client_impl()
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

void Client_impl::onClientConnect()
{
}

void Client_impl::onClientDisconnect()
{
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
