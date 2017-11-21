
#pragma once

#include <common.h>
#include <utility/utility.h>

#include <functional>
#include <map>
#include <vector>

//enum class ActionType
//{
//	Default,
//	ClientPredicted,
//	ClientOnly
//};

class ActionListener
{
public:
	static std::vector<ActionListener*>& getList();

public:
	ActionListener(int16_t playerId);
	~ActionListener();

	int16_t getPlayerId() const;

	template<typename F, class I>
	void registerAction(const char* name, bool(F::*function)(void), I* object)
	{
		std::string actionName(toLower(name));
		m_actionMap[std::hash<std::string>()(actionName)] = std::bind(function, object);
	}

	/* @return true if action is to be consumed */
	bool executeAction(size_t action);

	/* Clears all action bindings of this listener */
	void clear();

private:
	std::map<size_t, std::function<bool(void)>> m_actionMap;
	int16_t m_playerId;
};