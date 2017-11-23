
#include <network/network_id_manager.h>

using namespace network;

NetworkIdManager::NetworkIdManager(uint32_t max)
{
	assert(max % 64 == 0);
	m_nextNetworkId = 0;
	m_size = max / 64;
	m_ids = new uint64_t[m_size];
	reset();
}

NetworkIdManager::~NetworkIdManager()
{
	delete[] m_ids;
}

void NetworkIdManager::set(int32_t networkId)
{
	assert(networkId >= 0 && networkId <= getMax());

	const int32_t index = networkId == 0 ? 0 : networkId / 64;
	const int32_t remainder = networkId % 64;
	m_ids[index] |= (1ULL << remainder);
}

void NetworkIdManager::remove(int32_t networkId)
{
	assert(networkId >= 0 && networkId <= getMax());

	const int32_t index = networkId == 0 ? 0 : networkId / 64;
	const int32_t remainder = networkId % 64;
	m_ids[index] &= ~(1ULL << remainder);
}

void NetworkIdManager::reset()
{
	std::fill(m_ids, m_ids + m_size, 0);
	m_nextNetworkId = 0;
}

int32_t NetworkIdManager::next()
{
	assert(hasAvailable());
	const int32_t result = m_nextNetworkId;

	increment();
	while (exists(m_nextNetworkId))
	{		
		increment();
	}
	return result;
}

void NetworkIdManager::increment()
{
	m_nextNetworkId++;
	const int32_t max = getMax();
	if (m_nextNetworkId >= max)
	{
		m_nextNetworkId -= max;
	}
}