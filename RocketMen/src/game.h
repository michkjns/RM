
#pragma once

#include <stdint.h>
#include "time.h"

class Game
{
public:
	virtual bool initialize() = 0;
	
	/** Fixed timestep update
	*	Use for game logic affecting physics
	* @param	uint64_t timestep	the timestep in microseconds
	*/
	virtual void fixedUpdate(uint64_t timestep) = 0;

	/** Normal update
	*	Use for everything that does not affect physics
	* @param	const Time& time	the game time
	*/
	virtual void update(const Time& time) = 0;

	virtual void terminate() = 0;
};
