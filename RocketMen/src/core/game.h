
#pragma once

#include <cstdint>
#include "game_time.h"
class ActionBuffer;
class Game
{
public:
	virtual bool initialize() = 0;
	
	/** Fixed timestep update
	*	Use for game logic affecting physics
	* @param	float dt	the delta time in microseconds
	*/
	virtual void fixedUpdate(float dt) = 0;

	/** Normal update
	*	Use for everything that does not affect physics
	* @param	const Time& time	the game time
	*/
	virtual void update(const Time& time) = 0;

	virtual void terminate() = 0;

public:
	uint64_t getTimestep();
	void     setTimestep(uint64_t timestep);

	/** Processes the actionbuffer and performs action callbacks */
	void     processActions(ActionBuffer& actions);
	
private:
	uint64_t m_timestep;
};
