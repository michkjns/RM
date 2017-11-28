

#include <physics/rigidbody.h>

#include <physics/physics_box2d.h>

#ifdef PHYSICS_BOX2D

extern b2World g_boxWorld;

extern Vector2 toVector2(const b2Vec2& a);
extern b2Vec2  tob2(const Vector2& a);

Rigidbody::Rigidbody()
{
	b2BodyDef dynamicBodyDef;
	dynamicBodyDef.position.Set(0.0f, 0.0f);
	dynamicBodyDef.type = b2_dynamicBody;

	b2Body* body = g_boxWorld.CreateBody(&dynamicBodyDef);
	
	m_impl = static_cast<RigidbodyImpl*>(body);
}

Rigidbody::~Rigidbody()
{
}

void Rigidbody::setPosition(const Vector2& position)
{
	m_impl->SetTransform(tob2(position), m_impl->GetAngle());
}

Vector2 Rigidbody::getPosition() const
{
	return toVector2(m_impl->GetPosition());
}

void Rigidbody::setAngle(float angle)
{
	m_impl->SetTransform(m_impl->GetPosition(), angle);
}

float Rigidbody::getAngle() const
{
	return m_impl->GetAngle();
}

void Rigidbody::setTransform(const Vector2& position, float angle)
{
	m_impl->SetTransform(tob2(position), angle);
}

void Rigidbody::setLinearVelocity(const Vector2& vel)
{
	m_impl->SetLinearVelocity(tob2(vel));
}

Vector2 Rigidbody::getLinearVelocity() const
{
	return toVector2(m_impl->GetLinearVelocity());
}

void Rigidbody::applyLinearImpulse(const Vector2& force, const Vector2& position)
{
	m_impl->ApplyLinearImpulse(tob2(force), tob2(position), true);
}

Vector2 Rigidbody::getWorldCenter() const
{
	return toVector2(m_impl->GetWorldCenter());
}

float Rigidbody::getMass() const
{
	return m_impl->GetMass();
}

RigidbodyImpl* Rigidbody::getImpl() const
{
	return m_impl;
}

#endif // PHYSICS_BOX2D
