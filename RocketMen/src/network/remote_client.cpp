
#include <network/remote_client.h>

#include <core/debug.h>
#include <network/common_network.h>
#include <network/connection.h>

using namespace network;

RemoteClient::RemoteClient() :
	m_connection(nullptr),
	m_id(INDEX_NONE),
	m_numPlayers(0),
	m_nextNetworkId(0)
{
	std::fill(m_recentNetworkIds,  m_recentNetworkIds  + s_networkIdBufferSize, INDEX_NONE);
}

RemoteClient::~RemoteClient()
{
}

void RemoteClient::initialize(int32_t id, Connection* connection)
{
	assert(isAvailable());
	assert(connection != nullptr);

	m_connection = connection;
	m_id         = id;
	m_numPlayers = 0;
}

void RemoteClient::clear()
{
	m_id = INDEX_NONE;

	assert(m_connection != nullptr);
	delete m_connection;
	m_connection = nullptr;
}

void RemoteClient::setNumPlayers(uint32_t numPlayers)
{
	assert(numPlayers < s_maxPlayersPerClient);
	m_numPlayers = numPlayers;
}

bool RemoteClient::isUsed() const
{
	return (m_id > INDEX_NONE);
}

bool RemoteClient::isAvailable() const
{
	return m_id == INDEX_NONE;
}

int32_t RemoteClient::getId() const
{
	return m_id;
}

uint32_t RemoteClient::getNumPlayers() const
{
	return m_numPlayers;
}

Connection* RemoteClient::getConnection() const 
{
	return m_connection;
}

bool network::operator==(const RemoteClient& a, const RemoteClient& b)
{
	return (a.m_id == b.m_id);
}

bool network::operator!=(const RemoteClient& a, const RemoteClient& b)
{
	return (a.m_id != b.m_id);
}
