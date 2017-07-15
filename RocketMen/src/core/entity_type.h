
#pragma once

#include <stdint.h>

enum class EntityType : int16_t
{
	Entity = 0,
	Character,
	Rocket,

	NUM_ENTITY_TYPES
};