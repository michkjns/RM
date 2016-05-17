
#include <entity.h>

#include <physics.h>

#include <vector>

static std::vector<Entity*> s_entities;
static uint32_t s_entityID;

Entity::Entity() :
	m_rigidbody(nullptr)
{
}

Entity::Entity(uint32_t id)
{
	m_id = id;
}

Entity* Entity::create()
{
	Entity* entity = new Entity(s_entityID++);
	s_entities.push_back(entity);
	return entity;
}

void Entity::flushEntities()
{
	for (auto it = s_entities.begin(); it != s_entities.end();)
	{
		if ((*it)->isDestroyed())
		{
			delete (*it);
			it = s_entities.erase(it);
		}
		else
			it++;
	}
}

void Entity::destroy()
{
	m_isDestroyed = true;
}

bool Entity::isDestroyed() const
{
	return m_isDestroyed;
}

Entity::~Entity()
{
}

Rigidbody* Entity::getRigidbody() const
{
	return m_rigidbody;
}

glm::vec2 Entity::getPosition() const
{
	return m_rigidbody->getPosition();
}

void Entity::setRigidbody(Rigidbody* rb)
{
	m_rigidbody = rb;
}
