
#include <core/entity_factory.h>

#include <bitstream.h>
#include <network.h>

#include <physics.h>
#include <map>
#include <functional>

static std::map<EntityType, IEntityFactory*> s_factoryMap;

static std::vector<Entity*> s_entities;
static std::vector<Entity*> s_newEntities;
static uint32_t s_entityID;

static uint32_t s_nextTempID = 1;

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
			if (s_nextTempID > s_maxSpawnPredictedEntities)
				s_nextTempID = 1;

			if (!Network::requestEntity(this))
			{
				LOG_DEBUG("Couldn't Request Entity!");
				kill();
			}
		}
	}
}

Entity* Entity::instantiate(ReadStream& stream, bool shouldReplicate)
{
	EntityType type = EntityType::Entity;
	int32_t intType = 0;
	stream.serializeInt(intType, 0, (int32_t)EntityType::COUNT);
	type = static_cast<EntityType>(intType);
	return s_factoryMap.at(type)->instantiate(stream, shouldReplicate);
}

bool Entity::serializeFull(Entity* entity, ReadStream& stream, bool skipType)
{
	if (!skipType)
	{
		int32_t intType = static_cast<int32_t>(entity->getType());
		stream.serializeInt(intType, 0, (int32_t)EntityType::COUNT);
	}
	return s_factoryMap.at(entity->getType())->serializeFull(entity, stream);
}

bool Entity::serializeFull(Entity* entity, WriteStream& stream, bool skipType)
{
	if (!skipType)
	{
		int32_t intType = static_cast<int32_t>(entity->getType());
		stream.serializeInt(intType, 0, (int32_t)EntityType::COUNT);
	}
	return s_factoryMap.at(entity->getType())->serializeFull(entity, stream);
}

bool Entity::serialize(Entity* entity, WriteStream& stream)
{
	return s_factoryMap.at(entity->getType())->serialize(entity, stream);
}

bool Entity::serialize(Entity* entity, ReadStream& stream)
{
	return s_factoryMap.at(entity->getType())->serialize(entity, stream);;
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
}

std::vector<Entity*>& Entity::getList()
{
	return s_entities;
}

void Entity::registerFactory(IEntityFactory* factory)
{
	assert(factory);
	s_factoryMap[factory->getType()] = factory;
}

//==============================================================================

void Entity::kill()
{
	m_id = 0;
	if (Network::isServer())
	{
		Network::destroyEntity(m_networkID);
	}
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

//EntityFactory::EntityFactory()
//{
//}
//
//void EntityFactory::registerFactory(EntityType type, EntityFactory* factory)
//{
//	s_factoryMap.insert(std::make_pair(type, factory));
//}
