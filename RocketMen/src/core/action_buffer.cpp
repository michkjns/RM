
#include <core/action_buffer.h>

#include <assert.h>

void ActionBuffer::insert(const input::Action& action)
{
	assert(s_maxActions - m_numActions > 1);
	if (m_numActions != s_maxActions)
	{
		m_actions[m_numActions++] = action;
	}
}

void ActionBuffer::clear()
{
	m_numActions = 0;
	for (auto i : m_actions)
	{
		i.set(0, 0);
	}
}

uint32_t ActionBuffer::getNumActions() const
{
	return m_numActions;
}
