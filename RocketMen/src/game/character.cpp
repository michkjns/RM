
#include <game/character.h>

#include <utility/bitstream.h>
#include <core/debug.h>
#include <core/entity_manager.h>
#include <core/input.h>
#include <core/transform2d.h>
#include <game/rocket.h>
#include <graphics/camera.h>
#include <network/network.h>
#include <physics/physics.h>

using namespace rm;

DECLARE_ENTITY_IMPL(Character);

Character::Character() :
	m_rigidbody(nullptr),
	m_actionListener(nullptr)
{
	m_rigidbody = Physics::createCharacterBody(Vector2(1.0f, 1.0f), this);
	m_rigidbody->setPosition(m_transform.getWorldPosition());
	m_transform.setRigidbody(m_rigidbody);
}

Character::~Character()
{
	Physics::destroyRigidbody(m_rigidbody);
	delete m_actionListener;
}

void Character::update(float /*deltaTime*/)
{
	if (Network::isClient())
	{
		const Vector2 screenMousePosition = input::getMousePosition();
		const Vector2 worldMousePosition = Camera::mainCamera->screenToWorld(screenMousePosition);
		m_aimDirection = glm::normalize(worldMousePosition - m_transform.getWorldPosition());
	}
}

void Character::fixedUpdate(float /*deltaTime*/)
{
}

void Character::debugDraw()
{
	if (Network::isLocalPlayer(getOwnerPlayerId()))
	{
		Vector2 mp = Camera::mainCamera->screenToWorld(input::getMousePosition());
		Vector2 pos = m_transform.getWorldPosition();

	//	Renderer::get()->drawLineSegment(pos, mp, Color(1.f, 0.f, 0.f, 1.f));
		Renderer::get()->drawLineSegment(pos, pos + m_aimDirection * 10.f, Color(1.f, 0.f, 0.f, 1.f));
	}
}

bool Character::Fire()
{
	const float power = 20.f;

	Rocket* rocket = new Rocket();
	const Vector2 pos = m_transform.getWorldPosition() + m_aimDirection * 0.20f;
	rocket->getTransform().setLocalPosition(pos);
	rocket->initialize(this, m_aimDirection, power);
	Entity::instantiate(rocket);

	const bool consumeAction = true;
	return consumeAction;
}

void Character::startContact(Entity* /*other*/)
{
}

void Character::endContact(Entity* /*other*/)
{
}

Rigidbody* Character::getRigidbody() const
{
	return m_rigidbody;
}

void Character::posessbyPlayer(int16_t playerId)
{
	assert(m_actionListener == nullptr);
	assert(playerId != INDEX_NONE);

	m_actionListener = new ActionListener(playerId);
	m_actionListener->registerAction("Fire", &Character::Fire, this);

	m_ownerPlayerId = playerId;
}

template<typename Stream>
bool Character::serializeFull(Stream& stream)
{
	Entity::serializeFull(stream);

	int32_t playerId = INDEX_NONE;
	if (Stream::isWriting)
	{
		playerId = m_actionListener ? static_cast<int32_t>(m_actionListener->getPlayerId()) : INDEX_NONE;
	}
	serializeInt(stream, playerId);

	if (Stream::isReading)
	{
		if (m_actionListener == nullptr && playerId != INDEX_NONE)
		{
			posessbyPlayer(static_cast<int16_t>(playerId));
		}
	}

	return ensure(serialize(stream));
}

template<typename Stream>
bool Character::serialize(Stream& stream)
{
	return ensure(m_transform.serialize(stream));
}

template<typename Stream>
bool Character::reverseSerialize(Stream& stream)
{
	if (!serializeVector2(stream, m_aimDirection))
		return ensure(false);

//	LOG_DEBUG("serialized aimDirection %f,%f", m_aimDirection.x, m_aimDirection.y);

	return true;
}