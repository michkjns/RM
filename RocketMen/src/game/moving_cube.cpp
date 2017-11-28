
#include "moving_cube.h"

#include <core/input.h>
#include <graphics/camera.h>
#include <graphics/renderer.h>
#include <physics/physics.h>

using namespace rm;

DECLARE_ENTITY_IMPL(MovingCube);

MovingCube::MovingCube() :
	m_speed(2.0f),
	m_direction(1.f)
{
	m_sprite = "demoTexture";
	m_transform.setLocalPosition(Vector2(0.f, 3.f));

}

MovingCube::~MovingCube()
{
	
}

void MovingCube::update(float /*deltaTime*/)
{

}

void MovingCube::fixedUpdate(float deltaTime)
{
	Vector2 position = m_transform.getLocalPosition();
	position += Vector2(m_speed * m_direction, 0.f) * deltaTime;

	if (position.x < -5.f)
	{
		m_direction = 1.f;
	}
	else if (position.x > 5.f)
	{
		m_direction = -1.f;
	}
	m_transform.setLocalPosition(position);
}

void MovingCube::debugDraw()
{
}

void MovingCube::startContact(Entity* /*other*/)
{

}

void MovingCube::endContact(Entity* /*other*/)
{

}

template<typename Stream>
inline bool MovingCube::serializeFull(Stream& stream)
{
	ensure(Entity::serializeFull(stream));
	return ensure(serialize(stream));
}

template<typename Stream>
inline bool MovingCube::serialize(Stream& stream)
{
	return ensure(m_transform.serialize(stream));
}

template<typename Stream>
inline bool MovingCube::reverseSerialize(Stream& /*stream*/)
{
	return true;
}