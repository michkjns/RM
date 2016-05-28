
#include <core/entity.h>

#include <bitstream.h>
#include <network.h>

#include <physics.h>
#include <map>
#include <functional>

static std::map<EntityType, EntityFactory*> s_factoryMap;
static std::vector<Entity*> s_entities;
static std::vector<Entity*> s_newEntities;
static uint32_t s_entityID;

static const uint32_t s_maxSpawnPredictedEntities = 8;
static uint32_t s_nextTempID = 1;
//static bool m_tempNetworkID[s_maxSpawnPredictedEntities];

Entity::Entity() :
	m_id(0),
	m_isInitialized(false),
	m_networkID(-1)
{
}

void Entity::initialize(bool replicate)
{
	bool newEntity = true;
	if (!s_entities.empty())
	{
		for (const auto& ent : s_entities) // If this is a re-initialization,
		{                                  // Prevent duplicates..
			if (ent == this)
				newEntity = false;
		}
	} 
	if (newEntity)
	{
		m_id = ++s_entityID;
		s_newEntities.push_back(this);
	}
	m_isInitialized = true;

	if (m_networkID >= 0)
		return;

	if (replicate)
	{
		if (Network::isServer())
		{
			Network::generateNetworkID(this);
		}
		else
		{
			m_networkID = s_nextTempID++ * -1;
			if (s_nextTempID >= s_maxSpawnPredictedEntities)
				s_nextTempID = 1;

			Network::requestEntity(this);
		}
	}
}

Entity* Entity::instantiate(EntityInitializer* initializer, bool shouldReplicate,
                            Entity* toReplace)
{
	return s_factoryMap.at(initializer->type)->instantiateEntity(initializer,
	                                                            shouldReplicate,
	                                                            toReplace);
}

void Entity::flushEntities()
{
	for (auto it = s_entities.begin(); it != s_entities.end();)
	{
		if ((*it)->isAlive() == false)
		{
			delete (*it);
			it = s_entities.erase(it);
		}
		else
			it++;
	}

	for (auto it : s_newEntities)
	{
		s_entities.push_back(it);
	}
	s_newEntities.clear();
}

void Entity::killEntities()
{
	for (auto it = s_entities.begin(); it != s_entities.end();)
	{
		delete (*it);
		it = s_entities.erase(it);
	}
	
	// or ?

//	for (auto it = s_entities.begin(); it != s_entities.end();)
//	{
//		(*it)->kill();
//		it++;
//	}
}

std::vector<Entity*>& Entity::getList()
{
	return s_entities;
}

//==============================================================================

void Entity::kill()
{
	m_id = 0;
}

bool Entity::isAlive() const
{
	return (m_id != 0);
}

std::string Entity::getSpriteName() const
{
	return m_sprite;
}

void Entity::setNetworkID(int32_t networkID)
{
	m_networkID = networkID;
}

int32_t Entity::getNetworkID() const
{
	return m_networkID;
}

void Entity::serializeFull(BitStream& stream)
{
}

//void Entity::deserializeFull(BitStream* stream)
//{
//}

void Entity::startContact(Entity* other)
{
	LOG_DEBUG("Entity::startContact");
}

void Entity::endContact(Entity* other)
{
}

Entity::~Entity()
{
}

void Entity::setSprite(std::string name)
{
	m_sprite = name;
}

Transform& Entity::getTransform()
{
	return m_transform;
}

//==============================================================================

EntityFactory::EntityFactory()
{
}

void EntityFactory::registerFactory(EntityType type, EntityFactory* factory)
{
	s_factoryMap.insert(std::make_pair(type, factory));
}
