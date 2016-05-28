
#include <game/rocket.h>

#include <bitstream.h>
#include <network.h>
#include <physics.h>

using namespace rm;

static float s_maxRocketLifetime = 5.0f;

Rocket::Rocket() :
	m_rigidbody(nullptr),
	m_owner(nullptr),
	m_lifetimeSeconds(0.f)
{
}

Rocket::Rocket(Vector2 direction, float accelerationPower) :
	m_rigidbody(nullptr),
	m_owner(nullptr),
	m_direction(direction),
	m_accelerationPower(accelerationPower),
	m_lifetimeSeconds(0.f)
{
	m_sprite = "demoTexture";
}

Rocket::~Rocket()
{
	if (m_rigidbody)
		Physics::destroyRigidbody(m_rigidbody);
}

void Rocket::initialize(Entity* owner, Vector2 direction, float power, bool shouldReplicate)
{
	m_owner             = owner;
	m_direction         = direction;
	m_accelerationPower = power;

	initialize(shouldReplicate);
}

void Rocket::initialize(bool shouldReplicate)
{
	Entity::initialize(shouldReplicate);

	if (m_rigidbody == nullptr)
	{
		physics::Fixture fixture;
		fixture.isSensor = true;
		m_rigidbody = Physics::createBoxRigidbody(Vector2(0.50f, 0.50f), fixture, this);
	}
	m_rigidbody->setPosition(getTransform().getWorldPosition());
	m_transform.setRigidbody(m_rigidbody);
}

void Rocket::update(float deltaTime)
{

}

void Rocket::fixedUpdate(float deltaTime)
{
	assert(m_isInitialized);
	m_rigidbody->setLinearVelocity(m_direction * m_accelerationPower);
	m_lifetimeSeconds += deltaTime;

	if (Network::isServer())
	{
		if (m_lifetimeSeconds >= s_maxRocketLifetime)
			kill();
	}
}

void Rocket::setDirection(Vector2 direction, float power)
{
	m_rigidbody->setLinearVelocity(direction * power);
	m_direction         = direction;
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

Rigidbody* Rocket::getRigidbody() const
{
	return m_rigidbody;
}

void Rocket::serializeFull(BitStream& stream)
{
	RocketFactory::RocketInitializer init;
	init.position       = m_transform.getWorldPosition();
	init.direction      = m_direction;
	init.networkID      = m_networkID;
	init.power          = m_accelerationPower;
	init.ownerNetworkID = m_owner->getNetworkID();

	stream.writeInt32(sizeof(init));
	stream.writeData(reinterpret_cast<char*>(&init), sizeof(init));
}

//void Rocket::deserializeFull(BitStream* stream)
//{
//
//}

void Rocket::startContact(Entity* other)
{
	//LOG_DEBUG("Rocket::startContact");
	if (isAlive())
	{
		if (other == m_owner) return;
		RocketExplode explosionEvent = { m_rigidbody->getPosition(), 0, 3.f, 5.f };

		// Physics::blastExplosion(explosionEvent.pos, explosionEvent.radius, explosionEvent.power);
		kill();
	}
}

void Rocket::endContact(Entity* other)
{
}

//==============================================================================

RocketFactory RocketFactory::s_factory;

Entity* RocketFactory::instantiateEntity(EntityInitializer* initializer, bool shouldReplicate, Entity* toReplace)
{
	if (Network::isClient() && toReplace == nullptr)
	{
		// TODO
		// client and server running
		// prevent duplicate function calls..
		return nullptr;
	}

	RocketInitializer* init = dynamic_cast<RocketInitializer*>(initializer);

	Rocket* rocket = (toReplace != nullptr) ? dynamic_cast<Rocket*>(toReplace) : new Rocket();

	Entity* owner = nullptr;
	for (const auto& entity : Entity::getList())
	{
		if (entity->getNetworkID() == init->ownerNetworkID)
		{
			owner = entity;
			break;
		}
	}
	rocket->setNetworkID(init->networkID);
	rocket->getTransform().setLocalPosition(Vector2(init->position[0],
													init->position[1]));
	rocket->initialize(owner, init->direction, init->power);

	return rocket;
}
