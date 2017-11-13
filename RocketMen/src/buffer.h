
#pragma once
#include <common.h>

template <typename T>
class Buffer
{
public:
	Buffer(uint32_t capacity);
	~Buffer() {	delete[] m_buffer; }

	T& insert();
	void remove(T& element);
	uint32_t getCount() const;
	void clear();

	T* begin();
	T* begin() const;
	T* end();
	T* end() const;

	inline T& operator[](size_t i) { return m_buffer[i]; }
	inline const T& operator[](size_t i) const { return m_buffer[i]; }

private:
	T*       m_buffer;
	uint32_t m_capacity;
	uint32_t m_count;
};

template<typename T>
inline Buffer<T>::Buffer(uint32_t capacity) :
	m_capacity(capacity)
{
	m_buffer = (T*) new uint8_t[capacity * (uint32_t)sizeof(T)];
}

template<typename T>
inline T& Buffer<T>::insert()
{
	assert(m_count < m_capacity);
	new (m_buffer + m_count) T();
	return m_buffer[m_count++];
}

template<typename T>
inline void Buffer<T>::remove(T& element)
{
	const uint32_t index = static_cast<uint32_t>(&element - m_buffer);
	assert(index < m_count);
	m_buffer[index].~T();
	std::swap(m_buffer[index], m_buffer[--m_count]);
}

template<typename T>
inline uint32_t Buffer<T>::getCount() const
{
	return m_count;
}

template<typename T>
inline void Buffer<T>::clear()
{
	for (uint32_t i = 0; i < m_count; i++)
	{
		m_buffer[i].~T();
	}
	m_count = 0;
}

template<typename T>
inline T* Buffer<T>::begin()
{
	return m_buffer;
}

template<typename T>
inline T* Buffer<T>::begin() const
{
	return m_buffer;
}

template<typename T>
inline T* Buffer<T>::end()
{
	return m_buffer + m_count;
}

template<typename T>
inline T* Buffer<T>::end() const
{
	return m_buffer + m_count;
}
