
#pragma once

#include <includes.h>
#include <core/transform.h>

class Rigidbody;

class Entity
{
public:
	static void  flushEntities();

public:
	Entity();
	~Entity();

	virtual void initialize();
	virtual void update(float deltaTime) {};
	virtual void fixedUpdate() {};
	
	Transform& getTransform();

	void kill();
	bool isAlive() const;

protected:
	Transform m_transform;
	int32_t   m_sprite;
	uint32_t  m_id;
};
