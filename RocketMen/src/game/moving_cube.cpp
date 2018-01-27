
#include "moving_cube.h"

#include <core/input.h>
#include <graphics/camera.h>
#include <graphics/renderer.h>
#include <physics/physics.h>

using namespace rm;

DEFINE_ENTITY_FACTORY(MovingCube);

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
	serializeCheck(stream, "begin_moving_cube_full");
	ensure(Entity::serializeFull(stream));
	if (!serialize(stream))
	{
		return ensure(false);
	}
	serializeCheck(stream, "end_moving_cube_full");
	return true;
}

template<typename Stream>
inline bool MovingCube::serialize(Stream& stream)
{
	serializeCheck(stream, "begin_moving_cube");
	if (!m_transform.serialize(stream))
	{
		return ensure(false);
	}

	serializeCheck(stream, "end_moving_cube");

	return true;
}
