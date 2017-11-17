
#pragma once

#include <common.h>
#include <utility.h>

#include <functional>
#include <map>
#include <vector>

enum class ActionType
{
	Default,
	ClientPredicted,
	ClientOnly
};

class ActionListener
{
public:
	static std::vector<ActionListener*>& getList();

public:
	ActionListener(int16_t playerId);
	~ActionListener();

	int16_t getPlayerId() const;

	template<typename F, class I>
	void registerAction(const char* name, void(F::*function)(void), I* object, ActionType type = ActionType::Default)
	{
		if (canRegisterAction(type))
		{
			std::string actionName(toLower(name));
			m_actionMap[std::hash<std::string>()(actionName)] = std::bind(function, object);
		}
	}

	void executeAction(size_t action);
	bool canRegisterAction(ActionType type) const;

	/* Clears all action bindings of this listener */
	void clear();

private:
	std::map<size_t, std::function<void(void)>> m_actionMap;
	int16_t m_playerId;
};