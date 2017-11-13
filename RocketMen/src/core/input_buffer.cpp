
#include <core/input_buffer.h>

#include <common.h>
#include <network/network_message.h>

using namespace input;

ActionBuffer::ActionBuffer() :
	m_numActions(0)
{
}

void ActionBuffer::insert(const input::Action& event)
{
	assert(m_numActions < s_maxActions);
	if (m_numActions != s_maxActions)
	{
		m_actions[m_numActions++] = event;
	}
}

void ActionBuffer::insert(const ActionBuffer& other)
{
	for (auto action : other)
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

int32_t ActionBuffer::readFromMessage(network::IncomingMessage& message)
{
	assert(message.type == network::MessageType::PlayerInput);
	const int32_t playerId = message.data.readInt32();
	if (playerId <= INDEX_NONE)
	{
		return INDEX_NONE;
	}
	const uint32_t numEvents = message.data.readInt32();
	if (numEvents > s_maxActions)
	{
		return INDEX_NONE;
	}

	message.data.readBytes(reinterpret_cast<char*>(m_actions), numEvents * sizeof(input::Action));
	m_numActions = numEvents;
	return playerId;
}
