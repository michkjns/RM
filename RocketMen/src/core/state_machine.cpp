
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
	ASSERT(m_currentState == nullptr, "All states have to be destroyed before the StateMachine");
}

GameState* StateMachine::getState() const
{
	return m_currentState;
}
