
#pragma once

#include <core/state_machine.h>
#include <cstdint>

#define DECLARE_GAME_INFO(GAME_NAME, GAME_VERSION) \
virtual const char* const getName()    const override { return GAME_NAME; } \
virtual const char* const getVersion() const override { return GAME_VERSION; }

class Game
{
public:
	Game();
	virtual ~Game();

	virtual void initialize() = 0;
	virtual void update(const class Time& time);
	virtual void tick(float fixedDeltaTime);
	virtual void terminate();
	virtual void onPlayerJoin(int16_t playerId) = 0;
	virtual void onPlayerLeave(int16_t playerId) = 0;

public:
	void initialize(GameStateFactory* stateFactory, uint32_t initialStateId);
	virtual const char* const getName()    const;
	virtual const char* const getVersion() const;

	uint64_t getTimestep();
	void     setTimestep(uint64_t timestep);

	void processPlayerActions(class ActionBuffer& inputActions, int16_t playerId);
	GameState* pushState(uint32_t stateId);
	void popState();

protected:
	uint64_t m_timestep;
	StateMachine m_stateMachine;
	GameStateFactory* m_stateFactory;
};
