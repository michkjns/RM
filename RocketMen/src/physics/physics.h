
#pragma once

#include <common.h>
#include <physics/rigidbody.h>
#include <physics/staticbody.h>

#include <vector>

namespace physics
{
	static const uint32_t s_maxRigidbodies = 64;

	struct Fixture
	{
		float friction    = 0.2f;
		float restitution = 0.0f;
		float density     = 0.0f;
		bool  isSensor    = false;
		uint16_t category = 0x0001;
		uint16_t mask     = 0xFFFF;
	};
};

class Entity;

//==============================================================================

class Physics
{
public:
	void initialize();
	void step(float timestep);

	static Rigidbody*  createCharacterBody(const Vector2& dimensions,
										   Entity* owner);

	static Rigidbody*  createBoxRigidbody(const Vector2& dimensions,
	                                      const physics::Fixture& fixture,
										  Entity* owner);

	static Staticbody* createStaticBody(const Vector2& position, 
										const Vector2& dimensions,
										const physics::Fixture& fixture);

	static bool        destroyStaticbody(Staticbody* sb);
	static bool        destroyRigidbody(Rigidbody* rb);
	
	static void        blastExplosion(const Vector2& position, float radius,
									  float power);

	static void        loadCollisionFromTilemap(const std::string& tilemap);
	static void        drawDebug();

	static void        destroyBodies();

private:
	uint32_t m_rigidbodyIDCounter;
};

