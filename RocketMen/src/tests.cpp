
#include <bitstream.h>

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
	WriteStream ws = {};
	ReadStream  rs = {};

	ws.m_buffer = new uint32_t[256];
	ws.m_bufferLength = 256;

	rs.m_buffer = new uint32_t[256];
	rs.m_bufferLength = 256;


	packet.value = true;
	packet.serialize(ws);
	packet2.serialize(ws);
	memcpy(rs.m_buffer, ws.m_buffer, ws.m_bufferLength);

	SamplePacket  r_packet;
	SamplePacket2 r_packet2;
//	r_packet.rocket = new Rocket();
	r_packet.serialize(rs);
	r_packet2.serialize(rs);

	if (r_packet.value != packet.value)
	{
		assert(false);
		return false;
	}
	if(r_packet.value2 != packet.value2)
	{
		assert(false);
		return false;
	}
	if(r_packet2.value != packet2.value)
	{
		assert(false);
		return false;
	}

	if(r_packet2.fvalue != packet2.fvalue)
	{
		assert(false);
		return false;
	}

	if(glm::distance(r_packet2.vector, packet2.vector) > 0.01f)
	{
		assert(false);
		return false;
	}

	if(glm::distance(r_packet2.vector2, packet2.vector2) > 0.1f)
	{
		assert(false);
		return false;
	}

	if(r_packet2.fvalue2 != packet2.fvalue2)
	{
		assert(false);
		return false;
	}

	if(r_packet2.value2 != packet2.value2)
	{
		assert(false);
		return false;
	}

	if (glm::distance(r_packet2.vector3, packet2.vector3) > 0.01f)
	{
		assert(false);
		return false;
	}

	if (glm::distance(r_packet2.vector4, packet2.vector4) > 0.01f)
	{
		assert(false);
		return false;
	}

	delete[] ws.m_buffer;
	delete[] rs.m_buffer;

	//ws = {};
	//rs = {};

	//ws.m_buffer = new uint32_t[256];
	//ws.m_bufferLength = 256;

	//rs.m_buffer = new uint32_t[256];
	//rs.m_bufferLength = 256;

	//Rocket* rocket1 = new Rocket();
	//rocket1->getTransform().setLocalPosition(Vector2(1.0f, 2.0f));
	//rocket1->initialize(&character, Vector2(1.0f, 0.0f), 20.0f, true);
	//rocket1->setNetworkID(0);

	//Rocket* rocket2 = new Rocket();
	//rocket1->serializeFull(ws);
	//memcpy(rs.m_buffer, ws.m_buffer, ws.m_bufferLength);
	//rocket2->serializeFull(rs);

	//Vector2 pos1 = rocket1->getTransform().getLocalPosition();
	//Vector2 pos2 = rocket2->getTransform().getLocalPosition();
	//Vector2 vel1 = rocket1->getRigidbody()->getLinearVelocity();
	//Vector2 vel2 = rocket2->getRigidbody()->getLinearVelocity();

	//if (glm::distance( pos1, pos2)> 0.01f)
	//{
	//	assert(false);
	//	return false;
	//}

	//if (glm::distance(vel1, vel2)> 0.01f)
	//{
	//	assert(false);
	//	return false;
	//}

	//delete[] ws.m_buffer;
	//delete[] rs.m_buffer;
	//
	//rocket1->kill();
	//rocket2->kill();

	return true;
}