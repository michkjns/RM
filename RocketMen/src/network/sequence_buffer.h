
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
	void     remove(Sequence sequence);
	int32_t  getSize() const;
	T*       getEntry(Sequence sequence) const;
	Sequence getCurrentSequence() const;
	T*       getAtIndex(int32_t index);
private:
	int32_t  getIndex(uint16_t sequence) const;

	T*        m_buffer;
	bool*     m_exist;
	Sequence* m_sequences;
	Sequence  m_currentSequence;
	int32_t   m_size;
};

inline bool sequenceGreaterThan(Sequence s1, Sequence s2)
{
	return ((s1 > s2) && (s1 - s2 <= 32768)) ||
		((s1 < s2) && (s2 - s1 > 32768));
}

inline bool sequenceLessThan(Sequence s1, Sequence s2)
{
	return ((s1 > s2) && (s1 - s2 <= 32768)) ||
		((s1 < s2) && (s2 - s1 > 32768));
}

inline int32_t sequenceDifference(Sequence s1, Sequence s2)
{
	int32_t s1_32 = s1;
	int32_t s2_32 = s2;
	if (abs(s1_32 - s2_32) >= 32768)
	{
		if (s1_32 > s2_32)
		{
			s2_32 += 65536;
		}
		else
		{
			s1_32 += 65536;
		}
	}
	return s1_32 - s2_32;
}

template<typename T>
SequenceBuffer<T>::SequenceBuffer(int32_t size) :
	m_currentSequence(0),
	m_size(size)
{
	m_buffer    = new T[size];
	m_exist     = new bool[size];
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
	if (sequenceLessThan(sequence, m_currentSequence - m_size))
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
void SequenceBuffer<T>::remove(Sequence sequence)
{
	m_exist[getIndex(sequence)] = false;
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
	if (m_sequences[index] == sequence)
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
inline T* SequenceBuffer<T>::getAtIndex(int32_t index)
{
	assert(index >= 0);
	assert(index < m_size);
	return m_exist[index] ? m_buffer[index] : nullptr;
}

template<typename T>
int32_t SequenceBuffer<T>::getIndex(Sequence sequence) const
{
	return sequence % m_size;
}
