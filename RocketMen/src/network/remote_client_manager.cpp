
#include <network/remote_client_manager.h>

#include <core/debug.h>
#include <network/connection.h>
#include <network/remote_client.h>

using namespace network;

RemoteClientManager::RemoteClientManager(int32_t size) :
	m_localClientId(INDEX_NONE),
	m_clientIdCounter(0)
{
	assert(size > 0);
	m_size = size;
	m_clients = new RemoteClient[m_size];
}

RemoteClientManager::~RemoteClientManager()
{
	delete[] m_clients;
}

RemoteClient* RemoteClientManager::add(Connection* connection)
{
	assert(connection != nullptr);
	if (getClient(connection))
	{
		return nullptr;
	}

	for (RemoteClient* client = begin(); client != end(); client++)
	{
		if (client->isAvailable())
		{
			client->initialize(m_clientIdCounter++, connection);
			return client;
		}
	}

	return nullptr;
}

void RemoteClientManager::remove(RemoteClient* client)
{
	assert(client != nullptr);

	if (!client->getConnection()->isClosed())
	{
		LOG_INFO("Server: Client %i has timed out", client->getId());
		client->getConnection()->close();
	}

	client->clear();
}

int32_t RemoteClientManager::getMaxClients() const
{
	return m_size;
}

int32_t RemoteClientManager::count() const
{
	int32_t result = 0;
	for (RemoteClient* client = begin(); client != end(); client++)
	{
		if (client->isUsed())
		{
			result++;
		}
	}
	return result;
}

void RemoteClientManager::clear()
{
	for (RemoteClient* client = begin(); client != end(); client++)
	{
		if (client->isUsed())
		{
			client->getConnection()->close();
			client->clear();
		}
	}
}

void RemoteClientManager::sendMessage(struct Message* message, bool skipLocalClient)
{
	for (RemoteClient* client = begin(); client != end(); client++)
	{
		if (client->isUsed())
		{
			if (!(skipLocalClient && client->getId() == m_localClientId))
			{
				client->sendMessage(message);
			}
		}
	}
}

void RemoteClientManager::updateConnections(const Time& time)
{
	for (RemoteClient* client = begin(); client != end(); client++)
	{
		if (client->isUsed() && client->getId() != m_localClientId)
		{
			client->getConnection()->update(time);
		}
	}
}

void RemoteClientManager::sendPendingMessages(const Time & time)
{
	for (RemoteClient* client = begin(); client != end(); client++)
	{
		if (client->isUsed())
		{
			client->getConnection()->sendPendingMessages(time);
		}
	}
}

void RemoteClientManager::setLocalClientId(int32_t id)
{
	m_localClientId = id;
}

int32_t network::RemoteClientManager::getLocalClientId() const
{
	return m_localClientId;
}

RemoteClient* network::RemoteClientManager::getClient(const Address& address) const
{
	for (RemoteClient* client = begin(); client != end(); client++)
	{
		if (client->isUsed() && client->getConnection()->getAddress() == address)
		{
			return client;
		}
	}
	return nullptr;
}

RemoteClient* network::RemoteClientManager::getClient(const Connection* connection) const
{
	for (RemoteClient* client = begin(); client != end(); client++)
	{
		if (client->isUsed() && client->getConnection() == connection)
		{
			return client;
		}
	}
	return nullptr;
}

RemoteClient* RemoteClientManager::begin()
{
	return m_clients;
}

RemoteClient* RemoteClientManager::begin() const
{
	return m_clients;
}

RemoteClient* RemoteClientManager::end()
{
	return m_clients + m_size;
}

RemoteClient* RemoteClientManager::end() const
{
	return m_clients + m_size;
}