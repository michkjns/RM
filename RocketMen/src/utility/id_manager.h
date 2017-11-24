
#pragma once

#include <common.h>
#include <core/debug.h>

class IdManager
{
public:
	IdManager(int32_t max) :
		m_max(max),
		m_nextId(0)
	{
		m_size = static_cast<int32_t>(glm::ceil(max / 64.f));
		m_ids = new uint64_t[m_size];

		clear();
	}

	~IdManager()
	{
		delete[] m_ids;
	}

	inline int32_t getMax() const
	{
		return m_max;
	}

	inline bool hasAvailable() const
	{
		for (int32_t i = 0; i < m_size - 1; i++)
		{
			if (m_ids[i] != UINT64_MAX)
				return true;
		}
		return (m_ids[m_size - 1] != UINT64_MAX >> (64 - (m_max % 64)));
	}

	inline bool exists(int32_t id) const
	{
		assert(id >= 0 && id < m_max);
		const int32_t index = id == 0 ? 0 : id / 64;
		const int64_t remainder = static_cast<uint64_t>(id % 64);

		if (index < m_size)
		{
			return (m_ids[index] & (1ULL << remainder)) != 0;
		}

		return false;
	}

	inline void set(int32_t id)
	{
		assert(id >= 0 && id <= m_max);

		const int32_t index = id == 0 ? 0 : id / 64;
		const int32_t remainder = id % 64;
		m_ids[index] |= (1ULL << remainder);
	}

	inline void remove(int32_t id)
	{
		assert(id >= 0 && id < m_max);

		const int32_t index = id == 0 ? 0 : id / 64;
		const int32_t remainder = id % 64;
		m_ids[index] &= ~(1ULL << remainder);
	}

	inline void clear()
	{
		std::fill(m_ids, m_ids + m_size, 0);
		m_nextId = 0;
	}

	inline int32_t getNext()
	{
		if (!hasAvailable())
		{
			return INDEX_NONE;
		}

		const int32_t result = m_nextId;
		set(result);

		if (hasAvailable())
		{
			while (exists(m_nextId))
			{
				m_nextId++;
				if (m_nextId >= m_max)
				{
					m_nextId -= m_max;
				}
			}
		}
		return result;
	}

private:
	int32_t m_max;
	int32_t m_size;

	uint64_t* m_ids;
	int32_t m_nextId;
};
