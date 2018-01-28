
#include <game/rocket.h>

#include <utility/bitstream.h>
#include <core/debug.h>
#include <core/entity_manager.h>
#include <network/network.h>
#include <physics/physics.h>
#include <utility/utility.h>

using namespace rm;

DEFINE_ENTITY_FACTORY(Rocket);

static float s_maxRocketLifetime = 5.0f;

Rocket::Rocket() :
	m_isInitialized(false),
	m_owner(nullptr),
	m_lifetimeSeconds(0.f),
	m_gracePeriod(true)
{
	physics::Fixture fixture;
	fixture.isSensor = true;
	m_rigidbody = Physics::createBoxRigidbody(Vector2(0.50f, 0.50f), fixture, this);
	m_transform.setRigidbody(m_rigidbody);
	m_spriteName = "demoTexture";
	m_transform.setScale(Vector2(0.5f));
}

Rocket::~Rocket()
{
	Physics::destroyRigidbody(m_rigidbody);
}

void Rocket::initialize(Entity* owner, const Vector2& direction, float power)
{
	m_owner             = owner;
	m_accelerationPower = power;
	m_direction         = glm::normalize(direction);

	m_rigidbody->setLinearVelocity(m_accelerationPower * m_direction);
	m_isInitialized = true;
}

void Rocket::update(float /*deltaTime*/)
{

}

void Rocket::fixedUpdate(float deltaTime)
{
	assert(m_isInitialized);
	m_rigidbody->setLinearVelocity(m_direction * m_accelerationPower);
	m_lifetimeSeconds += deltaTime;

	if (Network::isServer())
	{
		if (m_gracePeriod && m_lifetimeSeconds >= 1.5f)
		{
			m_gracePeriod = false;
		}

		if (m_lifetimeSeconds >= s_maxRocketLifetime) 
		{
			kill();
		}
	}
}

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

void Rocket::startContact(Entity* other)
{
	if (m_gracePeriod && other == m_owner)
		return;

	if (isAlive())
	{
		if (Network::isServer())
		{
			Physics::blastExplosion(m_rigidbody->getPosition(), 3.f, 200.f);
			kill();
		}
	}
}

void Rocket::endContact(Entity* /*other*/)
{
}

template<typename Stream>
bool Rocket::serializeFull(Stream& stream)
{
	ensure(Entity::serializeFull(stream));

	int32_t ownerId = INDEX_NONE;
	if (Stream::isWriting)
	{
		assert(m_owner != nullptr);
		ownerId = m_owner->getNetworkId();
	}

	serializeInt(stream, ownerId, 0, s_maxNetworkedEntities);
	if (Stream::isReading)
	{
		if (ownerId >= s_maxNetworkedEntities || ownerId < 0)
			return ensure(false);

		auto& entityList = EntityManager::getEntities();
		m_owner = findPtrByPredicate(entityList.begin(), entityList.end(),
			[ownerId](Entity* entity) -> bool { return entity->getNetworkId() == ownerId; });
	}
	
	if (!serializeFloat(stream, m_accelerationPower))
	{
		return ensure(false);
	}

	if (!serialize(stream))
		return ensure(false);

	if (Stream::isReading)
	{
		initialize(m_owner, glm::normalize(m_rigidbody->getLinearVelocity()), m_accelerationPower);
	}

	return true;
}

template<typename Stream>
bool Rocket::serialize(Stream& stream)
{	
	if (!m_transform.serialize(stream))
		return ensure(false);

	return true;
}
