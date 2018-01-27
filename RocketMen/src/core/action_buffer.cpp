
#include <core/action_buffer.h>

#include <common.h>
#include <network/message.h>

using namespace input;

ActionBuffer::ActionBuffer() :
	m_actions(s_maxActions)
{
}

void ActionBuffer::insert(const Action& action)
{
	assert(m_actions.getCount() < s_maxActions);
	m_actions.insert(action);
}

void ActionBuffer::insert(const ActionBuffer& otherBuffer)
{
	for (const Action& action : otherBuffer)
	{
		insert(action);
	}
}

void ActionBuffer::remove(Action& action)
{
	m_actions.remove(action);
}

void ActionBuffer::clear()
{
	m_actions.clear();
}

uint32_t ActionBuffer::getCount() const
{
	return m_actions.getCount();
}

bool ActionBuffer::isEmpty() const
{
	return m_actions.getCount() == 0;
}

const input::Action* ActionBuffer::begin() const
{
	return m_actions.begin();
}

const input::Action* ActionBuffer::end() const
{
	return m_actions.end();
}

bool ActionBuffer::serialize(ReadStream& stream)
{
	uint32_t numActions = 0;
	serializeInt(stream, numActions);
	if (numActions > s_maxActions)
	{
		return false;
	}

	if (numActions > 0)
	{
		if (!stream.serializeData(reinterpret_cast<char*>(m_actions.begin()),
			numActions * sizeof(Action)))
		{
			return false;
		}
	}

	return true;
}

bool ActionBuffer::serialize(WriteStream& stream)
{
	uint32_t numActions = m_actions.getCount();

	serializeInt(stream, numActions);
	if (numActions > 0)
	{
		stream.serializeData(reinterpret_cast<const char*>(begin()),
			numActions * sizeof(input::Action));
	}

	return true;
}
