
#pragma once
#pragma warning(default:4061)
#pragma warning(default:4062)

#include <common.h>

enum class EntityType : int16_t
{
	Entity = 0,
	Character,
	Rocket,

	NUM_ENTITY_TYPES
};

#define ReturnStringEnumCase(name) \
	case name: return #name

static inline const char* entityTypeAsString(EntityType type)
{
	switch (type)
	{
		ReturnStringEnumCase(EntityType::Entity);
		ReturnStringEnumCase(EntityType::Character);
		ReturnStringEnumCase(EntityType::Rocket);
		case EntityType::NUM_ENTITY_TYPES:
		{
			return "Invalid EntityType";
		}
	}

	assert(false);
	return "Invalid EntityType";
}
#undef ReturnStringEnumCase
