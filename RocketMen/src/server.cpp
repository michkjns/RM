
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

	unsigned int getNumClients() const override;


private:
	void OnClientConnect();
	void OnClientDisconnect();

	unsigned int m_numClients;
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

unsigned int Server_impl::getNumClients() const
{
	return m_numClients;
}

void Server_impl::OnClientConnect()
{
	m_numClients++;
}

void Server_impl::OnClientDisconnect()
{
	m_numClients--;
	assert(m_numClients >= 0);
}
