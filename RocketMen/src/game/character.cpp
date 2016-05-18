
#include <game/character.h>

#include <core/transform.h>
#include <physics.h>

Character::Character() :
	m_rigidbody(nullptr)
{
}

Character::~Character()
{
	if (m_rigidbody) delete m_rigidbody;
}

void Character::initialize()
{
	Entity::initialize();

	m_rigidbody = Physics::createRigidbody();
}

void Character::update(float deltaTime)
{

}

void Character::fixedUpdate()
{
	assert(m_rigidbody);
	if (m_transform.isDirty())
	{
		m_rigidbody->setTransform(m_transform.getWorldPosition(), m_transform.getWorldRotation());
	}
	else
	{
		m_transform.setLocalPosition(m_rigidbody->getPosition());
		m_transform.setLocalRotation(m_rigidbody->getAngle());
	}
}
