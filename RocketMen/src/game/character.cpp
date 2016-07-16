
#include <game/character.h>

#include <bitstream.h>
#include <core/input.h>
#include <core/transform.h>
#include <game/rocket.h>
#include <graphics/camera.h>
#include <network.h>
#include <physics.h>

using namespace rm;
using namespace input;

Character::Character() :
	m_rigidbody(nullptr)
{
}

Character::~Character()
{
	if(m_rigidbody)
		Physics::destroyRigidbody(m_rigidbody);

	if (m_actionListener)
		delete m_actionListener;
}

void Character::initialize(bool shouldReplicate)
{
	m_actionListener = new ActionListener();
	m_actionListener->registerAction("Fire", &Character::Fire, this);

	m_rigidbody = Physics::createCharacterBody(Vector2(1.0f, 1.0f), this);
	m_rigidbody->setPosition(m_transform.getWorldPosition());
	m_transform.setRigidbody(m_rigidbody);

	Entity::initialize(shouldReplicate);
}

void Character::update(float deltaTime)
{

}

void Character::fixedUpdate(float deltaTime)
{
	glm::vec2 vel = getRigidbody()->getLinearVelocity();

	if (Input::getKey(Key::SPACE))
	{
		vel.x = vel.y = 0.0f;
	}

	getRigidbody()->setLinearVelocity(vel);
}

void Character::debugDraw()
{
	Vector2 mp = Camera::mainCamera->screenToWorld(Input::getMousePosition());
	Vector2 pos = m_transform.getWorldPosition();

	Renderer::get()->drawLineSegment(pos, mp, Color(1.f, 0.f, 0.f, 1.f));

}

template<typename Stream>
bool Character::serializeFull(Stream& stream)
{
	serializeInt(stream, m_networkID, -32, 128);
	
	int32_t sprLength = 0;
	if (Stream::isWriting)
		sprLength = int32_t(m_sprite.length());
	serializeInt(stream, sprLength, 0, 32);
	
	if (Stream::isReading)
		m_sprite.resize(sprLength);
	for (int32_t i = 0; i < sprLength; i++)
	{
		int32_t character = int32_t(m_sprite[i]);
		serializeInt(stream, character, CHAR_MIN, CHAR_MAX);
		if (Stream::isReading)
			m_sprite[i] = char(character);
	}
	
	if(!m_isInitialized)
		initialize(false);

	float angle;
	if (Stream::isWriting)
		angle = m_transform.getLocalRotation();
	serializeFloat(stream, angle, 0.0f, 1.0f, 0.01f);
	if (Stream::isReading)
		m_transform.setLocalRotation(angle);

	Vector2 pos;
	if (Stream::isWriting)
		pos = m_transform.getLocalPosition();
	serializeVector2(stream, pos);
	if (Stream::isReading)
		m_transform.setLocalPosition(pos);

	Vector2 vel;
	if (Stream::isWriting)
		vel = m_rigidbody->getLinearVelocity();
	serializeVector2(stream, vel);
	if (Stream::isReading)
		m_rigidbody->setLinearVelocity(vel);

	return true;
}

template<typename Stream>
bool Character::serialize(Stream & stream)
{
	Vector2 pos;
	if (Stream::isWriting)
		pos = m_transform.getLocalPosition();
	serializeVector2(stream, pos);
	if (Stream::isReading)
		m_transform.setLocalPosition(pos);

	Vector2 vel;
	if (Stream::isWriting)
		vel = m_rigidbody->getLinearVelocity();
	serializeVector2(stream, vel);
	if (Stream::isReading)
		m_rigidbody->setLinearVelocity(vel);

	return true;
}

void Character::Fire()
{
	if (!Camera::mainCamera) 
		return;

	const Vector2 mp        = Input::getMousePosition();
	const Vector2 mpWorld   = Camera::mainCamera->screenToWorld(mp);
	const Vector2 direction = 
		glm::normalize(mpWorld - m_transform.getWorldPosition());
	const float   power     = 20.f;

	Rocket* rocket = new Rocket();
	const Vector2 pos = m_transform.getWorldPosition() + direction * 1.0f;
	rocket->getTransform().setLocalPosition(pos);
	rocket->initialize(this, direction, power, true);
	//LOG_DEBUG("Character::Fire() %f, %f", pos.x, pos.y);
}

void Character::startContact(Entity* other)
{
}

void Character::endContact(Entity* other)
{
}

Rigidbody* Character::getRigidbody() const
{
	return m_rigidbody;
}

//==============================================================================

CharacterFactory CharacterFactory::s_factory;

Entity* CharacterFactory::instantiateEntity(ReadStream& stream,
                                            bool shouldReplicate)
{
	Character* character = new Character();
	character->serializeFull(stream);

	return character;
}

bool CharacterFactory::serializeFull(Entity* entity, WriteStream& stream)
{
	return dynamic_cast<Character*>(entity)->serializeFull(stream);
}

bool CharacterFactory::serializeFull(Entity* entity, ReadStream& stream)
{
	return dynamic_cast<Character*>(entity)->serializeFull(stream);
}

bool CharacterFactory::serialize(Entity* entity, WriteStream& ws)
{
	return dynamic_cast<Character*>(entity)->serialize(ws);
}

bool CharacterFactory::serialize(Entity* entity, ReadStream& rs)
{
	return dynamic_cast<Character*>(entity)->serialize(rs);
}
