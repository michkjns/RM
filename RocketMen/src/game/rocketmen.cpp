
#include <game/rocketmen.h>
#include <common.h>

#include <core/debug.h>
#include <core/input.h>
#include <game/game_state_ids.h>
#include <game/rocketmen_state_factory.h>
#include <graphics/camera.h>
#include <core/entity_manager.h>
#include <game/character.h>

using namespace input;
using namespace rm;

void RocketMenGame::initialize()
{
	Game::initialize(new RocketMenStateFactory());
	pushState(uint32_t(GameStateID::Gameplay));

	const int32_t pixelsPerMeter = 16;

	if (Renderer* renderer = Renderer::get())
	{
		Camera::mainCamera = new Camera(graphics::ProjectionMode::Orthographic,
			                            static_cast<float>(renderer->getScreenSize().x),
			                            static_cast<float>(renderer->getScreenSize().y),
		                                pixelsPerMeter);

		Camera::mainCamera->setScale(Vector3(2.f));
	}
}

void RocketMenGame::terminate()
{
	delete Camera::mainCamera;
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
	LOG_INFO("RM: Player %d has left", playerId);
}
