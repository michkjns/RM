
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
	//if (Input::getKey(Key::A))
	//{
	//	vel.x = -50.0f  * deltaTime;
	//}

	//if (Input::getKey(Key::D))
	//{
	//	vel.x = 50.0f  * deltaTime;
	//}

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
	//	LOG_INFO("%f, %f", mp.x, mp.y);

	Renderer::get()->drawLineSegment(pos, mp, Color(1.f, 0.f, 0.f, 1.f));

}

void Character::serializeFull(BitStream& stream)
{
//	const Vector2 pos = m_transform.getWorldPosition();
//
//	stream->writeInt16(static_cast<int16_t>(getType()));
//	stream->writeFloat(pos.x);
//	stream->writeFloat(pos.y);

	CharacterFactory::CharacterInitializer init;
	init.sprite    = m_sprite;
	init.position  = m_transform.getWorldPosition();
	init.networkID = m_networkID;
	init.velocity  = m_rigidbody->getLinearVelocity();

	stream.writeInt32(sizeof(init));
	stream.writeData(reinterpret_cast<char*>(&init), sizeof(init));
}
//
//void Character::deserializeFull(BitStream* stream)
//{
//	Vector2 pos(stream->readFloat(), stream->readFloat());
//	m_transform.setLocalPosition(pos);
//}

void Character::Fire()
{
	const Vector2 mp        = Input::getMousePosition();
	const Vector2 mpWorld   = Camera::mainCamera->screenToWorld(mp);
	const Vector2 pos       = m_transform.getWorldPosition();
	const Vector2 direction = glm::normalize(mpWorld - 
	                                            m_transform.getWorldPosition());
	const float   power     = 20.f;

	Rocket* rocket = new Rocket();
	rocket->getTransform().setLocalPosition(m_transform.getWorldPosition());
	rocket->initialize(this, direction, power, true);
	LOG_DEBUG("Character::Fire()");
}

void Character::startContact(Entity* other)
{
	//LOG_DEBUG("Character::startContact");
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

Entity* CharacterFactory::instantiateEntity(EntityInitializer* initializer,
                                            bool shouldReplicate,
                                            Entity* toReplace)
{
	CharacterInitializer* init = dynamic_cast<CharacterInitializer*>(initializer);

	Character* character = toReplace ? 
		dynamic_cast<Character*>(toReplace) : new Character();

	character->setSprite(init->sprite);
	character->setNetworkID(init->networkID);
	character->initialize();
	character->getRigidbody()->setPosition(init->position);
	character->getRigidbody()->setPosition(init->velocity);

	return character;
}
