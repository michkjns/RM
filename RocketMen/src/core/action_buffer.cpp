
#include <core/action_buffer.h>

#include <common.h>
#include <network/network_message.h>

using namespace input;

ActionBuffer::ActionBuffer() :
	m_numActions(0)
{
}

void ActionBuffer::insert(const Action& action)
{
	assert(m_numActions < s_maxActions);
	if (m_numActions != s_maxActions)
	{
		m_actions[m_numActions++] = action;

	}
}

void ActionBuffer::insert(const ActionBuffer& otherBuffer)
{
	for (const Action& action : otherBuffer)
	{
		insert(action);
	}
}

void ActionBuffer::clear()
{
	m_numActions = 0;
}

uint32_t ActionBuffer::getCount() const
{
	return m_numActions;
}

bool ActionBuffer::isEmpty() const
{
	return getCount() == 0;
}

const input::Action* ActionBuffer::begin() const
{
	return m_actions;
}

const input::Action* ActionBuffer::end() const
{
	return m_actions + m_numActions;
}

void ActionBuffer::readFromMessage(network::IncomingMessage& message)
{
	assert(message.type == network::MessageType::PlayerInput);

	const uint32_t numActions = message.data.readInt32();
	if (numActions > s_maxActions)
	{
		assert(false);
		return;
	}

	message.data.readBytes(reinterpret_cast<char*>(m_actions), numActions * sizeof(Action));
	m_numActions = numActions;
	assert(m_numActions < s_maxActions);
}

void ActionBuffer::writeToMessage(network::Message& message)
{
	const uint32_t numActions = getCount();
	assert(numActions < s_maxActions);

	message.data.writeInt32(numActions);
	message.data.writeData(reinterpret_cast<const char*>(begin()),
		numActions * sizeof(input::Action));
}
