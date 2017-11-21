
#pragma once

#include <game/game_time.h>

#include <cstdint>

#define DECLARE_GAME_INFO(GAME_NAME, GAME_VERSION) \
virtual const char* const getName()    const override { return GAME_NAME; } \
virtual const char* const getVersion() const override { return GAME_VERSION; }

class Game
{
public:
	/** Initialize 
	*	Use to initialize the game class
	*/
	virtual bool initialize();
	
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

	virtual void onPlayerJoin(int16_t playerId) = 0;
	virtual void onPlayerLeave(int16_t playerId) = 0;

public:
	virtual const char* const getName()    const;
	virtual const char* const getVersion() const;

	uint64_t getTimestep();
	void     setTimestep(uint64_t timestep);

	void processPlayerActions(class ActionBuffer& inputActions, int16_t playerId);
	
private:
	uint64_t m_timestep;
};
