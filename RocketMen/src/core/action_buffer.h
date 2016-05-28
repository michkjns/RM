
#pragma once

#include <core/action.h>

#include <string>

static const uint32_t s_maxActions = 16;
//==============================================================================

class ActionBuffer
{
public:
	void insert(const input::Action& action);
	void erase(const input::Action& action);
	void clear();

	uint32_t getNumActions() const;

	input::Action* begin();
	input::Action* end();

private:
	input::Action m_actions[s_maxActions];
	uint32_t      m_numActions;

public:
	inline input::Action operator[] (uint32_t i) const
	{
		return (m_actions[i]);
	}

	inline input::Action& operator[] (uint32_t i)
	{
		return (m_actions[i]);
	}
};