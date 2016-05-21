
#include <game/character.h>

#include <graphics/camera.h>
#include <core/input.h>
#include <core/transform.h>
#include <game/rocket.h>
#include <physics.h>

using namespace rm;

Character::Character() :
	m_rigidbody(nullptr)
{
}

Character::~Character()
{
	if(m_rigidbody)
		Physics::destroyRigidbody(m_rigidbody);
}

void Character::initialize()
{
	Entity::initialize();

	m_rigidbody = Physics::createCharacterBody(Vector2(1.0f, 1.0f), this);
	m_transform.setRigidbody(m_rigidbody);
}

void Character::update(float deltaTime)
{
	Vector2 mp = Camera::mainCamera->screenToWorld(Input::getMousePosition());
	Vector2 pos = m_transform.getWorldPosition();
//	LOG_INFO("%f, %f", mp.x, mp.y);

	if (Input::getMouseDown(input::MouseButton::MOUSE_LEFT))
	{
		Vector2 direction = glm::normalize(mp - m_transform.getWorldPosition());
		float power = 10.f;
		Rocket* rocket = new Rocket();
		rocket->initialize();
		rocket->setDirection(direction, power);
		rocket->getTransform().setLocalPosition(m_transform.getWorldPosition()
												+ direction * 2.0f);
	}
}

void Character::fixedUpdate(float deltaTime)
{
	//assert(m_rigidbody);
	//if (m_transform.isDirty())
	//{
	//	m_rigidbody->setTransform(m_transform.getWorldPosition(), m_transform.getWorldRotation());
	//}
	//else
	{
		//m_transform.setLocalPosition(m_rigidbody->getPosition());
		//m_transform.setLocalRotation(m_rigidbody->getAngle());
	}
}

void rm::Character::startContact(Entity* other)
{
	//LOG_DEBUG("Character::startContact");
}

void rm::Character::endContact(Entity* other)
{
}

Rigidbody* Character::getRigidbody() const
{
	return m_rigidbody;
}
