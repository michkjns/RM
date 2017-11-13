
#pragma once

#include <core/action.h>

#include <string>

static const uint32_t s_maxActions = 16;
//==============================================================================

namespace network
{
	struct IncomingMessage;
};

class ActionBuffer
{
public:
	ActionBuffer();

	void insert(const input::Action& action);
	void insert(const ActionBuffer& other);
	void clear();

	uint32_t getCount() const;
	bool     isEmpty()  const;

	const input::Action* begin() const;
	const input::Action* end()   const;

	int32_t readFromMessage(network::IncomingMessage& message);

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