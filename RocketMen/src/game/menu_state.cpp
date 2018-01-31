
#include "menu_state.h"

#include <common.h>
#include <core/debug.h>
#include <core/input.h>
#include <game/game_state_ids.h>
#include <game/rocketmen_game.h>
#include <network/network.h>
#include <utility/commandline_options.h>

using namespace rm;
using namespace std::placeholders;

static const network::Address localhost("127.0.0.1", s_defaultServerPort);
//=============================================================================

MenuState::MenuState() :
	m_locked(false)
{
}

MenuState::~MenuState()
{
}

void MenuState::initialize(Game* game)
{
	ASSERT(game != nullptr);
	m_locked = false;
}

void MenuState::enter(Game* /*game*/)
{

}

void MenuState::destroy(Game* /*game*/)
{
}

void MenuState::update(Game* game, const Time& /*time*/)
{
	ASSERT(game != nullptr);

	if (!m_locked)
	{
		if (input::getKey(input::Key::NUM_1))
		{
			host(game);
			join(game, localhost);
		}
		else if (input::getKey(input::Key::NUM_2))
		{
			host(game);
		}
		else if (input::getKey(input::Key::NUM_3))
		{
			join(game, localhost);
		}

	}
}

void MenuState::tick(Game* /*game*/, float /*fixedDeltaTime*/)
{
}

void MenuState::render(Game* /*game*/)
{
}

void MenuState::parseCommandLineOptions(Game* game, const CommandLineOptions& options)
{
	ASSERT(game != nullptr);

	if (options.isSet("--listen"))
	{
		m_locked = true;
		if (game->createSession(GameSessionType::Listen))
		{
			game->joinSession(localhost, std::bind(&MenuState::onSessionJoinCallback, this, _1, _2));
		}
	}
	else if (options.isSet("--dedicated"))
	{
		host(game);
	}
}

void MenuState::host(Game* game)
{
	m_locked = true;
	if (game->createSession(GameSessionType::Online))
	{
		game->pushState(GameStateID::Gameplay);
	}
}

void MenuState::join(Game* game, const network::Address& address)
{
	ASSERT(game != nullptr);

	m_locked = true;
	game->joinSession(address, std::bind(&MenuState::onSessionJoinCallback, this, _1, _2));
}

void MenuState::onSessionJoinCallback(Game* game, JoinSessionResult result)
{
	ASSERT(game != nullptr);

	if (result == JoinSessionResult::Joined)
	{
		game->pushState(GameStateID::Gameplay);
	}
	else if (result == JoinSessionResult::Failed)
	{
		LOG_INFO("MenuState: Failed to join session");
	}

	m_locked = false;
}
