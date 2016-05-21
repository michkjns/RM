
#include <core/entity.h>

#include <physics.h>

static std::vector<Entity*> s_entities;
static std::vector<Entity*> s_newEntities;
static uint32_t s_entityID;

Entity::Entity() :
	m_id(0),
	m_isInitialized(false)
{
}

void Entity::initialize()
{
	m_id = ++s_entityID;
	s_newEntities.push_back(this);
	m_isInitialized = true;
}

void Entity::flushEntities()
{
	for (auto it = s_entities.begin(); it != s_entities.end();)
	{
		if ((*it)->isAlive())
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

void Entity::kill()
{
	m_id = 0;
}

bool Entity::isAlive() const
{
	return (m_id == 0);
}

std::string Entity::getSpriteName() const
{
	return m_sprite;
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
