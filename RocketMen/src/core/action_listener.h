
#pragma once

#include <common.h>
#include <utility.h>

#include <functional>
#include <map>
#include <vector>

class ActionListener
{
public:
	static std::vector<ActionListener*>& getList();

public:
	ActionListener();
	~ActionListener();

	void setPlayerId(int32_t playerId);
	void setControllerId(int32_t controllerId);

	int32_t getPlayerId()     const;
	int32_t getControllerId() const;

	template<typename F, class I>
	void registerAction(const char* name, void(F::*function)(void), I* object)
	{
		std::string actionName(toLower(name));
		m_actionMap[std::hash<std::string>()(actionName)] = std::bind(function, object);
	}

	void executeAction(size_t action);

	/* Clears all action bindings of this listener */
	void clear();

private:
	std::map<size_t, std::function<void(void)>> m_actionMap;

	int32_t m_playerId;
	int32_t m_controllerId;

};