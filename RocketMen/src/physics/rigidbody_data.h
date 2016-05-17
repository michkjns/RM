
#pragma once

#define PHYSICS_BOX2D

#ifdef PHYSICS_BOX2D

#include <Box2D/Box2D.h>

/** Wrapper for Box2D */
class RigidbodyData : public b2Body
{
	virtual ~RigidbodyData() = delete;
};

#endif