
/* 	http://gafferongames.com/building-a-game-network-protocol/
*/

#pragma once

#include <common.h>

#include <algorithm>
#include <memory>

#define RM_SERIALIZE_CHECK 1

class BitReader
{
public:
	BitReader(const char* data, int32_t numBytes);
	~BitReader();

	int32_t readBits(int32_t numBits);
	void readBytes(char* dest, int32_t numBytes);

	char*  getData() const { return reinterpret_cast<char*>(m_data); }
	int32_t getDataLength() const { return m_size; }

	bool alignToByte();

private:
	uint64_t  m_scratch;
	uint32_t* m_data;
	int32_t   m_scratchBits;
	int32_t   m_numWords;
	int32_t   m_wordIndex;
	int32_t   m_size;
	int32_t   m_numBitsRead;
	int32_t   m_numBits;
};

class BitWriter
{
public:
	BitWriter(char* buffer, int32_t numBytes);
	~BitWriter();

	void writeBits(uint32_t value, int32_t numBits);
	void writeBytes(const char* data, int32_t numBytes);
	void flush();

	char*   getData() const { return reinterpret_cast<char*>(m_data); }
	int32_t getDataLength() const { return (m_numBitsWritten + 7) / 8; }

	bool alignToByte();
private:
	uint64_t  m_scratch;
	uint32_t* m_data;
	int32_t   m_scratchBits;
	int32_t   m_numWords;
	int32_t   m_wordIndex;
	int32_t   m_size;
	int32_t   m_numBits;
	int32_t   m_numBitsWritten;
};

class WriteStream
{
public:
	static const bool isReading = false;
	static const bool isWriting = true;

	WriteStream(int32_t numBytes);
	~WriteStream();

	bool serializeBits(uint32_t value, int32_t numBits);
	bool serializeBool(bool& value);
	bool serializeInt(int32_t& value, int32_t min, int32_t max);
	bool serializeInt(uint32_t& value, uint32_t min, uint32_t max);
	bool serializeByte(const char byte);
	bool serializeData(const char* data, int32_t dataLength);
	bool serializeCheck(const char* string);

	void alignToByte() { m_writer.alignToByte(); }
	void flush() { m_writer.flush(); }
	void release() { m_buffer = nullptr; }
	void skipBits(int32_t /*numBits*/) { ASSERT(false, "Attempted to skip bits on a WriteStream"); }

	char*  getData() const { return m_buffer; }
	inline int32_t getDataLength() const { return m_writer.getDataLength(); }
	inline int32_t getBufferSize() const { return m_size; }

private:
	char* m_buffer;
	BitWriter m_writer;
	int32_t m_size;
};

class ReadStream
{
public:
	static const bool isReading = true;
	static const bool isWriting = false;

	/** numBytes must be rounded to 32 bits */
	ReadStream(const char* sourceData, int32_t numBytes);
	~ReadStream();

	bool serializeBits(uint32_t& value, int32_t numBits);
	bool serializeBool(bool& dest);
	bool serializeInt(int32_t& dest, int32_t min, int32_t max);
	bool serializeInt(uint32_t& value, uint32_t min, uint32_t max);
	bool serializeByte(char& dest);
	bool serializeData(char* dest, int32_t length);
	bool serializeCheck(const char* string);
	void flush() {}
	bool alignToByte() { return m_reader.alignToByte(); }
	void skipBits(int32_t numBits) {
		for (int32_t i = 0; i < numBits; i++)
		{
			m_reader.readBits(1);
		}
	}

	char*  getData() const { return m_reader.getData(); }
	inline int32_t getDataLength() const { return m_size; }

private:
	char* m_buffer;
	BitReader m_reader;
	int32_t m_size;
};

class MeasureStream
{
public:
	static const bool isReading = false;
	static const bool isWriting = true;

	MeasureStream() { m_numBitsMeasured = 0; }
	~MeasureStream() {}

	bool serializeBits(uint32_t value, int32_t numBits);
	bool serializeBool(bool dest);
	bool serializeInt(int32_t dest, int32_t min, int32_t max);
	bool serializeInt(uint32_t value, uint32_t min, uint32_t max);
	bool serializeByte(char dest);
	bool serializeData(const char* dest, int32_t length);
	bool serializeCheck(const char* string);
	bool alignToByte();

	inline int32_t getMeasuredBytes() const { return (m_numBitsMeasured + 7) / 8; }
	inline int32_t getMeasuredBits() const { return m_numBitsMeasured; }

private:
	int32_t m_numBitsMeasured;
};

/* Clamp value n to [lower, upper]
* @return clamped value n
*/
template <typename T>
T clamp(const T& n, const T& lower, const T& upper)
{
	return std::max(lower, std::min(n, upper));
}

/* Calculates number of bits required to represent an integer in range [min, max] */
inline int32_t bitsRequired(uint32_t min, uint32_t max)
{
	return int32_t((min == max) ? 0 : floor(log2(max - min) + 1));
}

/* Serialize (0, 32] number of bits */
template<typename Stream, typename T>
void serializeBits(Stream& stream, T& value, int32_t numBits)
{
	ASSERT(numBits > 0);
	ASSERT(numBits <= 32);
	uint32_t var32 = static_cast<uint32_t>(value);
	stream.serializeBits(var32, numBits);
	value = static_cast<T>(var32);
}

/* Serialize a bool or single bit, 0 or 1 */
template<typename Stream>
void serializeBool(Stream& stream, bool& value)
{
	stream.serializeBool(value);
}

/* Serialize a number of bytes */
template<typename Stream>
void serializeData(Stream& stream, char* data, int32_t length)
{
	ASSERT(data != nullptr);
	ASSERT(length > 0);
	stream.serializeData(data, length);
}

/* Serialize an unsigned integer value compressed between range [min, max] */
template<typename Stream>
void serializeInt(Stream& stream, uint32_t& value, uint32_t min, uint32_t max)
{
	ASSERT(min < max);

	if (Stream::isWriting)
	{
		ASSERT(value >= min);
		ASSERT(value <= max);
	}

	stream.serializeInt(value, min, max);

	if (Stream::isReading)
	{
		ASSERT(value >= min);
		ASSERT(value <= max);
	}
}

/* Serialize a 32-bit signed integer value  */
template<typename Stream>
void serializeInt(Stream& stream, int32_t& value)
{
	stream.serializeBits((uint32_t&)value, 32);
}

/* Serialize a 32-bit unsigned integer value  */
template<typename Stream>
void serializeInt(Stream& stream, uint32_t& value)
{
	stream.serializeBits(value, 32);
}

/* Serialize a signed integer value compressed between range [min, max] */
template<typename Stream>
void serializeInt(Stream& stream, int32_t& value, int32_t min, int32_t max)
{
	ASSERT(min < max, "min was larger than max");

	if (Stream::isWriting)
	{
		ASSERT(value >= min);
		ASSERT(value <= max);
	}

	stream.serializeInt(value, min, max);

	if (Stream::isReading)
	{
		ASSERT(value >= min, "value out of range");
		ASSERT(value <= max, "value out of range");
	}
}

/* Serialize a 32-bit float value, uncompressed */
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

/* Serialize a 32-bit float value within range [min, max] with 
*  an explicit precision, compressed */
template<typename Stream>
bool serializeFloat(Stream& stream, float& value, float min, 
                        float max, float precision)
{
	ASSERT(min < max, "min was smaller than max");
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

/* Serialize a vector2 compressed within range [min, max] with an explicit
   precision */
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

/* Serialize a vector2, uncompressed */
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

/* Serialize a vector3 compressed within range [min, max] with an explicit
precision */
template<typename Stream>
bool serializeVector3(Stream& stream, Vector3& vector, float min, 
                      float max, float precision)
{
	float values[3];
	if (Stream::isWriting)
	{
		values[0] = vector[0];
		values[1] = vector[1];
		values[2] = vector[2];
	}
	
	if (!serializeFloat(stream, values[0], min, max, precision))
		return false;
	if (!serializeFloat(stream, values[1], min, max, precision))
		return false;
	if (!serializeFloat(stream, values[2], min, max, precision))
		return false;

	if (Stream::isReading)
	{
		vector = Vector3(values[0], values[1], values[2]);
	}

	return true;
}

/* Serialize a vector3, uncompressed */
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

template<typename Stream>
bool serializeCheck(Stream& stream, const char* string)
{
	const bool success = stream.serializeCheck(string);
	ASSERT(success, "Serialize Check failed");
	return success;
}
