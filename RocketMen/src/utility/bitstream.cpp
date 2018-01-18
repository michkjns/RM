
#include <utility/bitstream.h>

#include <common.h>
#include <utility/utility.h>

#include <bitset>
#include <assert.h>

extern "C" unsigned long crcFast(unsigned char const message[], int nBytes);

BitReader::BitReader(const char* data, int32_t numBytes) :
	m_scratch(0),
	m_data((uint32_t*)(data)),
	m_scratchBits(0),
	m_numWords((numBytes + 3) / 4),
	m_wordIndex(0),
	m_size(numBytes),
	m_numBitsRead(0),
	m_numBits(numBytes * 8)
{
	assert(numBytes > 0);
	assert(data != nullptr);
}

BitReader::~BitReader()
{
}

int32_t BitReader::readBits(int32_t numBits)
{
	assert(numBits > 0);
	assert(numBits <= 32);
	assert(m_numBitsRead + numBits <= m_numBits);
	assert(m_scratchBits >= 0 && m_scratchBits <= 64);

	if (m_scratchBits < numBits)
	{
		// Load next 32 bits to scratch
		assert(m_wordIndex < m_numWords);
		m_scratch |= uint64_t(m_data[m_wordIndex]) << m_scratchBits;
		m_scratchBits += 32;
		m_wordIndex++;
	}

	const int32_t value = m_scratch & ((uint64_t(1) << numBits) - 1);
	m_scratch >>= numBits;
	m_scratchBits -= numBits;
	m_numBitsRead += numBits;

	return value;
}

void BitReader::readBytes(char* dest, int32_t numBytes)
{
	assert(dest != nullptr);
	assert(numBytes > 0);
	assert(m_numBitsRead + numBytes * 8 <= m_numBits);
	align();
	assert((m_numBitsRead % 8) == 0);

	const int32_t numHeadBytes = std::max((4 - (m_numBitsRead % 32) / 8) % 4, numBytes);
	for (int32_t i = 0; i < numHeadBytes; i++)
	{
		dest[i] = (char)readBits(8);
	}
	if (numHeadBytes == numBytes)
	{
		return;
	}

	const int32_t numWords = (numBytes - numHeadBytes) / 4;
	if (numWords > 0)
	{
		assert((m_numBitsRead % 32) == 0);
		memcpy(dest + numHeadBytes, &m_data[m_wordIndex], numWords * 4);
		m_numBitsRead += numWords * 32;
		m_wordIndex += numWords;
		m_scratchBits = 0;
	}

	const int32_t tailStart = numHeadBytes + numWords * 4;
	const int32_t numTailBytes = numBytes - tailStart;

	assert(numHeadBytes + numWords * 4 + numTailBytes == numBytes);
	assert(numTailBytes >= 0 && numTailBytes < 4);
	for (int32_t i = 0; i < numTailBytes; ++i)
	{
		dest[tailStart + i] = (char)readBits(8);
	}
}

void BitReader::align()
{
	const int32_t remainderBits = m_numBitsRead & 7;
	if (remainderBits != 0)
	{
		uint32_t padding = (char)readBits(8 - remainderBits);
		assert(m_numBitsRead % 8 == 0);
		assert(padding == 0);
	}
}

ReadStream::ReadStream(const char* buffer, int32_t numBytes) :
	m_buffer(new char[numBytes]),
	m_reader(m_buffer, numBytes)
{
	assert(buffer != nullptr);
	assert(numBytes > 0);
	assert((numBytes % 4) == 0);

	memcpy(m_buffer, buffer, numBytes);
}

ReadStream::~ReadStream()
{
	delete[] m_buffer;
}

/** Read up to 32 bits */
bool ReadStream::serializeBits(uint32_t& value, int32_t numBits)
{
	assert(numBits > 0);
	assert(numBits <= 32);
	const uint32_t readValue = m_reader.readBits(numBits);
	value = readValue;

	return true;
}

bool ReadStream::serializeBool(bool& dest)
{
	serializeBits(reinterpret_cast<uint32_t&>(dest), 1);

	return true;
}

bool ReadStream::serializeInt(int32_t& value, int32_t min, int32_t max)
{
	assert(min < max);

	const uint32_t bits = bitsRequired(min, max);
	uint32_t value32 = 0;
	serializeBits(value32, bits);
	value = value32 + min;

	return true;
}

bool ReadStream::serializeInt(uint32_t& value, uint32_t min, uint32_t max)
{
	assert(min < max);

	const uint32_t bits = bitsRequired(min, max);
	uint32_t value32 = 0;
	serializeBits(value32, bits);
	value = value32 + min;

	return true;
}

bool ReadStream::serializeByte(char& dest)
{
	uint32_t value32 = static_cast<uint32_t>(dest);
	serializeBits(value32, 8);
	dest = static_cast<uint8_t>(value32);

	return true;
}

bool ReadStream::serializeData(char* dest, int32_t length)
{
	m_reader.readBytes(dest, length);

	return true;
}

bool ReadStream::serializeCheck(const char* string)
{
#if RM_SERIALIZE_CHECK
	uint32_t value = 0;
	serializeBits(value, 32);

	const uint32_t hash = crcFast(reinterpret_cast<const unsigned char*>(string),
		static_cast<int32_t>(strlen(string)));
	return value == hash;
#else
	return true;
#endif
}

BitWriter::BitWriter(char* buffer, int32_t numBytes) :
	m_scratch(0),
	m_data(reinterpret_cast<uint32_t*>(buffer)),
	m_scratchBits(0),
	m_numWords((numBytes + 3) / 4),
	m_wordIndex(0),
	m_size(numBytes),
	m_numBitsWritten(0),
	m_numBits(static_cast<int32_t>(numBytes) * 8)
{
	assert(numBytes > 0);
	assert((numBytes % 4) == 0);
}

BitWriter::~BitWriter()
{
}

void BitWriter::writeBits(uint32_t value, int32_t numBits)
{
	assert(numBits > 0);
	assert(numBits <= 32);
	assert(m_numBitsWritten + numBits < m_numBits);

	value &= (uint64_t(1) << numBits) - 1; // Masks out all bits above numBits
	m_scratch |= uint64_t(value) << m_scratchBits; // Writes the bits to scratch
	m_scratchBits += numBits;

	if (m_scratchBits >= 32)
	{
		// Write 32 bits from scratch to buffer
		assert(m_wordIndex < m_numWords);
		m_data[m_wordIndex] = m_scratch & 0xFFFFFFFF;
		m_scratch >>= 32;
		m_scratchBits -= 32;
		m_wordIndex++;
	}

	m_numBitsWritten += numBits;
}

void BitWriter::writeBytes(const char* data, int32_t numBytes)
{
	assert(data != nullptr);
	assert(numBytes > 0);
	assert(m_numBitsWritten + numBytes * 8 <= m_numBits);

	alignToByte();
	assert((m_numBitsWritten % 8) == 0);

	const int32_t numHeadBytes = std::max((4 - (m_numBitsWritten % 32) / 8) % 4, numBytes);
	for (int32_t i = 0; i < numHeadBytes; ++i)
	{
		writeBits(data[i], 8);
	}
	if (numHeadBytes == numBytes)
	{
		return;
	}

	flush();

	const int32_t numWords = (numBytes - numHeadBytes) / 4;
	if (numWords > 0)
	{
		assert((m_numBitsWritten % 32) == 0);
		memcpy(&m_data[m_wordIndex], data + numHeadBytes, numWords * 4);
		m_numBitsWritten += numWords * 32;
		m_wordIndex += numWords;
		m_scratch = 0;
	}

	const int32_t tailStart = numHeadBytes + numWords * 4;
	const int32_t numTailBytes = numBytes - tailStart;

	assert(numHeadBytes + numWords * 4 + numTailBytes == numBytes);
	assert(numTailBytes >= 0 && numTailBytes < 4);
	for (int32_t i = 0; i < numTailBytes; ++i)
	{
		writeBits(m_data[tailStart + i], 8);
	}
}

void BitWriter::flush()
{
	if (m_scratchBits != 0)
	{
		assert(m_wordIndex < m_numWords);
		m_data[m_wordIndex] = m_scratch & 0xFFFFFFFF;
		m_scratch >>= 32;
		m_scratchBits -= 32;
		m_wordIndex++;
	}
	assert(m_scratchBits <= 0);
}

void BitWriter::alignToByte()
{
	const int32_t remainderBits = m_numBitsWritten % 8;
	if (remainderBits != 0)
	{
		uint32_t zeroPadding = 0;
		writeBits(zeroPadding, 8 - remainderBits);
		assert(m_numBitsWritten % 8 == 0);
	}
}


WriteStream::WriteStream(int32_t numBytes) :
	m_buffer(new char[numBytes]),
	m_writer(m_buffer, numBytes),
	m_size(numBytes)
{
	assert(numBytes > 0);
	assert((numBytes % 4) == 0);
}

WriteStream::~WriteStream()
{
	delete[] m_buffer;
}

/** Write up to 32 bits */
bool WriteStream::serializeBits(uint32_t value, int32_t numBits)
{
	m_writer.writeBits(value, numBits);

	return true;
}

/* Write a bool as 0 or 1 bit */
bool WriteStream::serializeBool(bool& value)
{
	serializeBits(reinterpret_cast<uint32_t&>(value), 1);

	return true;
}

/* Write and compress a 32-bit integer in in range [min, max] */
bool WriteStream::serializeInt(int32_t& value, int32_t min, int32_t max)
{
	assert(min < max);
	assert(value >= min);
	assert(value <= max);

	const int32_t bits = bitsRequired(min, max);
	uint32_t uvalue = value - min;
	serializeBits(uvalue, bits);

	return true;
}

bool WriteStream::serializeInt(uint32_t& value, uint32_t min, uint32_t max)
{
	assert(min < max);
	assert(value >= min);
	assert(value <= max);

	const int32_t bits = bitsRequired(min, max);
	uint32_t uvalue = value - min;
	serializeBits(uvalue, bits);

	return true;
}

/* Write a full byte */
bool WriteStream::serializeByte(const char byte)
{
	serializeBits(uint32_t(byte), 8);

	return true;
}

bool WriteStream::serializeData(const char* data, int32_t length)
{
	assert(data != nullptr);
	assert(length >= 1);
	m_writer.writeBytes(data, length);
	return true;
}

bool WriteStream::serializeCheck(const char* string)
{
#if RM_SERIALIZE_CHECK
	int32_t hash = crcFast(reinterpret_cast<const unsigned char*>(string),
		static_cast<int32_t>(strlen(string)));
	serializeBits(hash, 32);
#endif
	return true;
}
