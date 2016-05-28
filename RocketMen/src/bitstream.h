
#pragma once

#include <cstdint>

/** BitStream class
*   for compact serialization
*/
class BitStream
{
public:
	BitStream();
	~BitStream();

	/** Write data replacing the current buffer */
	void writeBuffer(char* data, size_t length);
																		
	/** Write data appending to the current buffer */
	void writeData(char* data, size_t length);
				 											
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

	void resetReading() ;
	
private:
	static const uint32_t s_bufferSize = 65536;

	char        m_buffer[s_bufferSize];
	int32_t     m_readTotalBytes;
	int32_t	    m_readBit;
	const char* m_readData;

	int32_t     m_writeTotalBytes;
	int32_t     m_writeLength;
	int32_t     m_writeByte;
	int32_t     m_writeBit;
	char*       m_writeData;
};
