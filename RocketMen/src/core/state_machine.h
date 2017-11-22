
#pragma once

#include <common.h>

class GameState;
class GameStateFactory;

class StateMachine
{
public:
	StateMachine();
	~StateMachine();

	GameState* getState() const;
	
	template<typename T>
	void push(GameState* state, T* owner);

	template<typename T>
	void pop(T* owner);

private:
	GameState* m_currentState;
};

template<typename T>
void StateMachine::push(GameState* state, T* owner)
{
	assert(state != nullptr);
	state->m_previousState = m_currentState;
	m_currentState = state;

	m_currentState->initialize(owner);
	m_currentState->enter(owner);
}

template<typename T>
void StateMachine::pop(T* owner)
{
	GameState* poppedState = m_currentState;
	m_currentState = m_currentState->m_previousState;
	if (m_currentState != nullptr)
	{
		m_currentState->enter(owner);
	}

	poppedState->destroy(owner);
	delete poppedState;
}
