
#pragma once

#include <common.h>
#include <core/game_state.h>

class GameStateFactory
{
public:
	virtual GameState* getState(uint32_t stateType) const = 0;
};