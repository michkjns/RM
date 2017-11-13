
#include <core/action_listener.h>

static std::vector<ActionListener*> s_actionListeners;

std::vector<ActionListener*>& ActionListener::getList()
{
	return s_actionListeners;
}

ActionListener::ActionListener(int32_t playerId) :
	m_playerId(playerId)
{
	s_actionListeners.push_back(this);
}

//ActionListener::ActionListener() :
//	m_playerId(-1)
//{
//	s_actionListeners.push_back(this);
//}

ActionListener::~ActionListener()
{
	for (auto it = s_actionListeners.begin(); it != s_actionListeners.end();)
	{
		if (*it == this)
		{
			s_actionListeners.erase(it);
			return;
		}
		it++;
	}
}

void ActionListener::executeAction(size_t action)
{
	if (m_actionMap.find(action) != m_actionMap.end())
	{
		m_actionMap[action]();
	}
	else
	{
		printf("Action not found! %i\n", (int)action);
	}
}

void ActionListener::setPlayerId(int32_t playerId)
{
	m_playerId = playerId;
}

void ActionListener::clear()
{
	m_actionMap.clear();
}

int32_t ActionListener::getPlayerId() const
{
	return m_playerId;
}
