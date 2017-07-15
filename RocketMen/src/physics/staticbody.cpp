
#include <physics/staticbody.h>

#include <physics/physics_box2d.h>


#ifdef PHYSICS_BOX2D

//using namespace physics;
extern b2World g_boxWorld;

extern Vector2 toVector2(const b2Vec2& a);
extern b2Vec2  tob2(const Vector2& a);


Staticbody::Staticbody(const Vector2& position)
{
	b2BodyDef staticBodyDef;
	staticBodyDef.position.Set(position.x, position.y);
	staticBodyDef.type = b2_staticBody;

	b2Body* body = g_boxWorld.CreateBody(&staticBodyDef);

	m_impl = static_cast<StaticbodyData*>(body);
}

Staticbody::~Staticbody()
{
}

StaticbodyData* Staticbody::getImpl() const
{
	return m_impl;
}

#endif