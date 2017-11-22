
#include <core/game.h>

#include <core/action_buffer.h>
#include <core/action_listener.h>
#include <core/debug.h>
#include <core/game_state.h>
#include <core/game_state_factory.h>
#include <core/state_machine.h>

#include <map>

Game::Game()
{
}

Game::~Game()
{
	while (GameState* state = m_stateMachine.getState())
	{
		popState();
	}
}

void Game::update(const Time& time)
{
	if (GameState* state = m_stateMachine.getState())
	{
		state->update(this, time);
	}
}

void Game::tick(float fixedDeltaTime)
{
	if (GameState* state = m_stateMachine.getState())
	{
		state->tick(this, fixedDeltaTime);
	}
}

void Game::terminate()
{
	delete m_stateFactory;
}

void Game::initialize(GameStateFactory* stateFactory, uint32_t stateId)
{
	LOG_INFO("Game: [%s v%s]\n", getName(), getVersion());

	assert(stateFactory != nullptr);
	m_stateFactory = stateFactory;

	setTimestep(33333ULL / 2);
	pushState(stateId);
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

void Game::processPlayerActions(ActionBuffer& actions, int16_t playerId)
{
	for (uint32_t i = 0; i < actions.getCount();)
	{
		input::Action& action = actions[i];
		for (auto listener : ActionListener::getList())
		{
			if (listener->getPlayerId() == playerId)
			{
				if (listener->executeAction(action.getHash()))
				{
					actions.remove(action);
				}
				else
				{
					i++;
				}
			}
		}
	}
}

GameState* Game::pushState(uint32_t stateId)
{
	GameState* state = m_stateFactory->getState(stateId);
	assert(state != nullptr);

	m_stateMachine.push(state, this);

	return state;
}

void Game::popState()
{
	m_stateMachine.pop(this);
}
