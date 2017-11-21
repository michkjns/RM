
#include <core/state_machine.h>
#include <core/game_state.h>
#include <common.h>

#include <algorithm>

StateMachine::StateMachine() :
	m_currentState(nullptr)
{

}

StateMachine::~StateMachine()
{
	assert(m_currentState == nullptr);
}

GameState* StateMachine::getState() const
{
	return m_currentState;
}

void StateMachine::push(GameState* state)
{
	assert(state != nullptr);
	state->m_previousState = m_currentState;
	m_currentState = state;
}

GameState* StateMachine::pop()
{
	GameState* poppedState = m_currentState;
	m_currentState = m_currentState->m_previousState;
	return poppedState;
}
