
#include <network/remote_client.h>

#include <core/debug.h>
#include <network/common_network.h>
#include <network/connection.h>

using namespace network;

RemoteClient::RemoteClient() :
	m_connection(nullptr),
	m_id(INDEX_NONE),
	m_nextNetworkId(0),
	m_playerIds(s_maxPlayersPerClient)
{
	std::fill(m_recentNetworkIds,  m_recentNetworkIds  + s_networkIdBufferSize, INDEX_NONE);
}

RemoteClient::~RemoteClient()
{
}

void RemoteClient::initialize(int32_t id, Connection* connection)
{
	ASSERT(isAvailable());
	ASSERT(connection != nullptr);

	m_connection = connection;
	m_id         = id;
}

void RemoteClient::clear()
{
	m_id = INDEX_NONE;
	m_playerIds.clear();

	delete m_connection;
	m_connection = nullptr;
}

void RemoteClient::addPlayer(int16_t playerId)
{
	ASSERT(playerId >= 0);
	m_playerIds.insert(playerId);
}

void RemoteClient::sendMessage(Message* message)
{
	ASSERT(m_connection != nullptr);
	m_connection->sendMessage(message);
}

bool RemoteClient::isUsed() const
{
	return (m_id > INDEX_NONE);
}

bool RemoteClient::isAvailable() const
{
	return m_id == INDEX_NONE;
}

bool RemoteClient::ownsPlayer(int16_t playerId) const
{
	for (int16_t id : m_playerIds)
	{
		if (id == playerId)
		{
			return true;
		}
	}
	return false;
}

int32_t RemoteClient::getId() const
{
	return m_id;
}

uint32_t RemoteClient::getNumPlayers() const
{
	return m_playerIds.getCount();
}

Connection* RemoteClient::getConnection() const 
{
	return m_connection;
}

Buffer<int16_t>& RemoteClient::getPlayerIds()
{
	return m_playerIds;
}

bool network::operator==(const RemoteClient& a, const RemoteClient& b)
{
	return (a.m_id == b.m_id);
}

bool network::operator!=(const RemoteClient& a, const RemoteClient& b)
{
	return (a.m_id != b.m_id);
}
