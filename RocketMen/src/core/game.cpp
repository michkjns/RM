
#include <core/game.h>

#include <core/action_buffer.h>
#include <core/action_listener.h>
#include <core/debug.h>
#include <core/game_state.h>
#include <core/game_state_factory.h>
#include <core/state_machine.h>
#include <network/address.h>
#include <network/client.h>
#include <network/network.h>
#include <network/server.h>
#include <physics/physics.h>

#include <map>

Game::Game() :
	m_timestep(0),
	m_stateFactory(nullptr),
	m_client(nullptr),
	m_server(nullptr),
	m_sessionType(GameSessionType::None)
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
	if (m_server)
	{
		m_server->update(time);
	}

	if (m_client)
	{
		m_client->update(time);
	}

	if (GameState* state = m_stateMachine.getState())
	{
		state->update(this, time);
	}
}

void Game::tick(float fixedDeltaTime, Sequence frameCounter, Physics* physics)
{
	if (m_client)
	{
		m_client->tick(frameCounter);
	}

	if (m_server)
	{
		if (GameState* state = m_stateMachine.getState())
		{
			state->tick(this, fixedDeltaTime);
		}
		physics->step(fixedDeltaTime);
	}
}

void Game::terminate()
{
	delete m_stateFactory;
}

GameState* Game::initialize(GameStateFactory* stateFactory, uint32_t stateId)
{
	LOG_INFO("Game: [%s v%s]\n", getName(), getVersion());

	assert(stateFactory != nullptr);
	m_stateFactory = stateFactory;

	setTimestep(33333ULL / 2);
	return pushState(stateId);
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
	GameState* state = m_stateMachine.pop(this);
	delete state;
}

bool Game::createSession(GameSessionType type)
{
	assert(m_server == nullptr);

	LOG_INFO("Game: Creating server..");
	m_server = new network::Server(this);
	Network::setServer(m_server);

	m_sessionType = type;
	return m_server->host(s_defaultServerPort, type);
}

void Game::joinSession(const network::Address& address, 
	std::function<void(Game*, JoinSessionResult)> callback)
{
	assert(m_client == nullptr);

	LOG_INFO("Game: connecting to %s...", address.toString().c_str());
	m_client = new network::Client(this);
	Network::setClient(m_client);

	m_client->connect(address, callback);
}

void Game::leaveSession()
{
	if (m_server)
	{
		m_server->reset();
		delete m_server;
		m_server = nullptr;
		Network::setServer(nullptr);
	}

	if (m_client)
	{
		m_client->disconnect();
		delete m_client;
		m_client = nullptr;
		Network::setClient(nullptr);
	}

	m_sessionType = GameSessionType::None;
}

bool Game::isSessionActive() const
{
	if (m_server)
	{
		return m_server->getNumClients() > 0;
	}

	if (m_client)
	{
		return m_client->getState() == network::Client::State::Connected;
	}

	return false;
}
