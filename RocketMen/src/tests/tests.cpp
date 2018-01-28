
#include <utility/bitstream.h>
#include <utility/utility.h>

struct SerializationTestStruct
{
	SerializationTestStruct() 
	{
		ivalue = 5;
		fvalue = 0.14f;
		rand_ivalue = rand();
		vector = Vector3(1.03f, 2.04f, 3.05f);
		vector2 = Vector3(0.0f, 0.14f, 1.66f);
		fvalue2 = 800.0123f;
		ivalue2 = rand();
		vector3 = Vector2(0.545f, -0.1f);
		vector4 = Vector2(0.001f, -5.01f);
	}

	int32_t ivalue;
	float   fvalue;
	int32_t rand_ivalue;
	Vector3 vector;
	Vector3 vector2;
	float   fvalue2;
	int32_t ivalue2;
	Vector2 vector3;
	Vector2 vector4;

	template<typename Stream>
	bool serialize(Stream& stream)
	{
		assert(serializeCheck(stream, "begin_SerializationTestStruct"));
		serializeInt(stream, ivalue, 0, 10);
		assert(serializeFloat(stream, fvalue, 0.0f, 1.0f, 0.01f));
		serializeInt(stream, rand_ivalue);
		serializeVector3(stream, vector);
		serializeVector3(stream, vector2, 0.0f, 2.0f, 0.1f);
		serializeInt(stream, ivalue2);
		assert(serializeVector2(stream, vector3));
		assert(serializeFloat(stream, fvalue2));
		assert(serializeVector2(stream, vector3));
		serializeInt(stream, ivalue2);
		assert(serializeVector2(stream, vector3));
		assert(serializeVector2(stream, vector4));
		assert(serializeCheck(stream, "end_SerializationTestStruct"));
		return true;
	}
};

bool testMeasureStream()
{
	WriteStream writeStream(32);
	MeasureStream measureStream;

	const int32_t numInts = rand() % 4 + 1;
	for (int32_t i = 0; i < numInts; i++)
	{
		int32_t integer = rand();
		serializeInt(writeStream, integer);
		serializeInt(measureStream, integer);
	}
	
	const int32_t numBytes = rand() % 10 + 1;
	char* data = new char[numBytes];
	serializeData(writeStream, data, numBytes);
	serializeData(measureStream, data, numBytes);
	delete[] data;

	const int32_t numBits = rand() % 32 + 1;
	int32_t randomBits = rand();
	serializeBits(writeStream, randomBits, numBits);
	serializeBits(measureStream, randomBits, numBits);
	
	const int32_t measuredSize = measureStream.getMeasuredBytes();
	const int32_t writeSize = writeStream.getDataLength();
	assert(measuredSize == writeSize);
	if (measuredSize != writeSize)
	{
		return false;
	}

	return true;
}

bool testSerialization()
{
	if (!testMeasureStream())
	{
		return false;
	}

	SerializationTestStruct testStruct;
	WriteStream writeStream(256);

	char readBuffer[256];
	ReadStream readStream(readBuffer, roundTo(250, 4));
	MeasureStream measureStream;

	testStruct.serialize(measureStream);
	testStruct.serialize(writeStream);
	writeStream.flush();

	const int32_t measuredSize = measureStream.getMeasuredBytes();
	const int32_t writeSize = writeStream.getDataLength();
	assert(measuredSize == writeSize);

	memcpy(readStream.getData(), writeStream.getData(), writeStream.getDataLength());

	SerializationTestStruct receiveStruct;
	receiveStruct.serialize(readStream);

	if (!assert(receiveStruct.ivalue2 == testStruct.ivalue2))
	{
		return false;
	}
	if (!assert(receiveStruct.ivalue == testStruct.ivalue))
	{
		return false;
	}

	if (!assert(receiveStruct.fvalue == testStruct.fvalue))
	{
		return false;
	}

	if (!assert(receiveStruct.rand_ivalue == testStruct.rand_ivalue))
	{
		return false;
	}

	if (!assert(glm::distance(receiveStruct.vector, testStruct.vector) < 0.01f))
	{
		return false;
	}

	if(!assert(glm::distance(receiveStruct.vector2, testStruct.vector2) < 0.1f))
	{
		return false;
	}

	if(!assert(receiveStruct.fvalue2 == testStruct.fvalue2))
	{
		return false;
	}

	if(!assert(receiveStruct.ivalue2 == testStruct.ivalue2))
	{
		return false;
	}

	if (!assert(glm::distance(receiveStruct.vector3, testStruct.vector3) < 0.01f))
	{
		return false;
	}

	if (!assert(glm::distance(receiveStruct.vector4, testStruct.vector4) < 0.01f))
	{
		return false;
	}

	return true;
}