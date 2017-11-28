
#include "gameplay_state.h"

#include <core/entity_manager.h>
#include <core/game_time.h>
#include <core/input.h>
#include <core/input.h>
#include <core/resource_manager.h>
#include <game/character.h>
#include <game/moving_cube.h>
#include <game/rocket.h>
#include <game/rocketmen.h>
#include <graphics/camera.h>
#include <network/network.h>
#include <physics/physics.h>

using namespace rm;
using namespace input;

void GameplayState::initialize(Game* /*game*/)
{
	const char* defaultMapName = "testmap";

	ResourceManager::loadTexture("data/textures/square.png", "demoTexture");
	ResourceManager::loadTexture("data/textures/tilesheet.png", "tilesheet");
	ResourceManager::loadTilemap("data/testmap.16x16.csv", "tilesheet", defaultMapName);
	Physics::loadCollisionFromTilemap(defaultMapName);

	if (Network::isClient())
	{
		Network::addLocalPlayer(Controller::MouseAndKeyboard);
		input::mapAction("Fire", MouseButton::Left, ButtonState::Press);
		input::mapAction("Fire", ControllerButton(0), ButtonState::Press, Controller::Controller_0);
	}
}

void GameplayState::enter(Game* /*game*/)
{
	if (Network::isServer())
	{
		MovingCube* cube = new MovingCube();
		Entity::instantiate(cube);
	}
}

void GameplayState::destroy(Game* /*game*/)
{
}

void GameplayState::update(Game* game, const Time& time)
{
	const float deltaTime = time.getDeltaSeconds();

	const float axis = input::getAxis(ControllerId(0), 0);
	const float cameraSpeed = 2.20f * deltaTime;
	Camera::mainCamera->translate(Vector3(axis, 0.0f, 0.0f) * cameraSpeed);

	if (input::getKey(Key::LEFT))
	{
		Camera::mainCamera->translate(Vector3(-1.0f, 0.0f, 0.0f) * cameraSpeed);
	}

	if (input::getKey(Key::RIGHT))
	{
		Camera::mainCamera->translate(Vector3(1.0f, 0.0f, 0.0f) * cameraSpeed);
	}

	if (input::getKey(Key::UP))
	{
		Camera::mainCamera->translate(Vector3(0.0f, 1.0f, 0.0f) * cameraSpeed);
	}

	if (input::getKey(Key::DOWN))
	{
		Camera::mainCamera->translate(Vector3(0.0f, -1.0f, 0.0f) * cameraSpeed);
	}

	if (input::getKey(Key::ESCAPE))
	{
		game->leaveSession();
		game->popState();
	}
}

void GameplayState::tick(Game* /*game*/, float fixedDeltaTime)
{
	for (auto& it : EntityManager::getEntities())
	{
		if (it->isAlive())
		{
			it->fixedUpdate(fixedDeltaTime);
		}
	}
}

void GameplayState::render(Game* /*game*/)
{
	//RocketMenGame* rm = static_cast<RocketMenGame*>(game);
}
