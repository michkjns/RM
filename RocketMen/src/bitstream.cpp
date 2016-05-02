
#include "BitStream.h"

#include <bitset>
#include <assert.h>

static const uint32_t s_bufferSize = 65536;

class BitStream_impl : public BitStream
{
public:
	BitStream_impl();
	~BitStream_impl();

	void    writeBit(bool value, size_t repeat /* = 1 */)  override;
	void    writeByte(char value, size_t repeat = 1)       override;
	void    writeFloat(float value)                        override;
	void    writeInt8(int8_t value)                        override;
	void    writeInt16(int16_t value)                      override;
	void    writeInt32(int32_t value)                      override;
	void    writeInt64(int64_t value)                      override;
	void    writeBool(bool value)                          override;

	void    writeBuffer(char* data, size_t length)         override;
	void    writeData(char* data, size_t length)           override;

	void    readBytes(char* output, size_t size /* = 1 */) override;
	void    readBit(bool* output)                          override;
	float   readFloat()                                    override;
	int8_t  readInt8()                                     override;
	int16_t readInt16()                                    override;
	int32_t readInt32()                                    override;
	int64_t readInt64()                                    override;
	bool    readBool()                                     override;

	const size_t getLength() const override;
	const char*  getBuffer() const override;

	void resetReading() override;

private:
	char        m_buffer[s_bufferSize];
	int         m_readTotalBytes;
	int         m_readBit;
	const char* m_readData;

	int	  m_writeTotalBytes;
	int	  m_writeLength;
	int	  m_writeByte;
	int	  m_writeBit;
	char* m_writeData;
};

BitStream_impl::BitStream_impl() :
	m_readTotalBytes(0),
	m_readBit(0),
	m_readData(m_buffer),
	m_writeTotalBytes(0),
	m_writeLength(0),
	m_writeBit(0),
	m_writeData(m_buffer)
{
}

BitStream_impl::~BitStream_impl()
{
}

void BitStream_impl::readBytes(char* output, size_t numBytes)
{
	if (m_writeTotalBytes <= 0 && m_writeBit <= 0) return /*false*/;
	if (getLength() - m_readTotalBytes < (int)numBytes) return /*false*/;

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

void BitStream_impl::readBit(bool* output)
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

float BitStream_impl::readFloat()
{
	float out;
	readBytes(reinterpret_cast<char*>(&out), sizeof(float));
	return out;
}

int8_t BitStream_impl::readInt8()
{
	int8_t out;
	readBytes(reinterpret_cast<char*>(&out), sizeof(int8_t));
	return out;
}

int16_t BitStream_impl::readInt16()
{
	int16_t out;
	readBytes(reinterpret_cast<char*>(&out), sizeof(int16_t));
	return out;
}

int32_t BitStream_impl::readInt32()
{
	int32_t out;
	readBytes(reinterpret_cast<char*>(&out), sizeof(int32_t));
	return out;
}

int64_t BitStream_impl::readInt64()
{
	int64_t out;
	readBytes(reinterpret_cast<char*>(&out), sizeof(int64_t));
	return out;
}

bool BitStream_impl::readBool()
{
	bool out;
	readBit(&out);
	return out;
}

void BitStream_impl::writeBuffer(char* data, size_t length)
{
	assert(length < s_bufferSize);
	// Buffer overflow
	if (length >= s_bufferSize) return;

	memcpy(m_buffer, data, length);
	m_writeTotalBytes = static_cast<int>(length);
}

void BitStream_impl::writeByte(char value, size_t repeat)
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

void BitStream_impl::writeBit(bool value, size_t amount /* = 1 */)
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

void BitStream_impl::writeData(char* data, size_t length)
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

const size_t BitStream_impl::getLength() const
{
	size_t length = m_writeTotalBytes;
	if (m_writeBit > 0)
	{
		length++;
	}
	return length;
}

const char* BitStream_impl::getBuffer() const
{
	return m_buffer;
}

void BitStream_impl::writeFloat(float value)
{
	writeData(reinterpret_cast<char*>(&value), sizeof(value));
}

void BitStream_impl::writeInt8(int8_t value)
{
	writeData(reinterpret_cast<char*>(&value), sizeof(value));
}

void BitStream_impl::writeInt16(int16_t value)
{
	writeData(reinterpret_cast<char*>(&value), sizeof(value));
}

void BitStream_impl::writeInt32(int32_t value)
{
	writeData(reinterpret_cast<char*>(&value), sizeof(value));
}

void BitStream_impl::writeInt64(int64_t value)
{
	writeData(reinterpret_cast<char*>(&value), sizeof(value));
}

void BitStream_impl::writeBool(bool value)
{
	writeBit(value, 1);
}

void BitStream_impl::resetReading()
{
	m_readTotalBytes = 0;
	m_readBit = 0;
	m_readData = m_buffer;
}

BitStream* BitStream::create()
{
	return new BitStream_impl();
}