
#pragma once

#include <common.h>
#include <core/debug.h>

namespace network
{
	class NetworkIdManager
	{
	public:
		NetworkIdManager(uint32_t max);
		~NetworkIdManager();

		inline int32_t getMax() const
		{
			return m_size * 64;
		}

		inline bool hasAvailable() const 
		{
			for (int32_t i = 0; i < m_size; i++)
			{
				if (m_ids[i] != INT64_MAX)
					return true;
			}
			return false;
		}

		inline bool exists(int32_t networkId) const
		{
			assert(networkId >= 0 && networkId <= getMax());
			const int32_t index = networkId == 0 ? 0 : networkId / 64;
			const int64_t remainder = static_cast<uint64_t>(networkId % 64);

			if (index < m_size)
			{
				return (m_ids[index] & (1ULL << remainder)) != 0;
			}

			return false;
		}

		void set(int32_t networkId);
		void clear(int32_t networkId);
		void reset();
		int32_t next();

	private:
		void increment();
		uint64_t* m_ids;
		int32_t m_nextNetworkId;
		int32_t m_size;
	};
}; // namespace network