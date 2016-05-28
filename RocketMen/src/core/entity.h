
#pragma once

#include <includes.h>
#include <core/transform.h>

#include <functional>
#include <vector>

class Rigidbody;
class BitStream;

// TODO 
// Move this somewhere ?
enum class EntityType : int16_t
{
	Entity = 0,
	Character,
	Rocket
};

struct EntityInitializer
{
	virtual ~EntityInitializer() {};
	EntityType type      = EntityType::Entity;
	int32_t    networkID = -1;
};

class Entity
{
public:
	static Entity* instantiate(EntityInitializer* initializer, 
	                           bool shouldReplicate = false,
	                           Entity* toReplace = nullptr);

	static void flushEntities();
	static void killEntities();
	static std::vector<Entity*>& getList();

public:
	Entity();
	virtual ~Entity();
	EntityType getType() const { return EntityType::Entity; }

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

	virtual void serializeFull(BitStream& stream)  = 0;
	//virtual void deserializeFull(BitStream* stream) = 0;

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

	virtual Entity* instantiateEntity(EntityInitializer* initializer,
	                                  bool shouldReplicate = false,
	                                  Entity* toReplace = nullptr) = 0;
	friend class Entity;
};
