
#pragma once

#include <utility/buffer.h>
#include <common.h>
#include <core/action.h>

#include <string>

static const uint32_t s_maxActions = 16;
//=============================================================================

namespace network
{
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

	bool serialize(class ReadStream& message);
	bool serialize(class WriteStream& message);

private:
	Buffer<input::Action> m_actions;

public:
	inline input::Action operator[] (uint32_t index) const
	{
		ASSERT(index < getCount() && index >= 0, "Invalid index");
		return (m_actions[index]);
	}

	inline input::Action& operator[] (uint32_t index)
	{
		ASSERT(index < getCount() && index >= 0, "Invalid index");
		return (m_actions[index]);
	}
};
