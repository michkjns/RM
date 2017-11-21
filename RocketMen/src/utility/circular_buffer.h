
#pragma once

#include <common.h>

#include <array>

/* CircularBuffer
*  Simple buffer that allows endless inserting of values, wrapping back to the
*  front when full.
*/
template<class T, size_t N>
class CircularBuffer
{
public:
	CircularBuffer() { m_index = 0; };
	~CircularBuffer() {};

	bool contains(const T& value)
	{
		for (const auto& i : m_buffer)
		{
			if (i == value)
				return true;
		}
		return false;
	}

	void insert(const T& value)
	{
		m_buffer[m_index] = value;
		m_index = (m_index + 1) % (m_buffer.size());
	}

	void fill(const T& value)
	{
		m_buffer.fill(value);
	}

	/* Finds the index of given value 
	* @return  int32_t index of value if found, otherwise INDEX_NONE
	*/
	int32_t find(const T& value)
	{
		for (int32_t i = 0; i < m_buffer.size(); i++)
		{
			if (m_buffer[i] == value)
				return i;
		}
		return INDEX_NONE;
	}

	inline T& operator[](size_t i) { return m_buffer[i]; }
	inline const T& operator[](size_t i) const { return m_buffer[i]; }

	auto begin() { return m_buffer.begin(); }
	auto end()   { return m_buffer.end(); }
private:
	std::array<T, N> m_buffer;
	int32_t m_index;
};
