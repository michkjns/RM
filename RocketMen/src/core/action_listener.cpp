
#include <core/action_listener.h>

static std::vector<ActionListener*> s_actionListeners;

std::vector<ActionListener*>& ActionListener::getList()
{
	return s_actionListeners;
}

ActionListener::ActionListener() :
	m_playerID(-1),
	m_controllerID(-1)
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

void ActionListener::setPlayerID(int32_t playerID)
{
	m_playerID = playerID;
}

void ActionListener::setControllerID(int32_t controllerID)
{
	m_controllerID = controllerID;
}

void ActionListener::clear()
{
	m_actionMap.clear();
	//m_actionMapTriggers.clear();
	//m_actionMapTriggers.clear();
}

int32_t ActionListener::getPlayerID() const
{
	return m_playerID;
}

int32_t ActionListener::getControllerID() const
{
	return m_controllerID;
}
