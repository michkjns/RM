
#include "BitStream.h"

#include <bitset>
#include <assert.h>

BitStream::BitStream() :
	m_readTotalBytes(0),
	m_readBit(0),
	m_readData(m_buffer),
	m_writeTotalBytes(0),
	m_writeLength(0),
	m_writeBit(0),
	m_writeData(m_buffer)
{
}

BitStream::~BitStream()
{
}

void BitStream::readBytes(char* output, size_t numBytes)
{
	if (m_writeTotalBytes <= 0 && m_writeBit <= 0) return;
	if (getLength() - m_readTotalBytes < (int)numBytes) return;

	if (m_readBit == 0)
	{
		for (int i = 0; i < (int)numBytes; i++)
		{
			output[i] = *m_readData;

			m_readData++;
			m_readTotalBytes++;
		}
	}
	else
	{
		for (size_t byte = 0; byte < numBytes; byte++)
		{
			std::bitset<8> bitset;
			for (int i = 0; i < 8; i++)
			{
				bool readValue;
				readBit(&readValue);
				bitset[i] = readValue;
			}

			*output = static_cast<char>( bitset.to_ulong());
			output++;
		}
	}
}

void BitStream::readBit(bool* output)
{
	std::bitset<8> bits(*m_readData);
	*output = bits[m_readBit++];
	if (m_readBit == 8)
	{
		m_readData++;
		m_readBit = 0;
		m_readTotalBytes++;
	}
}

float BitStream::readFloat()
{
	float out;
	readBytes(reinterpret_cast<char*>(&out), sizeof(float));
	return out;
}

int8_t BitStream::readInt8()
{
	int8_t out;
	readBytes(reinterpret_cast<char*>(&out), sizeof(int8_t));
	return out;
}

int16_t BitStream::readInt16()
{
	int16_t out;
	readBytes(reinterpret_cast<char*>(&out), sizeof(int16_t));
	return out;
}

int32_t BitStream::readInt32()
{
	int32_t out;
	readBytes(reinterpret_cast<char*>(&out), sizeof(int32_t));
	return out;
}

int64_t BitStream::readInt64()
{
	int64_t out;
	readBytes(reinterpret_cast<char*>(&out), sizeof(int64_t));
	return out;
}

bool BitStream::readBool()
{
	bool out;
	readBit(&out);
	return out;
}

void BitStream::writeBuffer(char* data, size_t length)
{
	assert(length < s_bufferSize);
	// Buffer overflow
	if (length >= s_bufferSize) return;

	memcpy(m_buffer, data, length);
	m_writeTotalBytes = static_cast<int>(length);
}

void BitStream::writeByte(char value, size_t repeat)
{
	for (int writeByte = 0; writeByte < (int)repeat; writeByte++)
	{
		if (m_writeBit == 0)
		{
			*m_writeData = value;
			m_writeData++;
			m_writeTotalBytes++;

			//Don't overflow the buffer
			if (m_writeTotalBytes == 65536) break;
		}
		else
		{
			std::bitset<8> bitValue(value);
			for (int bit = 0; bit < 8; bit++)
			{
				writeInt8(bitValue[bit]);
			}
		}
	}
}

void BitStream::writeBit(bool value, size_t amount /* = 1 */)
{
	//Don't overflow the buffer
	assert(m_writeTotalBytes < s_bufferSize);
	if (m_writeTotalBytes >= s_bufferSize) return /*false*/;

	int32_t numBytes = (int32_t)floorf((float)amount / 8);
	if (numBytes >= 1)
	{
		writeByte(value, numBytes);
		amount -= 8 * numBytes;
	}

	std::bitset<8> bits(*m_writeData);

	for (unsigned int i = 0; i < amount; i++)
	{
		if (m_writeBit == 8)
		{
			*m_writeData = static_cast<char>(bits.to_ulong());
			m_writeData++;
			bits = std::bitset<8>(*m_writeData);
			m_writeBit = 0;
			m_writeTotalBytes++;
		}

		bits[m_writeBit++] = value;
	}

	*m_writeData = static_cast<char>(bits.to_ulong());
}

void BitStream::writeData(char* data, size_t length)
{
	assert(length + m_writeTotalBytes < s_bufferSize);
	if (length + m_writeTotalBytes >= s_bufferSize) return;

#if 0 //TODO Fix this
	memcpy(m_writeData+1, static_cast<void*>(data), length);
	if (m_writeBit > 0)
	{
		for (int i = 0; i < length; i++)
		{
			m_writeData[i] = m_writeData[i] << m_writeBit;
		}
	}
	m_writeData += length;
	m_writeTotalBytes += length;
#else
	if (m_writeBit == 0)
	{
		memcpy(m_writeData, static_cast<void*>(data), length);
		m_writeData += length;
		m_writeTotalBytes += static_cast<int>(length);
	}
	else
	{
		for (size_t i = 0; i < length; i++)
		{
			writeInt8(data[i]);
		}
	}
#endif
}

const size_t BitStream::getLength() const
{
	size_t length = m_writeTotalBytes;
	if (m_writeBit > 0)
	{
		length++;
	}
	return length;
}

const char* BitStream::getBuffer() const
{
	return m_buffer;
}

void BitStream::writeFloat(float value)
{
	writeData(reinterpret_cast<char*>(&value), sizeof(value));
}

void BitStream::writeInt8(int8_t value)
{
	writeData(reinterpret_cast<char*>(&value), sizeof(value));
}

void BitStream::writeInt16(int16_t value)
{
	writeData(reinterpret_cast<char*>(&value), sizeof(value));
}

void BitStream::writeInt32(int32_t value)
{
	writeData(reinterpret_cast<char*>(&value), sizeof(value));
}

void BitStream::writeInt64(int64_t value)
{
	writeData(reinterpret_cast<char*>(&value), sizeof(value));
}

void BitStream::writeBool(bool value)
{
	writeBit(value, 1);
}

void BitStream::resetReading()
{
	m_readTotalBytes = 0;
	m_readBit = 0;
	m_readData = m_buffer;
}
