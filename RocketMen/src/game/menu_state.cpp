
#include "menu_state.h"

#include <common.h>
#include <game/game_state_ids.h>
#include <game/rocketmen.h>
#include <network/network.h>

using namespace rm;

MenuState::MenuState()
{
}

MenuState::~MenuState()
{
}

void MenuState::initialize(Game* game)
{
	game->pushState(GameStateID::Gameplay);
}

void MenuState::enter(Game* /*game*/)
{

}

void MenuState::destroy(Game* /*game*/)
{
}

void MenuState::update(Game* /*game*/, const Time& /*time*/)
{
	
}

void MenuState::tick(Game* /*game*/, float /*fixedDeltaTime*/)
{
	//RocketMenGame* rm = static_cast<RocketMenGame*>(game);
}

void MenuState::render(Game* /*game*/)
{
	//RocketMenGame* rm = static_cast<RocketMenGame*>(game);
}
