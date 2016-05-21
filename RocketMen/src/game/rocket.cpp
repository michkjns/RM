
#include <game/rocket.h>

#include <physics.h>

Rocket::Rocket()
{
}

Rocket::Rocket(Vector2 direction, float accelerationPower) :
	m_direction(direction),
	m_accelerationPower(accelerationPower)
{
	m_sprite = "demoTexture";
}

Rocket::~Rocket()
{
	if (m_rigidbody)
		Physics::destroyRigidbody(m_rigidbody);
}

void Rocket::initialize()
{
	Entity::initialize();

	physics::Fixture fixture;
	fixture.isSensor = true;
	m_rigidbody = Physics::createBoxRigidbody(Vector2(0.50f, 0.50f), fixture, this);
	
	m_transform.setRigidbody(m_rigidbody);
}

void Rocket::update(float deltaTime)
{

}

void Rocket::fixedUpdate(float deltaTime)
{
	assert(m_isInitialized);
	m_rigidbody->setLinearVelocity(m_direction * m_accelerationPower);
}

void Rocket::setDirection(Vector2 direction, float power)
{
	m_rigidbody->setLinearVelocity(direction * power);
	m_direction = direction;
	m_accelerationPower = power;
}

//Vector2 Rocket::getDirection() const
//{
//	return m_direction;
//}

void Rocket::setAccelerationPower(float accelerationPower)
{
	m_accelerationPower = accelerationPower;
}

float Rocket::getAccelerationPower() const
{
	return m_accelerationPower;
}

void Rocket::startContact(Entity* other)
{
	//LOG_DEBUG("Rocket::startContact");
	Physics::blastExplosion(m_rigidbody->getPosition(), 3.0f, 30.0f);
	kill();
}

void Rocket::endContact(Entity* other)
{
}
