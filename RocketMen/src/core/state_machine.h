
#pragma once
#include <cstdint>

class GameState;
class GameStateFactory;

class StateMachine
{
public:
	StateMachine();
	~StateMachine();

	GameState* getState() const;
	void push(GameState* state);
	GameState* pop();

private:
	GameState* m_currentState;
};