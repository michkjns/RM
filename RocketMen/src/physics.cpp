
#include <physics.h>

#include <core/entity.h>
#include <core/resource_manager.h>
#include <graphics/camera.h>
#include <graphics/renderer.h>
#include <network.h>
#include <physics/physics_box2d.h>

#include <array>

using namespace physics;

static std::vector<Rigidbody*>  s_rigidbodies;
static std::vector<Staticbody*> s_staticbodies;

#ifdef PHYSICS_BOX2D

Vector2 toglm(const b2Vec2& a) {
	return Vector2(a.x, a.y);
}

b2Vec2 tob2(const Vector2& a) {
	return b2Vec2(a.x, a.y);
}

static b2Vec2  s_gravity(0.0f, -9.80f);
       b2World g_boxWorld(s_gravity);

class PhysicsDraw : public b2Draw
{
public:
	void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);
	void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);
	void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color) {}
	void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color) {}
	void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color);
	void DrawTransform(const b2Transform& xf) {}
};

//==============================================================================
class ContactListener : public b2ContactListener
{
	void BeginContact(b2Contact* contact);
	void EndContact(b2Contact* contact);
};

//==============================================================================

class QueryCallback : public b2QueryCallback 
{
public:
	std::vector<b2Body*> foundBodies;
	bool ReportFixture(b2Fixture* fixture) {
		foundBodies.push_back(fixture->GetBody());
		return true;
	}
};

static PhysicsDraw     s_debugDrawInterface;
static ContactListener s_contactListener;

//==============================================================================

void Physics::initialize()
{
	m_rigidbodyIDCounter = 0;
	g_boxWorld.SetDebugDraw(&s_debugDrawInterface);
	g_boxWorld.SetContactListener(&s_contactListener);
	s_debugDrawInterface.SetFlags(b2Draw::e_shapeBit);
}

void Physics::step(float timestep)
{
	g_boxWorld.Step(timestep, 6, 2);
}

Rigidbody* Physics::createCharacterBody(const Vector2& dimensions, Entity* owner)
{
	Rigidbody* rb = new Rigidbody();
	rb->getImpl()->SetFixedRotation(true);

	b2PolygonShape dynamicBox;
	dynamicBox.SetAsBox(dimensions.x / 2.0f, dimensions.y / 2.0f);

	b2FixtureDef fixtureDef;
	fixtureDef.shape       = &dynamicBox;
	fixtureDef.density     = 1.0f;
	fixtureDef.friction    = 5.0f;


	rb->getImpl()->CreateFixture(&fixtureDef);
	rb->getImpl()->SetUserData(owner);

	s_rigidbodies.push_back(rb);
	return s_rigidbodies.back();
}

Rigidbody* Physics::createBoxRigidbody(const Vector2& dimensions, const Fixture& fixture, Entity* owner)
{
	Rigidbody* rb = new Rigidbody();
	
	b2PolygonShape dynamicBox;
	dynamicBox.SetAsBox(dimensions.x / 2.0f, dimensions.y / 2.0f);

	b2FixtureDef fixtureDef;
	fixtureDef.shape       = &dynamicBox;
	fixtureDef.density     = fixture.density;
	fixtureDef.friction    = fixture.friction;
	fixtureDef.restitution = fixture.restitution;
	fixtureDef.isSensor    = fixture.isSensor;
	fixtureDef.userData    = owner;

	rb->getImpl()->CreateFixture(&fixtureDef);
	rb->getImpl()->SetUserData(owner);

	s_rigidbodies.push_back(rb);
	return s_rigidbodies.back();
}

Staticbody* Physics::createStaticBody(const Vector2& position, const Vector2& dimensions, const Fixture& fixture)
{
	Staticbody* sb = new Staticbody(position);

	b2PolygonShape staticBox;
	staticBox.SetAsBox(dimensions.x / 2.0f, dimensions.y / 2.0f);

	b2FixtureDef fixtureDef;
	fixtureDef.shape       = &staticBox;
	fixtureDef.density     = fixture.density;
	fixtureDef.friction    = fixture.friction;
	fixtureDef.restitution = fixture.restitution;

	sb->getImpl()->CreateFixture(&fixtureDef);


	s_staticbodies.push_back(sb);
	return sb;
}

bool Physics::destroyRigidbody(Rigidbody* rb)
{
	if (!rb) 
		return false;

	for (auto it = s_rigidbodies.begin(); it != s_rigidbodies.end(); it++)
	{
		if ((*it) == rb)
		{
			g_boxWorld.DestroyBody(static_cast<b2Body*>(rb->getImpl()));
			delete (*it);
			s_rigidbodies.erase(it);
			return true;
		}
	}

	/** Rigidbody rb not found! */
	assert(false);
	return false;
}

void _applyBlastImpulse(b2Body* body, b2Vec2 pos, float force)
{
	b2Vec2 blastDir = body->GetWorldCenter() - pos;
	float distance = blastDir.Normalize();

	if (distance == 0)
		return;

	float invDistance = 1 / distance;
	float impulseMag = force * invDistance * invDistance;
	body->ApplyLinearImpulse(impulseMag * blastDir, body->GetWorldCenter(), true);
}

void Physics::blastExplosion(Vector2 position, float radius, float power)
{
	QueryCallback queryCallback;
	b2AABB aabb;
	aabb.lowerBound = tob2(position) - b2Vec2(radius, radius);
	aabb.upperBound = tob2(position) + b2Vec2(radius, radius);
	g_boxWorld.QueryAABB(&queryCallback, aabb);

	for (int i = 0; i < queryCallback.foundBodies.size(); i++) {
		b2Body* body = queryCallback.foundBodies[i];
		b2Vec2 bodyCom = body->GetWorldCenter();

		//ignore bodies outside the blast range
		if ((bodyCom - tob2(position)).Length() >= radius)
			continue;

		_applyBlastImpulse(body, tob2(position), power);
	}

}

bool Physics::destroyStaticbody(Staticbody* sb)
{
	if (!sb)
		return false;

	for (auto it = s_staticbodies.begin(); it != s_staticbodies.end(); it++)
	{
		if ((*it) == sb)
		{
			g_boxWorld.DestroyBody(static_cast<b2Body*>(sb->getImpl()));
			s_staticbodies.erase(it);
			return true;
		}
	}

	/** Staticbody sb not found! */
	assert(false);
	return false;
}

void Physics::generateWorld(std::string tilemapName)
{
#define GENERATE_WORLD_NAIVE
#ifdef GENERATE_WORLD_NAIVE
	TileMap& tilemap = ResourceManager::getTileMap(tilemapName.c_str());

	const Vector2 offset(-static_cast<float>(tilemap.getMapWidth() / 2.0f) + 0.5f, 
	                      static_cast<float>(tilemap.getMapHeight()/ 2.0f) - 0.5f);

	for (uint32_t x = 0; x < tilemap.getMapWidth(); x++)
	{
		for (uint32_t y = 0; y < tilemap.getMapHeight(); y++)
		{
			physics::Fixture fixture;
			char tile = tilemap.getMap()[x + y * tilemap.getMapWidth()];
			if (tile != '0')
			{
				
				Physics::createStaticBody(Vector2(x, -1.f * y) + offset,
				                          Vector2(1, 1.0f), fixture);

				/*Physics::createStaticBody(Vector2(x*2.f  - width/2, (-(float)y*2.f + height/2))
										  + offset,
										  Vector2(1, 1.0f), fixture);*/
			}
		}
	}
#endif
}

void Physics::drawDebug()
{
	g_boxWorld.DrawDebugData();
}

void Physics::destroyBodies()
{
	for (auto it = s_staticbodies.begin(); it != s_staticbodies.end();)
	{

		g_boxWorld.DestroyBody(static_cast<b2Body*>((*it)->getImpl()));
		delete (*it);
		it = s_staticbodies.erase(it);
	}

	for (auto it = s_rigidbodies.begin(); it != s_rigidbodies.end(); )
	{

		g_boxWorld.DestroyBody(static_cast<b2Body*>((*it)->getImpl()));
		delete (*it);
		it = s_rigidbodies.erase(it);
	}
}

//==============================================================================

void PhysicsDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	Renderer::get()->drawPolygon((Vector2*)(vertices), vertexCount, Color(color.r, color.g, color.b, color.a));
}

void PhysicsDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	Renderer::get()->drawPolygon((Vector2*)vertices, vertexCount, Color(color.r, color.g, color.b, color.a));
}

void PhysicsDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) 
{
	Renderer::get()->drawLineSegment(toglm(p1), toglm(p2), Color(color.r, color.g, color.b, color.a));
}

//==============================================================================

void ContactListener::BeginContact(b2Contact* contact)
{
//	LOG_DEBUG("ContactListenerL::startContact");

	if (!Network::isServer())
	{
		return;
	}

	Entity* entityA = static_cast<Entity*>(contact->GetFixtureA()->GetBody()->GetUserData());
	Entity* entityB = static_cast<Entity*>(contact->GetFixtureB()->GetBody()->GetUserData());

	if (entityA)
	{
		static_cast<Entity*>(entityA)->startContact(entityB);
	}

	if (entityB)
	{
		static_cast<Entity*>(entityB)->startContact(entityA);
	}
}

void ContactListener::EndContact(b2Contact* contact)
{
	if (!Network::isServer())
	{
		return;
	}

	Entity* entityA = static_cast<Entity*>(contact->GetFixtureA()->GetBody()->GetUserData());
	Entity* entityB = static_cast<Entity*>(contact->GetFixtureB()->GetBody()->GetUserData());
	if (entityA)
	{
		static_cast<Entity*>(entityA)->endContact(entityB);
	}

	if (entityB)
	{
		static_cast<Entity*>(entityB)->endContact(entityA);
	}
}

#endif // PHYSICS_BOX2D