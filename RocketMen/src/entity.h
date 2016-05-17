
#pragma once

#include <includes.h>

class Rigidbody;

class Entity
{
public:
	static Entity* create();
	static void    flushEntities();

public:
	void setRigidbody(Rigidbody* rigidbodyID);
	Rigidbody* getRigidbody() const;
	
	glm::vec2 getPosition() const;

	void destroy();
	bool isDestroyed() const;

private:
	Entity();
	Rigidbody* m_rigidbody;
	uint32_t   m_id;
	bool       m_isDestroyed;

protected:
	Entity(uint32_t id);
	~Entity();
};
