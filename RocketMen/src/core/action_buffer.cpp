
#include <core/action_buffer.h>

#include <common.h>
#include <network/network_message.h>

using namespace input;

ActionBuffer::ActionBuffer() :
	m_actions(s_maxActions)
{
}

void ActionBuffer::insert(const Action& action)
{
	assert(m_actions.getCount() < s_maxActions);
	Action& entry = m_actions.insert();
	entry = action;
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

void ActionBuffer::readFromMessage(network::IncomingMessage& message)
{
	assert(message.type == network::MessageType::PlayerInput);

	const uint32_t numActions = message.data.readInt32();
	if (numActions > s_maxActions)
	{
		assert(false);
		return;
	}

	message.data.readBytes(reinterpret_cast<char*>(m_actions.begin()), 
		numActions * sizeof(Action));
}

void ActionBuffer::writeToMessage(network::Message& message)
{
	const int32_t numActions = m_actions.getCount();

	message.data.writeInt32(numActions);
	message.data.writeData(reinterpret_cast<const char*>(begin()),
		numActions * sizeof(input::Action));
}
