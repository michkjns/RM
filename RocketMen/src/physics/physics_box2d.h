
#pragma once

#define PHYSICS_BOX2D

#ifdef PHYSICS_BOX2D

#include <Box2D/Box2D.h>

class RigidbodyImpl : public b2Body
{
	virtual ~RigidbodyImpl() = delete;
};

class StaticbodyData : public b2Body
{
	virtual ~StaticbodyData() = delete;
};

#endif