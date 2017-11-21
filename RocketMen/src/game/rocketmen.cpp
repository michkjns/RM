
#include <game/rocketmen.h>
#include <common.h>

#include <core/debug.h>
#include <core/entity_manager.h>
#include <core/input.h>
#include <core/resource_manager.h>
#include <game/character.h>
#include <game/rocket.h>
#include <graphics/camera.h>
#include <network/network.h>
#include <physics/physics.h>

using namespace input;
using namespace rm;

bool RocketMenGame::initialize()
{
	Game::initialize();

	const char* defaultMapName = "testmap";

	ResourceManager::loadTexture("data/textures/square.png", "demoTexture");

	ResourceManager::loadTexture("data/textures/tilesheet.png", "tilesheet");

	ResourceManager::loadTilemap("data/testmap.16x16.csv", "tilesheet", defaultMapName);
	Physics::loadCollisionFromTilemap(defaultMapName);

	const int32_t pixelsPerMeter = 16;

	if (Renderer* renderer = Renderer::get())
	{
		Camera::mainCamera = new Camera(graphics::ProjectionMode::Orthographic,
			                            static_cast<float>(renderer->getScreenSize().x),
			                            static_cast<float>(renderer->getScreenSize().y),
		                                pixelsPerMeter);

		Camera::mainCamera->setScale(Vector3(2.f));
	}

	if (Network::isClient())
	{
		Network::addLocalPlayer(Controller::MouseAndKeyboard);
		input::mapAction("Fire", MouseButton::Left, ButtonState::Press);
		input::mapAction("Fire", ControllerButton(0), ButtonState::Press, Controller::Controller_0);

		const char* localHostAddress = "127.0.0.1";
		Network::connect(network::Address(localHostAddress, s_defaultServerPort));
	}

	EntityFactory<Character>::initialize();
	EntityFactory<Rocket>::initialize();

	return true;
}

void RocketMenGame::fixedUpdate(float /*deltaTime*/)
{

}

void RocketMenGame::update(const Time& time)
{
	const float deltaTime = time.getDeltaSeconds();

	const float cameraSpeed = 0.20f * deltaTime;

	const float axis = input::getAxis(ControllerId(0), 0);
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
		Network::disconnect();
	}
}

void RocketMenGame::terminate()
{
	delete Camera::mainCamera;
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
