
#pragma once

#include <bitstream.h>
#include <common.h>
#include <core/entity_type.h>
#include <core/transform.h>

#include <functional>
#include <vector>

#define DEFINE_ENTITY_TYPE(x) \
	static const EntityType s_type = x; \
	static EntityType getTypeStatic() { return s_type; }; \
	virtual EntityType getType() override { return s_type; };


static const int32_t  s_maxSpawnPredictedEntities = 8;
static const uint32_t s_maxNetworkedEntities = 256;
static const int32_t s_numEntityTypes = 
	static_cast<int32_t>(EntityType::NUM_ENTITY_TYPES);

//==============================================================================

class Entity
{
public:
	static const EntityType s_type = EntityType::Entity;

	static Entity* instantiate(ReadStream& stream, 
		                       bool shouldReplicate = false);
	static bool serializeFull(Entity* entity, ReadStream& stream, 
                              bool includeType = true);

	static bool serializeFull(Entity* entity, WriteStream& stream,
                              bool includeType = true);

	static bool serialize(Entity* entity, WriteStream& stream);
	static bool serialize(Entity* entity, ReadStream& stream);

	static void flushEntities();
	static void killEntities();
	static void registerFactory(class IEntityFactory* factory);
	static EntityType getTypeStatic() { return s_type; };
	static std::vector<Entity*>& getList();

public:
	Entity();
	virtual ~Entity();
	virtual EntityType getType() { return s_type; }

	void initialize(bool shouldReplicate = false);
	virtual void update(float deltaTime) {};
	virtual void fixedUpdate(float deltaTime) {};
	virtual void debugDraw() {};
	
	void setSprite(std::string name);

	Transform& getTransform();

	void kill();
	bool isAlive() const;
	std::string getSpriteName() const;
	
	void setNetworkID(int32_t networkID);
	int32_t getNetworkID() const;

	virtual void startContact(Entity* other);
	virtual void endContact(Entity* other);

	DEBUG_ONLY(
		uint32_t getID() const;
	)

protected:
	bool        m_isInitialized;
	Transform   m_transform;
	std::string m_sprite;
	uint32_t    m_id;
	int32_t     m_networkID;
};
