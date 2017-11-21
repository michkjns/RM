
#include <core/state_machine.h>

#include <common.h>
#include <core/game_state_factory.h>

#include <algorithm>

StateMachine::StateMachine(uint32_t maxDepth) :
	m_currentIndex(INDEX_NONE),
	m_maxDepth(maxDepth)
{
	m_stack = new GameState*[maxDepth];
	std::fill(m_stack, m_stack + maxDepth, nullptr);	
}

StateMachine::~StateMachine()
{
	assert(m_currentIndex == INDEX_NONE);
	delete[] m_stack;
}

GameState* StateMachine::getCurrentState() const
{
	if (m_currentIndex >= 0)
	{
		return m_stack[m_currentIndex];
	}

	return nullptr;
}

GameState* StateMachine::push(GameStateFactory* factory, uint32_t StateId)
{
	assert(factory != nullptr);
	if (m_currentIndex < static_cast<int32_t>(m_maxDepth))
	{
		m_stack[++m_currentIndex] = factory->getState(StateId);
		return getCurrentState();
	}

	return nullptr;
}

void StateMachine::pop()
{
	assert(m_currentIndex >= 0);
	delete m_stack[m_currentIndex--];
}
