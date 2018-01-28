
#include <game/rocketmen_game.h>
#include <common.h>

#include <core/debug.h>
#include <core/input.h>
#include <game/game_state_ids.h>
#include <game/rocketmen_state_factory.h>
#include <graphics/camera.h>
#include <core/entity_manager.h>
#include <core/window.h>
#include <game/character.h>
#include <utility/commandline_options.h>

using namespace input;
using namespace rm;

void RocketMenGame::initialize(const GameContext& context)
{
	MenuState* menu = dynamic_cast<MenuState*>(Game::initialize(new RocketMenStateFactory(), GameStateID::Menu));
	assert(menu != nullptr);
	menu->parseCommandLineOptions(this, context.options);

	const int32_t pixelsPerMeter = 32;

	if (Renderer* renderer = Renderer::get())
	{
		assert(context.window != nullptr);

		const Vector2 viewportSize(context.window->getWidth(), context.window->getHeight());
		setMainCamera(new Camera(viewportSize, pixelsPerMeter));	
	}
}

void RocketMenGame::terminate()
{
	Game::terminate();
}

void RocketMenGame::onPlayerJoin(int16_t playerId)
{
	Character* character = new Character();

	character->getTransform().setLocalPosition( Vector2(-3.f, 3.f));
	character->setSprite("demoTexture");
	character->posessbyPlayer(playerId);
	
	EntityManager::instantiateEntity(character);
}

void RocketMenGame::onPlayerLeave(int16_t playerId)
{
	auto& entities = EntityManager::getEntities();
	for (auto entity : entities)
	{
		if (entity->getOwnerPlayerId() == playerId)
		{
			entity->kill();
		}
	}
	LOG_INFO("RM: Player %d has left the game", playerId);
}
