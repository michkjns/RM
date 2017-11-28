
#pragma once
#pragma warning(default:4061)
#pragma warning(default:4062)

#include <common.h>

enum class EntityType : int16_t
{
	Entity = 0,
	Character,
	Rocket,
	MovingCube,

	NUM_ENTITY_TYPES
};

#define CASE_RETURN_STRING(name) \
	case name: return #name

static inline const char* entityTypeAsString(EntityType type)
{
	switch (type)
	{
		CASE_RETURN_STRING(EntityType::Entity);
		CASE_RETURN_STRING(EntityType::Character);
		CASE_RETURN_STRING(EntityType::Rocket);
		CASE_RETURN_STRING(EntityType::MovingCube);
		case EntityType::NUM_ENTITY_TYPES:
		{
			return "Invalid EntityType";
		}
	}

	assert(false);
	return "Invalid EntityType";
}
#undef CASE_RETURN_STRING
