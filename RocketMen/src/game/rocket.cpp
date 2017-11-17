
#include <game/rocket.h>

#include <bitstream.h>
#include <core/debug.h>
#include <core/entity_manager.h>
#include <network.h>
#include <physics.h>
#include <utility.h>

using namespace rm;

EntityFactory<Rocket> EntityFactory<Rocket>::s_factory;

static float s_maxRocketLifetime = 5.0f;

Rocket::Rocket() :
	m_isInitialized(false),
	m_rigidbody(nullptr),
	m_owner(nullptr),
	m_lifetimeSeconds(0.f),
	m_gracePeriod(true)
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
	{
		Physics::destroyRigidbody(m_rigidbody);
	}
}

void Rocket::initialize(Entity* owner, Vector2 direction, float power)
{
	m_owner             = owner;
	m_accelerationPower = power;
	m_direction         = glm::normalize(direction);

	if (m_rigidbody == nullptr)
	{
		setupRigidBody();
	}

	m_transform.setRigidbody(m_rigidbody);
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

template<typename Stream>
bool Rocket::serializeFull(Stream& stream)
{
	int32_t ownerId = -1;
	Entity* owner = nullptr;
	Vector2 vel;
	Vector2 pos;

	serializeInt(stream, m_networkId, -s_maxSpawnPredictedEntities, s_maxNetworkedEntities);
	if (Stream::isReading)
	{
		if (m_networkId < -s_maxSpawnPredictedEntities || std::abs(m_networkId) > s_maxNetworkedEntities)
		{
			LOG_WARNING("Rocket: Received invalid networkID");
			return false;
		}
	}
	if (Stream::isWriting)
	{
		ownerId = m_owner->getNetworkId();
		vel     = m_rigidbody->getLinearVelocity();
		pos     = m_transform.getLocalPosition();
	}

	serializeInt(stream, ownerId, 0, s_maxNetworkedEntities);
	if (Stream::isReading)
	{
		if (ownerId >= s_maxNetworkedEntities || ownerId < 0)
			return false;

		auto entityList = EntityManager::getEntities();
		owner = findPtrByPredicate(entityList.begin(), entityList.end(),
			[ownerId](Entity* entity) -> bool { return entity->getNetworkId() == ownerId; });
	}
	
	if (!serializeVector2(stream, vel, -100.0f, 100.0f, 0.01f))
	{
		return false;
	}

	if (!serializeFloat(stream, m_accelerationPower))
	{
		return false;
	}

	if (!m_isInitialized)
	{
		initialize(owner, glm::normalize(vel), m_accelerationPower);
	}

	if (!serializeVector2(stream, pos, -100.0f, 100.0f, 0.01f))
	{
		return false;
	}

	if (Stream::isReading)
	{
		m_transform.setLocalPosition(pos);
		m_rigidbody->setLinearVelocity(vel);
	}

	return true;
}

template<typename Stream>
bool Rocket::serialize(Stream& stream)
{
	float angle;
	Vector2 vel;
	Vector2 pos;

	if (Stream::isWriting)
	{
		pos = m_transform.getLocalPosition();
		vel = m_rigidbody->getLinearVelocity();
	}

	if (serializeFloat(stream, angle, 0.0f, 2.0f, 0.01f))
	{
		if (Stream::isReading)
		{
			m_transform.setLocalRotation(angle);
		}
	}
	else
	{
		return false;
	}

	serializeVector2(stream, pos);
	if (Stream::isReading)
	{
		m_transform.setLocalPosition(pos);
	}

	serializeVector2(stream, vel);
	if (Stream::isReading)
	{
		m_rigidbody->setLinearVelocity(vel);
	}

	return true;
}

template<typename Stream>
bool Rocket::reverseSerialize(Stream& /*stream*/)
{
	return true;
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

void Rocket::setupRigidBody()
{
	assert(m_rigidbody == nullptr);
	physics::Fixture fixture;
	fixture.isSensor = true;
	m_rigidbody = Physics::createBoxRigidbody(Vector2(0.50f, 0.50f), fixture, this);
}
