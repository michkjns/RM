
#pragma once

#include <includes.h>
#include <bitstream.h>
#include <core/transform.h>

#include <functional>
#include <vector>

class Rigidbody;

static const int32_t  s_maxSpawnPredictedEntities = 8;
static const uint32_t s_maxNetworkedEntities = 256;

//==============================================================================

enum class EntityType : int16_t
{
	Entity = 0,
	Character,
	Rocket,

	COUNT
};

class Entity
{
public:
	static Entity* instantiate(ReadStream& stream, 
                               bool shouldReplicate = false);
	static bool    serializeFull(Entity* entity, ReadStream& stream, 
                                 bool skipType = false);
	static bool    serializeFull(Entity* entity, WriteStream& stream,
                                 bool skipType = false);
	static bool    serialize(Entity* entity, WriteStream& stream);
	static bool    serialize(Entity* entity, ReadStream& stream);

	static void flushEntities();
	static void killEntities();
	static std::vector<Entity*>& getList();

public:
	Entity();
	virtual ~Entity();
	virtual EntityType getType() const { return EntityType::Entity; }

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

protected:
	bool        m_isInitialized;
	Transform   m_transform;
	std::string m_sprite;
	uint32_t    m_id;
	int32_t     m_networkID;
};

//==============================================================================

class EntityFactory
{
protected:
	EntityFactory();
	static EntityType getType() { return EntityType::Character; }
	static void registerFactory(EntityType type, EntityFactory* factory);

	virtual Entity* instantiateEntity(ReadStream& stream,
	                                  bool shouldReplicate = false) = 0;

	virtual bool   serializeFull(Entity* entity, WriteStream& ws) = 0;
	virtual bool   serializeFull(Entity* entity, ReadStream& rs)  = 0;
	virtual bool   serialize(Entity* entity, WriteStream& ws) = 0;
	virtual bool   serialize(Entity* entity, ReadStream& rs) = 0;
	friend class Entity;
};
