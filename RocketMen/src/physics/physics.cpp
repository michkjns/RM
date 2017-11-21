
#include <physics/physics.h>

#include <core/debug.h>
#include <core/entity.h>
#include <core/resource_manager.h>
#include <graphics/camera.h>
#include <graphics/renderer.h>
#include <network/network.h>
#include <physics/physics_box2d.h>

#include <array>

using namespace physics;

static std::vector<Rigidbody*>  s_rigidbodies;
static std::vector<Staticbody*> s_staticbodies;

#ifdef PHYSICS_BOX2D

/** b2Vec2 to Vector2 */
Vector2 toVector2(const b2Vec2& a)
{
	return Vector2(a.x, a.y);
}

/** Vector2 to b2Vec2 */
b2Vec2 tob2(const Vector2& a) 
{
	return b2Vec2(a.x, a.y);
}

static b2Vec2  s_gravity(0.0f, -9.80f);
       b2World g_boxWorld(s_gravity);

class PhysicsDraw : public b2Draw
{
public:
	void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);
	void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);
	void DrawCircle(const b2Vec2& /*center*/, float32 /*radius*/, const b2Color& /*color*/) {};
	void DrawSolidCircle(const b2Vec2& /*center*/, float32 /*radius*/, const b2Vec2& /*axis*/, const b2Color& /*color*/) {};
	void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color);
	void DrawTransform(const b2Transform& /*xf*/) {};
};

// ============================================================================

class ContactListener : public b2ContactListener
{
	void BeginContact(b2Contact* contact);
	void EndContact(b2Contact* contact);
};

// ============================================================================

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

// ============================================================================

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
	Rigidbody* rigidBody = new Rigidbody();
	rigidBody->getImpl()->SetFixedRotation(true);

	b2PolygonShape dynamicBox;
	dynamicBox.SetAsBox(dimensions.x / 2.0f, dimensions.y / 2.0f);

	b2FixtureDef fixtureDef;
	fixtureDef.shape       = &dynamicBox;
	fixtureDef.density     = 1.0f;
	fixtureDef.friction    = 5.0f;


	rigidBody->getImpl()->CreateFixture(&fixtureDef);
	rigidBody->getImpl()->SetUserData(owner);

	s_rigidbodies.push_back(rigidBody);
	return s_rigidbodies.back();
}

Rigidbody* Physics::createBoxRigidbody(const Vector2& dimensions, const Fixture& fixture, Entity* owner)
{
	Rigidbody* rigidBody = new Rigidbody();
	
	b2PolygonShape dynamicBox;
	dynamicBox.SetAsBox(dimensions.x / 2.0f, dimensions.y / 2.0f);

	b2FixtureDef fixtureDef;
	fixtureDef.shape       = &dynamicBox;
	fixtureDef.density     = fixture.density;
	fixtureDef.friction    = fixture.friction;
	fixtureDef.restitution = fixture.restitution;
	fixtureDef.isSensor    = fixture.isSensor;
	fixtureDef.userData    = owner;

	rigidBody->getImpl()->CreateFixture(&fixtureDef);
	rigidBody->getImpl()->SetUserData(owner);

	s_rigidbodies.push_back(rigidBody);
	return s_rigidbodies.back();
}

Staticbody* Physics::createStaticBody(const Vector2& position, const Vector2& dimensions, const Fixture& fixture)
{
	Staticbody* staticBody = new Staticbody(position);

	b2PolygonShape staticBox;
	staticBox.SetAsBox(dimensions.x / 2.0f, dimensions.y / 2.0f);

	b2FixtureDef fixtureDef;
	fixtureDef.shape       = &staticBox;
	fixtureDef.density     = fixture.density;
	fixtureDef.friction    = fixture.friction;
	fixtureDef.restitution = fixture.restitution;

	staticBody->getImpl()->CreateFixture(&fixtureDef);

	s_staticbodies.push_back(staticBody);
	return staticBody;
}

bool Physics::destroyRigidbody(Rigidbody* rigidBody)
{
	assert(rigidBody != nullptr);

	for (auto it = s_rigidbodies.begin(); it != s_rigidbodies.end(); it++)
	{
		if ((*it) == rigidBody)
		{
			g_boxWorld.DestroyBody(static_cast<b2Body*>(rigidBody->getImpl()));
			delete (*it);
			s_rigidbodies.erase(it);
			return true;
		}
	}

	/** rigidbody not found! */
	assert(false);
	return false;
}

void _applyBlastImpulse(b2Body* body, b2Vec2 pos, float force)
{
	b2Vec2 blastDir = body->GetWorldCenter() - pos;
	float distance = blastDir.Normalize();

	if (distance != 0)
	{
		float invDistance = 1 / distance;
		float impulseMag = force * invDistance * invDistance;
		body->ApplyLinearImpulse(impulseMag * blastDir, body->GetWorldCenter(), true);
	}
}

void Physics::blastExplosion(Vector2 position, float radius, float power)
{
	QueryCallback queryCallback;
	b2AABB aabb;
	aabb.lowerBound = tob2(position) - b2Vec2(radius, radius);
	aabb.upperBound = tob2(position) + b2Vec2(radius, radius);
	g_boxWorld.QueryAABB(&queryCallback, aabb);

	for (int i = 0; i < queryCallback.foundBodies.size(); i++) 
	{
		b2Body* body = queryCallback.foundBodies[i];
		b2Vec2 bodyCom = body->GetWorldCenter();

		if ((bodyCom - tob2(position)).Length() < radius)
		{
			_applyBlastImpulse(body, tob2(position), glm::min(power, 2.0f));
		}
	}

}

bool Physics::destroyStaticbody(Staticbody* staticBody)
{
	assert(staticBody != nullptr);

	for (auto it = s_staticbodies.begin(); it != s_staticbodies.end(); it++)
	{
		if ((*it) == staticBody)
		{
			g_boxWorld.DestroyBody(static_cast<b2Body*>(staticBody->getImpl()));
			s_staticbodies.erase(it);
			return true;
		}
	}

	/** staticBody not found! */
	assert(false);
	return false;
}

void Physics::loadCollisionFromTilemap(std::string tilemapName)
{
	TileMap& tilemap = ResourceManager::getTileMap(tilemapName.c_str());

	if (tilemap.isInitalized() == false)
	{
		LOG_ERROR("Physics::loadCollisionFromTilemap - Invalid tilemap");
		ensure(false);
		return;
	}

	struct Rect
	{
		uint32_t x;
		uint32_t y;
		uint32_t w;
		uint32_t h;
	};
	
	std::vector<Rect> rects;
	char* tempMap = new char[tilemap.getMapWidth() * tilemap.getMapHeight()];
	memcpy(tempMap, tilemap.getMap(), tilemap.getMapWidth() * tilemap.getMapHeight());

	Rect currentRect;
	for (uint32_t x = 0; x < tilemap.getMapWidth(); x++)
	{
		for (uint32_t y = 0; y < tilemap.getMapHeight(); y++)
		{
			char tile = tempMap[x + y * tilemap.getMapWidth()];
			if (tile != '0')
			{
				currentRect.x = x;
				currentRect.y = y;
				currentRect.w = 0;
				currentRect.h = 0;
				bool _break = false;
				uint32_t y_lim = tilemap.getMapHeight();
				for (uint32_t x1 = x; x1 < tilemap.getMapWidth(); x1++)
				{
					if (_break) break;
					for (uint32_t y1 = y; y1 < y_lim; y1++)
					{
						char& tile1 = tempMap[x1 + y1 * tilemap.getMapWidth()];
						if (y1 == y)
						{
							if (tile1 != '0')
							{
								currentRect.w++;
								tile1 = '0';
								if (x1 == x || y1 > y + currentRect.h)
									currentRect.h++;
								continue;
							}
							else
							{
								_break = true;
								break;
							}
						}
						if (tile1 != '0')
						{
							tile1 = '0';
							if (x1 == x || y1 > y + currentRect.h)
							{
								currentRect.h++;
							}
							
						}
						else 
						{
							y_lim = y1;
							break;
						};
					}
				}
				rects.push_back(currentRect);
			}
		}
	}
	delete[] tempMap;

	physics::Fixture fixture;
	for (auto& rect : rects)
	{
		const Vector2 offset(-static_cast<float>(tilemap.getMapWidth() / 2.0f) + 0.5f * rect.w,
							 static_cast<float>(tilemap.getMapHeight() / 2.0f) - 0.5f * rect.h);
		Physics::createStaticBody(Vector2(rect.x, -1.f * rect.y) + offset ,
								  Vector2(rect.w, rect.h), fixture);
	}
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

// ============================================================================

void PhysicsDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	if (Renderer* renderer = Renderer::get())
	{
		renderer->drawPolygon((Vector2*)vertices, vertexCount, Color(color.r, color.g, color.b, color.a));
	}
}

void PhysicsDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	if (Renderer* renderer = Renderer::get())
	{
		renderer->drawPolygon((Vector2*)vertices, vertexCount, Color(color.r, color.g, color.b, color.a));
	}
}

void PhysicsDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) 
{
	if (Renderer* renderer = Renderer::get())
	{
		renderer->drawLineSegment(toVector2(p1), toVector2(p2), Color(color.r, color.g, color.b, color.a));
	}
}

// ============================================================================

void ContactListener::BeginContact(b2Contact* contact)
{
	if (Network::isServer())
	{
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

}

void ContactListener::EndContact(b2Contact* contact)
{
	if (Network::isServer())
	{
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
}

#endif // PHYSICS_BOX2D