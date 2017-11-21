
#pragma once

#include <utility/buffer.h>
#include <common.h>
#include <core/action.h>

#include <string>

static const uint32_t s_maxActions = 16;
//=============================================================================

namespace network
{
	struct IncomingMessage;
	struct Message;
};

class ActionBuffer
{
public:
	ActionBuffer();

	void insert(const input::Action& action);
	void insert(const ActionBuffer& other);
	void remove(input::Action& action);
	void clear();

	uint32_t getCount() const;
	bool     isEmpty()  const;

	const input::Action* begin() const;
	const input::Action* end()   const;

	void readFromMessage(network::IncomingMessage& message);
	void writeToMessage(network::Message& message);

private:
	Buffer<input::Action> m_actions;

public:
	inline input::Action operator[] (uint32_t i) const
	{
		assert(i < getCount() && i >= 0);
		return (m_actions[i]);
	}

	inline input::Action& operator[] (uint32_t i)
	{
		assert(i < getCount() && i >= 0);
		return (m_actions[i]);
	}
};
