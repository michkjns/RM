
#pragma once

#include <utility/bitstream.h>
#include <common.h>
#include <core/entity_type.h>
#include <core/transform.h>

#include <functional>

#define DECLARE_ENTITY(x) \
	static const EntityType s_type = x; \
	static EntityType getTypeStatic() { return s_type; }; \
	virtual EntityType getType() override { return s_type; };

//=============================================================================

class Entity
{
public:
	static const EntityType s_type = EntityType::Entity;
	static EntityType getTypeStatic() { return s_type; };

	static void instantiate(Entity* entity);

public:
	Entity();
	virtual ~Entity();
	virtual EntityType getType() { return s_type; }

	virtual void update(float deltaTime) = 0;
	virtual void fixedUpdate(float deltaTime) = 0;
	virtual void debugDraw() {};
	
	void setSprite(std::string name);

	Transform& getTransform();

	/* Flags entity to be destroyed */
	void kill();

	/* @return false if entity is flagged to be destroyed */
	bool isAlive() const;

	std::string getSpriteName() const;
	
	void setNetworkId(int32_t networkId);
	int32_t getNetworkId() const;

	int16_t getOwnerPlayerId() const;

	/* @return true if entity is replicated between server and client*/
	bool isReplicated() const;

	/* @return true if entity is spawned locally and not yet acknowledged by the server*/
	bool isSpawnPrediction() const;

	virtual void startContact(Entity* other);
	virtual void endContact(Entity* other);

	int32_t getId() const;

protected:
	Transform   m_transform;
	std::string m_sprite;
	int32_t     m_networkId;
	int16_t     m_ownerPlayerId;

private:
	int32_t     m_id;

public:
	friend class EntityManager;
};
