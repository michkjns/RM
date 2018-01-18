
#pragma once

#include <common.h>

#include <array>

/* CircularBuffer
*  Simple buffer that allows endless inserting of values, wrapping back to the
*  front when full.
*/
template<class T>
class CircularBuffer
{
public:
	CircularBuffer(int32_t size) : 
		m_buffer(new T[size]),
		m_size(size), 
		m_index(0)
	{
		std::memset(m_buffer, 0, size * sizeof(T));
	};

	~CircularBuffer() { delete[] m_buffer; };

	bool contains(const T& value)
	{
		for (int32_t i = 0; i < m_size; i++)
		{
			if (m_buffer[i] == value)
				return true;
		}
		return false;
	}

	void insert(const T& value)
	{
		m_buffer[m_index] = value;
		m_index = (m_index + 1) % m_size;
	}

	void fill(const T& value)
	{
		std::fill(m_buffer, m_buffer + m_size, value);
	}

	/* Finds the index of given value 
	* @return  int32_t index of value if found, otherwise INDEX_NONE
	*/
	int32_t find(const T& value)
	{
		for (int32_t i = 0; i < m_size; i++)
		{
			if (m_buffer[i] == value)
				return i;
		}
		return INDEX_NONE;
	}

	inline T& operator[](size_t i) { return m_buffer[i]; }
	inline const T& operator[](size_t i) const { return m_buffer[i]; }

	T* begin() { return m_buffer; }
	T* begin() const { return m_buffer; }

	T* end() { return m_buffer + m_size; }
	T* end() const { return m_buffer + m_size; }
private:
	T*       m_buffer;
	int32_t  m_size;
	int32_t  m_index;
};
