
#include <core/game.h>

#include <core/action_listener.h>
#include <core/debug.h>
#include <core/action_buffer.h>
#include <map>

bool Game::initialize()
{
	LOG_INFO("Game: [%s v%s]\n", getName(), getVersion());
	
	setTimestep(33333ULL / 2);
	return true;
}

const char* const Game::getName() const
{
	return "Untitled Game";
}

const char* const Game::getVersion() const
{
	return "0.0";
}

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
		for (auto listener : ActionListener::getList())
		{
			listener->executeAction(actions[i].getHash());
			//input::Action& a = actions[i];
			//if (a == "PrintSomething")
			//{
			//	LOG_INFO("Something!");
			//}
		}
	}

	actions.clear();
}
