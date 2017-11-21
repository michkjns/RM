
#pragma once

class Game;
class Time;

class GameState
{
public:
	GameState() : m_previousState(nullptr) {};
	virtual ~GameState() {};

	virtual void initialize(Game* game)                 = 0;
	virtual void enter(Game* game)                      = 0;
	virtual void destroy(Game* game)                    = 0;
	virtual void update(Game* game, const Time& time)   = 0;
	virtual void tick(Game* game, float fixedDeltaTime) = 0;
	virtual void render(Game* game)                     = 0;

protected:
	GameState* m_previousState;

public:
	friend class StateMachine;
};