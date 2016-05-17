
#include <physics.h>

#include <physics/rigidbody_data.h>

using namespace physics;

#ifdef PHYSICS_BOX2D

static b2Vec2  s_gravity(0.0f, -10.0f);
b2World g_world(s_gravity);
static Physics* g_physics;

void Physics::initialize()
{
	m_rigidbodyIDCounter = 0;
	b2BodyDef groundBodyDef;
	groundBodyDef.position.Set(0.0f, -10.0f);

	b2Body* groundBody = g_world.CreateBody(&groundBodyDef);

	b2PolygonShape groundBox;
	groundBox.SetAsBox(50.0f, 10.0f);

	groundBody->CreateFixture(&groundBox, 0.0f);

	g_physics = this;
}

void Physics::step(float timestep)
{
	g_world.Step(timestep, 6, 2);
}

Rigidbody* Physics::createRigidbody()
{
	Rigidbody* rb = new Rigidbody();
	g_physics->m_rigidbodies.push_back(rb);
	return g_physics->m_rigidbodies.back();
}

bool Physics::destroyRigidbody(Rigidbody* rb)
{
	assert(rb != nullptr);

	for (auto it = g_physics->m_rigidbodies.begin(); it != g_physics->m_rigidbodies.end(); it++)
	{
		if ((*it) == rb)
		{
			g_world.DestroyBody(static_cast<b2Body*>(rb->getData()));
			g_physics->m_rigidbodies.erase(it);
			return true;
		}
	}

	/** Rigidbody rb not found! */
	assert(false);
	return false;
}

#endif // PHYSICS_BOX2D