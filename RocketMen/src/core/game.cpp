
#include "game.h"

#include <debug.h>
#include <core/action_buffer.h>
#include <map>

//std::map<size_t, std::function<void(void)>> exampleMap;

uint64_t Game::getTimestep()
{
	return m_timestep;
}

void Game::setTimestep(uint64_t timestep)
{
	m_timestep = timestep;
}

void Game::processActions(ActionBuffer& actions)
{
	for (uint32_t i = 0; i < actions.getNumActions(); i++)
	{
		input::Action& a = actions[i];
		if (a == "PrintSomething")
		{
			LOG_INFO("Something!");
		}
	}

	actions.clear();
}
