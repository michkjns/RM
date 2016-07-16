
#include <bitstream.h>

#include <utility.h>

#include <bitset>
#include <assert.h>

/** Write scratch to buffer */
void WriteStream::flush(bool increment)
{
	m_buffer[m_wordIndex] = static_cast<uint32_t>(m_scratch);
	
	if (increment) // Move along the buffer
	{
		m_wordIndex++;
		m_scratchBits -= 32;
		m_scratch >>= 32;

		m_buffer[m_wordIndex] = static_cast<uint32_t>(m_scratch);
		
		if (m_wordIndex == m_bufferLength)
			m_isFull = true;
	}

}

/** Write up to 32 bits */
void WriteStream::serializeBits(uint32_t value, uint32_t numBits)
{
	assert(numBits > 0);
	assert(numBits <= 32);
	
	value &= (uint64_t(1) << numBits) - 1; // Masks out all bits above numBits
	m_scratch |= uint64_t(value) << m_scratchBits; // Writes the bits to scratch

	m_scratchBits += numBits;
	
	flush(m_scratchBits >= 32);

	OnDebug(
		m_bitsWritten += numBits;
	);
}

/* Write a bool as 0 or 1 bit */
void WriteStream::serializeBool(bool& value)
{
	if (value)
		m_scratch |= (uint64_t(1) << m_scratchBits);

	m_scratchBits++;
	
	flush(m_scratchBits >= 32);

	OnDebug(
		m_bitsWritten += 1;
	);
}

/* Write and compress a 32-bit integer in in range [min, max] */
void WriteStream::serializeInt(int32_t& value, int32_t min, int32_t max)
{
	assert(min < max);	
	assert(value >= min); 
	assert(value <= max);

	const int32_t bits = bitsRequired(min, max);
	uint32_t uvalue = value - min;
	serializeBits(uvalue, bits);
}

/* Write a full byte */
void WriteStream::serializeByte(const uint8_t byte)
{
	serializeBits(uint32_t(byte), 8);
}

void WriteStream::serializeData(const uint8_t* data, int32_t dataLength)
{
	assert(data != nullptr);
	assert(dataLength > 0);

	if (m_bufferLength - m_wordIndex <= dataLength)
		return; // Stream buffer too short

	if ((m_scratchBits & 7) == 0) // Byte aligned
	{
		const int32_t bytesToWrite = std::min((32 - m_scratchBits) / 8, dataLength);
		const int32_t bytesToCopy = dataLength - bytesToWrite;

		// Fill scratch
		for (int32_t i = 0; i < bytesToWrite; i++)
			serializeBits(uint32_t(data[i]), 8);
		
		flush(true);

		// Copy remaining
		memcpy(m_buffer + m_wordIndex, data + bytesToWrite, dataLength - bytesToWrite);
		m_wordIndex += bytesToCopy;
OnDebug(
		m_bitsWritten += (dataLength-bytesToWrite) * 8;
);
	}
	else
	{
		for (int32_t i = 0; i < dataLength; i++)
		{
			serializeBits(data[i], 8);
		}
	}
}

/** Load next word to scratch */
void ReadStream::flush(bool)
{
	// Load into lower half of scratch
	if (m_scratchBits == 0)
	{
		assert(m_wordIndex < m_bufferLength);

		m_scratch |= m_buffer[m_wordIndex++];
		m_scratchBits += 32;
	}
	// Load into upper half of scratch
	if (m_scratchBits <= 32)
	{
		assert(m_wordIndex < m_bufferLength);
		m_scratch |= uint64_t(m_buffer[m_wordIndex++]) << m_scratchBits;
		m_scratchBits += 32;
	}
}

/** Read up to 32 bits */
void ReadStream::serializeBits(uint32_t& value, uint32_t numBits)
{
	assert(numBits > 0);
	assert(numBits <= 32);
	flush();

	value = m_scratch & (uint64_t(1) << numBits) - 1;
	m_scratch >>= numBits;
	m_scratchBits-= numBits;

	OnDebug(
		m_bitsRead += numBits;
	);
}

void ReadStream::serializeBool(bool& dest)
{
	flush();

	dest = (m_scratch & 1);
	m_scratch >>= 1;
	m_scratchBits--;

	OnDebug(
		m_bitsRead += 1;
	);
}

void ReadStream::serializeInt(int32_t& value, int32_t min, int32_t max)
{
	assert(min < max);

	flush();

	const uint32_t bits = bitsRequired(min, max);
	uint32_t value32 = 0;
	serializeBits(value32, bits);
	value = value32 + min;
}

void ReadStream::serializeByte(uint8_t& dest)
{
	flush();

	uint32_t value32 = static_cast<uint32_t>(dest);
	serializeBits(value32, 8);
	dest = static_cast<uint8_t>(value32);
}

void ReadStream::serializeData(uint8_t* dest, int32_t length)
{
	flush();
	if ((m_scratchBits & 7) == 0) // Byte aligned
	{
		if (m_scratchBits == 0)
		{
			memcpy(dest, m_buffer + m_wordIndex, length);
		}
		else
		{
			for (int32_t i = 0; i < length; i++)
			{
				serializeByte(dest[i]);
			}
		}
	}
};










BitStream::BitStream() :
	m_readTotalBytes(0),
	m_readBit(0),
	m_writeTotalBytes(0),
	m_writeLength(0),
	m_writeBit(0)
{
	m_buffer = std::unique_ptr<char[]>(new char[s_bufferSize]);
	m_writeData = m_buffer.get();
	m_readData  = m_buffer.get();
}

BitStream::BitStream(const BitStream& bs)
{
	m_buffer = std::unique_ptr<char[]>(new char[s_bufferSize]);
	memcpy(m_buffer.get(), bs.m_buffer.get(), s_bufferSize);

	m_readData        = m_buffer.get();
	m_readTotalBytes  = bs.m_readTotalBytes;
	m_readBit         = bs.m_readBit;

	m_writeTotalBytes = bs.m_writeTotalBytes;
	m_writeLength     = bs.m_writeLength;
	m_writeByte       = bs.m_writeByte;
	m_writeBit        = bs.m_writeBit;
	m_writeData       = m_buffer.get();
}

BitStream::~BitStream()
{
	//delete[] m_buffer;
}

BitStream& BitStream::operator=(const BitStream& other)
{
	if (&other == this)
		return *this;

	memcpy(m_buffer.get(), other.m_buffer.get(), s_bufferSize);

	m_readData        = m_buffer.get();
	m_readTotalBytes  = other.m_readTotalBytes;
	m_readBit         = other.m_readBit;

	m_writeTotalBytes = other.m_writeTotalBytes;
	m_writeLength     = other.m_writeLength;
	m_writeByte       = other.m_writeByte;
	m_writeBit        = other.m_writeBit;
	m_writeData       = m_buffer.get();

	return *this;

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

void BitStream::writeBuffer(const char* data, size_t length)
{
	assert(length < s_bufferSize);
	// Buffer overflow
	if (length >= s_bufferSize) return;

	memcpy(m_buffer.get(), data, length);
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

void BitStream::writeData(const char* data, size_t length)
{
	assert(length + m_writeTotalBytes < s_bufferSize);
	if (length + m_writeTotalBytes >= s_bufferSize) return;

#if 0 //TODO Fix this
	memcpy(m_writeData+1, data, length);
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
		memcpy(m_writeData, data, length);
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
	return m_buffer.get();
}

int32_t BitStream::getReadTotalBytes() const
{
	return m_readTotalBytes;
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
	m_readBit        = 0;
	m_readData       = m_buffer.get();
}

void BitStream::resetWriting()
{
	m_writeTotalBytes = 0;
	m_writeLength     = 0;
	m_writeByte       = 0;
	m_writeBit        = 0;
	m_writeData       = m_buffer.get();
}

void BitStream::reset()
{
	resetReading();
	resetWriting();
}
