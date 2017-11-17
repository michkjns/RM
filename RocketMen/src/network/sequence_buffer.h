
#pragma once
#include <common.h>

// http://io7m.com/documents/udp-reliable/#sequence-numbers
// https://gafferongames.com/post/reliable_ordered_messages/

template<typename T>
class SequenceBuffer
{
public:
	SequenceBuffer(int32_t size);
	~SequenceBuffer();

	T*       insert(Sequence sequence);
	bool     exists(Sequence sequence) const;
	bool     isAvailable(Sequence sequence) const;
	void     remove(Sequence sequence);
	void     removeOldEntries();
	bool     isEmpty() const;
	int32_t  getSize() const;
	T*       getEntry(Sequence sequence) const;
	Sequence getCurrentSequence() const;
	int32_t  getIndex(Sequence sequence) const;
	T*       getAtIndex(int32_t index) const;

private:
	T*        m_buffer;
	bool*     m_exist;
	Sequence* m_sequences;
	Sequence  m_currentSequence;
	int32_t   m_size;
	bool      m_firstEntry;
};

template<typename T>
SequenceBuffer<T>::SequenceBuffer(int32_t size) :
	m_currentSequence(0),
	m_size(size),
	m_firstEntry(true)
{
	m_buffer    = new T[size];
	m_exist     = new bool[size]();
	m_sequences = new Sequence[size];
}

template<typename T>
SequenceBuffer<T>::~SequenceBuffer()
{
	delete[] m_buffer;
	delete[] m_exist;
	delete[] m_sequences;
}

template<typename T>
T* SequenceBuffer<T>::insert(Sequence sequence)
{
	if (m_firstEntry)
	{
		m_currentSequence = sequence + 1;
		m_firstEntry = false;
	}
	else if (sequenceLessThan(sequence, m_currentSequence - static_cast<Sequence>(m_size)))
	{
		return nullptr;
	}
	else if (sequenceGreaterThan(sequence + 1, m_currentSequence))
	{
		m_currentSequence = sequence + 1;
	}

	const int32_t index = sequence % m_size;
	m_exist[index] = true;
	m_sequences[index] = sequence;

	return &m_buffer[index];
}

template<typename T>
bool SequenceBuffer<T>::exists(Sequence sequence) const
{
	return m_exist[getIndex(sequence)];
}

template<typename T>
inline bool SequenceBuffer<T>::isAvailable(Sequence sequence) const
{
	return !exists(sequence);
}

template<typename T>
bool SequenceBuffer<T>::isEmpty() const
{
	for (int32_t i = 0;  i < m_size; i++)
	{
		if (m_exist[i])
		{
			return false;
		}
	}
	return true;
}


template<typename T>
void SequenceBuffer<T>::remove(Sequence sequence)
{
	m_exist[getIndex(sequence)] = false;
}

template<typename T>
inline void SequenceBuffer<T>::removeOldEntries()
{
	const Sequence oldestSequence = m_currentSequence - static_cast<Sequence>(m_size);
	for (int32_t i = 0; i < m_size; i++)
	{
		if (m_exist[i] && sequenceLessThan(m_sequences[i], oldestSequence))
		{
			m_exist[i] = false;
		}
	}
}

template<typename T>
int32_t SequenceBuffer<T>::getSize() const
{
	return m_size;
}

template<typename T>
T* SequenceBuffer<T>::getEntry(Sequence sequence) const
{
	const int32_t index = getIndex(sequence);
	if (exists(sequence) && m_sequences[index] == sequence)
	{
		return &m_buffer[index];
	}

	return nullptr;
}

template<typename T>
inline Sequence SequenceBuffer<T>::getCurrentSequence() const
{
	return m_currentSequence;
}

template<typename T>
inline T* SequenceBuffer<T>::getAtIndex(int32_t index) const
{
	assert(index >= 0);
	assert(index < m_size);
	return m_exist[index] ? &m_buffer[index] : nullptr;
}

template<typename T>
int32_t SequenceBuffer<T>::getIndex(Sequence sequence) const
{
	return sequence % m_size;
}
