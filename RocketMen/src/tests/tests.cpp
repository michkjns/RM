
#include <utility/bitstream.h>

#include <game/character.h>

using namespace rm;

struct TestStruct
{
	TestStruct() 
	{
		value = 5;
		fvalue = 0.14f;
		vector = Vector3(1.03f, 2.04f, 3.05f);
		vector2 = Vector3(0.0f, 0.14f, 1.66f);
		fvalue2 = 800.0123f;
		value2 = 1234567;
		vector3 = Vector2(0.545f, -0.1f);
		vector4 = Vector2(0.001f, -5.01f);
	}

	int32_t value;
	float   fvalue;
	Vector3 vector;
	Vector3 vector2;
	float   fvalue2;
	int32_t value2;
	Vector2 vector3;
	Vector2 vector4;

	template<typename Stream>
	bool serialize(Stream& stream)
	{
		assert(serializeCheck(stream, "TestStruct_start"));
		serializeInt(stream, value, 0, 10);
		assert(serializeFloat(stream, fvalue, 0.0f, 1.0f, 0.01f));
		serializeVector3(stream, vector);
		serializeVector3(stream, vector2, 0.0f, 2.0f, 0.1f);
		serializeInt(stream, value2);
		assert(serializeVector2(stream, vector3));
		assert(serializeFloat(stream, fvalue2));
		assert(serializeVector2(stream, vector3));
		serializeInt(stream, value2);
		assert(serializeVector2(stream, vector3));
		assert(serializeVector2(stream, vector4));
		assert(serializeCheck(stream, "TestStruct_end"));
		return true;
	}
};

bool bitstreamTests()
{
	TestStruct testStruct;
	Character character;

	WriteStream writeStream(256);

	char readBuffer[256];
	ReadStream readStream(readBuffer, 256);
	
	testStruct.serialize(writeStream);
	writeStream.flush();
	memcpy(readStream.getData(), writeStream.getData(), writeStream.getDataLength());

	TestStruct receivePacket;
	receivePacket.serialize(readStream);

	if(!assert(receivePacket.value2 == testStruct.value2))
	{
		return false;
	}
	if(!assert(receivePacket.value == testStruct.value))
	{
		return false;
	}

	if(!assert(receivePacket.fvalue == testStruct.fvalue))
	{
		return false;
	}

	if(!assert(glm::distance(receivePacket.vector, testStruct.vector) < 0.01f))
	{
		return false;
	}

	if(!assert(glm::distance(receivePacket.vector2, testStruct.vector2) < 0.1f))
	{
		return false;
	}

	if(!assert(receivePacket.fvalue2 == testStruct.fvalue2))
	{
		return false;
	}

	if(!assert(receivePacket.value2 == testStruct.value2))
	{
		return false;
	}

	if (!assert(glm::distance(receivePacket.vector3, testStruct.vector3) < 0.01f))
	{
		return false;
	}

	if (!assert(glm::distance(receivePacket.vector4, testStruct.vector4) < 0.01f))
	{
		return false;
	}

	return true;
}