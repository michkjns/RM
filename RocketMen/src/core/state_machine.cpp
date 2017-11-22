
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
