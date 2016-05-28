
#include <core/action_buffer.h>

#include <assert.h>

using namespace input;

void ActionBuffer::insert(const input::Action& action)
{
	assert(s_maxActions - m_numActions > 1);
	if (m_numActions != s_maxActions)
	{
		m_actions[m_numActions++] = action;
	}
}

void ActionBuffer::erase(const input::Action& action)
{
	for (uint32_t i = 0; i < m_numActions; i++)
	{
		if (m_actions[i] == action)
		{
			memcpy(m_actions + i, m_actions + i + 1, sizeof(Action) * (s_maxActions - (i + 1)));
			m_numActions--;
		}
	}
}

void ActionBuffer::clear()
{
	m_numActions = 0;
	for (auto i : m_actions)
	{
		i.set(0, (float)0);
	}
}

uint32_t ActionBuffer::getNumActions() const
{
	return m_numActions;
}

input::Action* ActionBuffer::begin()
{
	return m_actions;
}

input::Action* ActionBuffer::end()
{
	return m_actions + m_numActions - 1;
}
