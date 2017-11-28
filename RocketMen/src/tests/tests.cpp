
#include <utility/bitstream.h>

#include <game/character.h>
#include <game/rocket.h>
#include <physics/rigidbody.h>

using namespace rm;

struct SamplePacket
{
	bool    value;
	int32_t value2;
//	Rocket* rocket;

	template<typename Stream>
	bool serialize(Stream& stream)
	{
		serializeBit(stream, value);
		serializeInt(stream, value2, 100, 130);
	//	rocket->serializeFull(stream);
		return true;
	}
};

struct SamplePacket2
{
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
		serializeInt(stream, value, 0, 10);
		serializeFloat(stream, fvalue, 0.0f, 1.0f, 0.01f);
		serializeVector3(stream, vector);
		serializeVector3(stream, vector2, 0.0f, 2.0f, 0.1f);
		serializeInt(stream, value2);
		serializeVector2(stream, vector3);
		serializeFloat(stream, fvalue2);
		serializeVector3(stream, vector2, 0.0f, 2.0f, 0.1f);
		serializeInt(stream, value2);		
		serializeVector3(stream, vector2, 0.0f, 2.0f, 0.1f);
		serializeInt(stream, value2);
		serializeVector2(stream, vector3);
		serializeInt(stream, value2);
		serializeFloat(stream, fvalue2);
		serializeVector3(stream, vector2, 0.0f, 2.0f, 0.1f);
		serializeInt(stream, value2);
		serializeVector3(stream, vector2, 0.0f, 2.0f, 0.01f);
		serializeVector2(stream, vector3);
		serializeVector3(stream, vector2, 0.0f, 4.0f, 0.01f);
		serializeVector2(stream, vector4);
		return true;
	}
};

bool streamTests()
{
	SamplePacket packet;
	SamplePacket2 packet2;
	Character character;

	packet.value = true;
	packet.value2 = 123;

	packet2.value   = 5;
	packet2.fvalue  = 0.14f;
	packet2.vector  = Vector3(1.03f, 2.04f, 3.05f);
	packet2.vector2 = Vector3(0.0f, 0.14f, 1.66f);
	packet2.fvalue2 = 800.0123f;
	packet2.value2  = 1234567;
	packet2.vector3 = Vector2(0.545f, -0.1f);
	packet2.vector4 = Vector2(0.001f, -5.01f);
	WriteStream writeStream(256);
	ReadStream  readStream(256);
	
	packet.serialize(writeStream);
	packet2.serialize(writeStream);
	memcpy(readStream.getBuffer(), writeStream.getBuffer(), writeStream.getLength());

	SamplePacket  receivePacket;
	SamplePacket2 receivePacket2;
	receivePacket.serialize(readStream);
	receivePacket2.serialize(readStream);

	if (!assert(receivePacket.value == packet.value))
	{
		return false;
	}

	if (!assert(receivePacket.value == packet.value))
	{
		return false;
	}
	if(!assert(receivePacket.value2 == packet.value2))
	{
		return false;
	}
	if(!assert(receivePacket2.value == packet2.value))
	{
		return false;
	}

	if(!assert(receivePacket2.fvalue == packet2.fvalue))
	{
		return false;
	}

	if(!assert(glm::distance(receivePacket2.vector, packet2.vector) < 0.01f))
	{
		return false;
	}

	if(!assert(glm::distance(receivePacket2.vector2, packet2.vector2) < 0.1f))
	{
		return false;
	}

	if(!assert(receivePacket2.fvalue2 == packet2.fvalue2))
	{
		return false;
	}

	if(!assert(receivePacket2.value2 == packet2.value2))
	{
		return false;
	}

	if (!assert(glm::distance(receivePacket2.vector3, packet2.vector3) < 0.01f))
	{
		return false;
	}

	if (!assert(glm::distance(receivePacket2.vector4, packet2.vector4) < 0.01f))
	{
		return false;
	}

	return true;
}