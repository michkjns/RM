
/* New bitstream read/write classes
	Based on
	http://gafferongames.com/building-a-game-network-protocol/
	Many thanks go to Glenn Fiedler for his awesome articles.
*/

#pragma once

#include <includes.h>

#include <algorithm>
#include <cstdint>
#include <memory>

class WriteStream
{
public:
	static const bool isReading = false;
	static const bool isWriting = true;

	void flush(bool increment);
	void serializeBits(uint32_t value, uint32_t numBits);
	void serializeBool(bool& value);
	void serializeInt(int32_t& value, int32_t min, int32_t max);
	void serializeByte(const uint8_t byte);
	void serializeData(const uint8_t* data, int32_t dataLength);
	int32_t getLength() { return (m_wordIndex + 1) * 4; }

	uint64_t  m_scratch;
	int32_t   m_scratchBits;
	int32_t   m_wordIndex;
	uint32_t* m_buffer;
	int32_t   m_bufferLength;
	bool      m_isFull;
	
OnDebug(
	int32_t m_bitsWritten
);

};

class ReadStream
{
public:
	static const bool isReading = true;
	static const bool isWriting = false;

	void flush(bool = false);
	void serializeBits(uint32_t& value, uint32_t numBits);
	void serializeBool(bool& dest);
	void serializeInt(int32_t& dest, int32_t min, int32_t max);
	void serializeByte(uint8_t& dest);
	void serializeData(uint8_t* dest, int32_t length);

	uint64_t  m_scratch;
	int32_t   m_scratchBits;
	int32_t   m_numBitsRead;
	int32_t   m_wordIndex;
	int32_t   m_bufferLength;
	uint32_t* m_buffer;
	bool      m_corrupted;
OnDebug(
	int32_t m_bitsRead;
);

};

template <typename T>
T clamp(const T& n, const T& lower, const T& upper)
{
	return std::max(lower, std::min(n, upper));
}

inline int32_t bitsRequired(uint32_t min, uint32_t max)
{
	return int32_t((min == max) ? 0 : floor(log2(max - min) + 1));
}

template<typename Stream>
void serializeBit(Stream& stream, bool& value)
{
	stream.serializeBool(value);
}

template<typename Stream>
void serializeInt(Stream& stream, uint32_t& value, uint32_t min = INT32_MIN,
				  uint32_t max = INT32_MAX)
{
	assert(min < max);

	if (Stream::isWriting)
	{
		assert(value >= min);
		assert(value <= max);
	}

	stream.serializeInt(value, min, max);

	if (Stream::isReading)
	{
		assert(value >= min);
		assert(value <= max);
	}
}

template<typename Stream>
void serializeInt(Stream& stream, int32_t& value, int32_t min = INT32_MIN, 
                  int32_t max = INT32_MAX)
{
	assert(min < max);

	if (Stream::isWriting)
	{
		assert(value >= min);
		assert(value <= max);
	}

	stream.serializeInt(value, min, max);

	if (Stream::isReading)
	{
		assert(value >= min);
		assert(value <= max);
	}
}

template<typename Stream>
bool serializeFloat(Stream& stream, float& value)
{
	union Value
	{
		float  asFloat;
		uint32_t asInt;
	} _value = {};

	if (Stream::isWriting)
		_value.asFloat = value;

	stream.serializeBits(_value.asInt, 32);

	if (Stream::isReading)
		value = _value.asFloat;

	return true;
}

template<typename Stream>
bool serializeFloat(Stream& stream, float& value, float min, 
                        float max, float precision)
{
	assert(min < max);
	if (min > max) return false;

	const float delta = max - min;
	const float values = delta / precision;
	const uint32_t maxIntValue = static_cast<uint32_t>(ceil(values));
	const int32_t bits = bitsRequired(0, maxIntValue);
	uint32_t intValue = 0;

	if (Stream::isWriting)
	{
		const float normalizedValue = clamp((value - min) / delta, 0.0f, 1.0f);
		intValue = static_cast<uint32_t>(floor(normalizedValue *
											   maxIntValue + 0.5f));
	}

	stream.serializeBits(intValue, bits);

	if (Stream::isReading)
	{
		const float normalizedValue = intValue / float(maxIntValue);
		value = normalizedValue * delta + min;
		if (value < min || value > max)
			return false;
	}

	return true;
}

template<typename Stream>
bool serializeVector2(Stream& stream, Vector2& vector, float min,
                      float max, float precision)
{
	float& value0 = vector[0];
	float& value1 = vector[1];

	if (!serializeFloat(stream, value0, min, max, precision))
		return false;
	if (!serializeFloat(stream, value1, min, max, precision))
		return false;

	return true;
}

template<typename Stream>
bool serializeVector2(Stream& stream, Vector2& vector)
{
	float& value0 = vector[0];
	float& value1 = vector[1];

	if(!serializeFloat(stream, value0))
		return false;
	if(!serializeFloat(stream, value1))
		return false;

	return true;
}

template<typename Stream>
void serializeVector3(Stream& stream, Vector3& vector, float min, 
                      float max, float precision)
{
	float values[3];
	if (Stream::isWriting)
	{
		values[0] = vector[0];
		values[1] = vector[1];
		values[2] = vector[2];
	}
	
	serializeFloat(stream, values[0], min, max, precision);
	serializeFloat(stream, values[1], min, max, precision);
	serializeFloat(stream, values[2], min, max, precision);

	if (Stream::isReading)
	{
		vector = Vector3(values[0], values[1], values[2]);
	}
}

template<typename Stream>
void serializeVector3(Stream& stream, Vector3& vector)
{
	float values[3];
	if (Stream::isWriting)
	{
		values[0] = vector[0];
		values[1] = vector[1];
		values[2] = vector[2];
	}

	serializeFloat(stream, values[0]);
	serializeFloat(stream, values[1]);
	serializeFloat(stream, values[2]);

	if (Stream::isReading)
	{
		vector = Vector3(values[0], values[1], values[2]);
	}
}

#if 1

/** (Old) BitStream class
*   for compact serialization
*   @note Memory intensive and slow, needs to be replaced
*/
class BitStream
{
public:
	BitStream();
	BitStream(const BitStream& bs);
	~BitStream();
	BitStream& BitStream::operator= (const BitStream&);

	/** Write data replacing the current buffer */
	void writeBuffer(const char* data, size_t length);
																		
	/** Write data appending to the current buffer */
	void writeData(const char* data, size_t length);
				 											
	void writeBit(bool value, size_t repeat = 1);
	void writeByte(char value, size_t repeat = 1);
	void writeFloat(float value);
	void writeInt8(int8_t value);
	void writeInt16(int16_t value);
	void writeInt32(int32_t value);
	void writeInt64(int64_t value);
	void writeBool(bool value);

	void    readBytes(char* output, size_t numBytes = 1);
	void    readBit(bool* output);
	float	readFloat();
	int8_t  readInt8();
	int16_t	readInt16();
	int32_t	readInt32();
	int64_t	readInt64();
	bool	readBool();

	const size_t getLength() const;
	const char*	 getBuffer() const;
	int32_t getReadTotalBytes() const;
	void resetReading();
	void resetWriting();
	void reset();
	
private:
	static const uint32_t s_bufferSize = 65536;

	std::unique_ptr<char[]> m_buffer;
	int32_t     m_readTotalBytes;
	int32_t	    m_readBit;
	const char* m_readData;

	int32_t     m_writeTotalBytes;
	int32_t     m_writeLength;
	int32_t     m_writeByte;
	int32_t     m_writeBit;
	char*       m_writeData;
};
#endif
