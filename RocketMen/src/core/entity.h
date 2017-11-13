
#pragma once

#include <bitstream.h>
#include <common.h>
#include <core/entity_type.h>
#include <core/transform.h>

#include <functional>

#define DECLARE_ENTITY(x) \
	static const EntityType s_type = x; \
	static EntityType getTypeStatic() { return s_type; }; \
	virtual EntityType getType() override { return s_type; };

static const int32_t  s_maxSpawnPredictedEntities = 16;
static const uint32_t s_maxNetworkedEntities = 256;

//==============================================================================

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

	/* @return true if entity is spawned locally and not yet acknowledged by the server*/
	bool isSpawnPrediction() const;

	virtual void startContact(Entity* other);
	virtual void endContact(Entity* other);

#ifdef _DEBUG
	uint32_t getId() const;
#endif // _DEBUG

protected:
	Transform   m_transform;
	std::string m_sprite;
	uint32_t    m_id;
	int32_t     m_networkId;

public:
	friend class EntityManager;
};
