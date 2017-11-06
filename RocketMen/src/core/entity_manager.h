
#pragma once

#include <core/entity_type.h>

#include <vector>

class Entity;

class EntityManager
{
public:
	static class IEntityFactory* getFactory(EntityType type);
	static void registerFactory(class IEntityFactory* factory);

	static void instantiateEntity(Entity* entity, bool enableReplication = true);
	static Entity* instantiateEntity(class ReadStream& stream, bool enableReplication = true);

	static bool serializeFullEntity(Entity* entity, class ReadStream& stream,
		bool includeType = true);

	static bool serializeFullEntity(Entity* entity, class WriteStream& stream,
		bool includeType = true);

	static bool serializeEntity(Entity* entity, class WriteStream& stream);
	static bool serializeEntity(Entity* entity, class ReadStream& stream);

	static void flushEntities();
	static void killEntities();
	static std::vector<Entity*>& getEntities();
};
