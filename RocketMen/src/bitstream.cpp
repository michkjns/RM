
#include "BitStream.h"

#include <bitset>
#include <assert.h>

#define BUFFERLENGTH 65536

class BitStream_impl : public BitStream
{
public:
	BitStream_impl();
	~BitStream_impl();

	void writeBits(bool value, size_t repeat /* = 1 */) override;
	void writeBytes(char value, size_t repeat /* = 1 */) override;
	void writeFloat(float value) override;
	void writeInt(int value) override;
	void writeBool(bool value) override;

	void writeBuffer(char* data, size_t length) override;
	void writeData(char* data, size_t length) override;

	void ReadBytes(char* output, size_t size /* = 1 */) override;
	void readBit(bool* output) override;
	float readFloat() override;
	int8_t readInt8() override;
	int32_t readInt32() override;
	int64_t readInt64() override;
	bool readBool() override;


	const size_t getLength() const override;
	const char* getBuffer() const override;

private:
	char m_buffer[BUFFERLENGTH];

	int	m_readTotalBytes;
	int	m_readBit;
	const char* m_readData;

	int	m_writeTotalBytes;
	int	m_writeLength;
	int	m_writeByte;
	int	m_writeBit;
	char* m_writeData;

};

BitStream_impl::BitStream_impl()
	: m_readTotalBytes(0)
	, m_readBit(0)
	, m_readData(m_buffer)
	, m_writeTotalBytes(0)
	, m_writeLength(0)
	, m_writeBit(0)
	, m_writeData(m_buffer)
{
}

BitStream_impl::~BitStream_impl()
{
}

void BitStream_impl::ReadBytes(char* output, size_t numBytes)
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
	ReadBytes(reinterpret_cast<char*>(&out), sizeof(float));
	return out;
}

int8_t BitStream_impl::readInt8()
{
	int out;
	ReadBytes(reinterpret_cast<char*>(&out), sizeof(int8_t));
	return out;
}

int32_t BitStream_impl::readInt32()
{
	int out;
	ReadBytes(reinterpret_cast<char*>(&out), sizeof(int32_t));
	return out;
}

int64_t BitStream_impl::readInt64()
{
	int out;
	ReadBytes(reinterpret_cast<char*>(&out), sizeof(int64_t));
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
	assert(length < BUFFERLENGTH);
	// Buffer overflow
	if (length >= BUFFERLENGTH) return;

	memcpy(m_buffer, data, length);
	m_writeTotalBytes = static_cast<int>(length);
}

void BitStream_impl::writeBytes(char value, size_t repeat)
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
				writeBits(bitValue[bit], 1);
			}
		}
	}
}

void BitStream_impl::writeBits(bool value, size_t repeat /* = 1 */)
{
	//Don't overflow the buffer
	assert(m_writeTotalBytes < BUFFERLENGTH);
	if (m_writeTotalBytes >= BUFFERLENGTH) return /*false*/;

	int32_t numBytes = (int32_t)floorf((float)repeat / 8);
	if (numBytes >= 1)
	{
		writeBytes(value, numBytes);
		repeat -= 8 * numBytes;
	}

	std::bitset<8> bits(*m_writeData);

	for (unsigned int i = 0; i < repeat; i++)
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
	assert(length + m_writeTotalBytes < BUFFERLENGTH);
	if (length + m_writeTotalBytes >= BUFFERLENGTH) return;
	// Buffer overflow

#if 0 //TODO(Michiel) Fix this implementation, it would be faster
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
			writeBytes(data[i], 1);
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

void BitStream_impl::writeInt(int value)
{
	writeData(reinterpret_cast<char*>(&value), sizeof(value));
}

void BitStream_impl::writeBool(bool value)
{
	writeBits(value, 1);
}

BitStream* BitStream::create()
{
	return new BitStream_impl();
}