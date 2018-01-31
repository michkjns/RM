
#include <game/character.h>

#include <utility/bitstream.h>
#include <core/debug.h>
#include <core/entity_manager.h>
#include <core/game.h>
#include <core/input.h>
#include <core/transform2d.h>
#include <game/rocket.h>
#include <graphics/camera.h>
#include <network/network.h>
#include <physics/physics.h>

using namespace rm;

DEFINE_ENTITY_FACTORY(Character);

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
		Game* game = getGame();
		ASSERT(game != nullptr);
		
		Camera* camera = game->getMainCamera();
		ASSERT(camera != nullptr);

		const Vector2 screenMousePosition = input::getMousePosition();
		const Vector2 worldMousePosition = camera->screenToWorld(screenMousePosition);
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
		Game* game = getGame();
		ASSERT(game != nullptr);
		
		Camera* camera = game->getMainCamera();
		ASSERT(camera != nullptr);

		const Vector2 position = m_transform.getWorldPosition();
		const float aimLineLength = 10.f;
		const LineSegment aimLine = { position, position + m_aimDirection * aimLineLength, RenderSpace::WorldSpace};
		Renderer::get()->drawLineSegment(aimLine, Color(1.f, 0.f, 0.f, 1.f));
	}
}

bool Character::Fire()
{
	const float power = 20.f;

	if (Network::isServer())
	{
		Rocket* rocket = new Rocket();
		const Vector2 pos = m_transform.getWorldPosition() + m_aimDirection * 0.20f;
		rocket->getTransform().setLocalPosition(pos);
		rocket->initialize(this, m_aimDirection, power);
		Entity::instantiate(rocket);
	}
	const bool consumeAction = false;
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
	ASSERT(m_actionListener == nullptr);
	ASSERT(playerId != INDEX_NONE);

	m_actionListener = new ActionListener(playerId);
	m_actionListener->registerAction("Fire", &Character::Fire, this);

	m_ownerPlayerId = playerId;
}

template<typename Stream>
bool Character::serializeFull(Stream& stream)
{
	serializeCheck(stream, "begin_character_full");

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

	if (!serialize(stream))
	{
		return false;
	}
	
	serializeCheck(stream, "end_character_full");

	return true;
}

template<typename Stream>
bool Character::serialize(Stream& stream)
{
	serializeCheck(stream, "begin_character");

	if (!m_transform.serialize(stream))
	{
		return false;
	}

	serializeCheck(stream, "end_character");
	return true;
}
