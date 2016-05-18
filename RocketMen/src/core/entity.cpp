
#include <core/entity.h>

#include <physics.h>

#include <vector>

static std::vector<Entity*> s_entities;
static uint32_t s_entityID;

Entity::Entity() :
	m_id(0)
{
}

void Entity::initialize()
{
	m_id = ++s_entityID;
	s_entities.push_back(this);
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
}

void Entity::kill()
{
	m_id = 0;
}

bool Entity::isAlive() const
{
	return (m_id == 0);
}

Entity::~Entity()
{
}


Transform& Entity::getTransform() 
{
	return m_transform;
}
