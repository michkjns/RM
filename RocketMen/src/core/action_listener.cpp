
#include <core/action_listener.h>
#include <core/debug.h>
#include <network/network.h>

static std::vector<ActionListener*> s_actionListeners;

std::vector<ActionListener*>& ActionListener::getList()
{
	return s_actionListeners;
}

ActionListener::ActionListener(int16_t playerId) :
	m_playerId(playerId)
{
	s_actionListeners.push_back(this);
}

ActionListener::~ActionListener()
{
	for (auto it = s_actionListeners.begin(); it != s_actionListeners.end();)
	{
		if (*it == this)
		{
			s_actionListeners.erase(it);
			return;
		}
		++it;
	}
}

bool ActionListener::executeAction(size_t action)
{
	if (m_actionMap.find(action) != m_actionMap.end())
	{
		return m_actionMap[action]();
	}
	
	LOG_DEBUG("Action not found! %i\n", (int)action);
	return false;
}

void ActionListener::clear()
{
	m_actionMap.clear();
}

int16_t ActionListener::getPlayerId() const
{
	return m_playerId;
}
