
#pragma once
#include <cstdint>

class GameState;
class GameStateFactory;

class StateMachine
{
public:
	StateMachine(uint32_t maxDepth);
	~StateMachine();

	GameState* getCurrentState() const;
	GameState* push(GameStateFactory* factory, uint32_t stateId);
	void pop();

private:
	GameState** m_stack;
	int32_t     m_currentIndex;
	uint32_t    m_maxDepth;
};