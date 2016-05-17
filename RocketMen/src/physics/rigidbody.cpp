
#include <physics/rigidbody.h>

#include <physics/rigidbody_data.h>

#ifdef PHYSICS_BOX2D

extern b2World g_world;

inline glm::vec2 toglm(const b2Vec2& a) {
	return glm::vec2(a.x, a.y);
}

inline b2Vec2 tob2(const glm::vec2& a) {
	return b2Vec2(a.x, a.y);
}

Rigidbody::Rigidbody()
{
	b2BodyDef dynamicBodyDef;
	dynamicBodyDef.position.Set(0.0f, 10.0f);
	dynamicBodyDef.type = b2_dynamicBody;

	b2Body* body = g_world.CreateBody(&dynamicBodyDef);

	b2PolygonShape dynamicBox;
	dynamicBox.SetAsBox(1.0f, 1.0f);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &dynamicBox;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.3f;

	body->CreateFixture(&fixtureDef);


	m_body = static_cast<RigidbodyData*>(body);
}

Rigidbody::~Rigidbody()
{
}

void Rigidbody::setPosition(const glm::vec2& position)
{
	m_body->SetTransform(tob2(position), m_body->GetAngle());
}

glm::vec2 Rigidbody::getPosition() const
{
	return toglm(m_body->GetPosition());
}

void Rigidbody::setAngle(float angle)
{
	m_body->SetTransform(m_body->GetPosition(), angle);
}

float Rigidbody::getAngle() const
{
	return m_body->GetAngle();
}

void Rigidbody::setTransform(const glm::vec2& position, float angle)
{
	m_body->SetTransform(tob2(position), angle);
}

void Rigidbody::setLinearVelocity(glm::vec2& vel)
{
	m_body->SetLinearVelocity(tob2(vel));
}

glm::vec2 Rigidbody::getLinearVelocity() const
{
	return toglm(m_body->GetLinearVelocity());
}

RigidbodyData* Rigidbody::getData() const
{
	return m_body;
}

#endif // PHYSICS_BOX2D