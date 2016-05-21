
#pragma once

#include <includes.h>
#include <core/transform.h>

#include <vector>

class Rigidbody;

class Entity
{
public:
	static void flushEntities();
	static void killEntities();
	static std::vector<Entity*>& getList();

public:
	Entity();
	virtual ~Entity();

	virtual void initialize();
	virtual void update(float deltaTime) {};
	virtual void fixedUpdate(float deltaTime) {};
	
	void setSprite(std::string name);

	Transform& getTransform();

	void kill();
	bool isAlive() const;
	std::string getSpriteName() const;

	virtual void startContact(Entity* other);
	virtual void endContact(Entity* other);

protected:
	bool        m_isInitialized;
	Transform   m_transform;
	std::string m_sprite;
	uint32_t    m_id;
};
