
#include <game/rocketmen.h>
#include <common.h>

#include <core/debug.h>
#include <core/entity_manager.h>
#include <core/input.h>
#include <core/resource_manager.h>
#include <game/character.h>
#include <game/rocket.h>
#include <graphics/camera.h>
#include <network.h>
#include <physics.h>

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
		Network::addLocalPlayer(0);
		Input::mapAction("Fire", MouseButton::Left, ButtonState::Press);
		Input::mapAction("Fire", ControllerButton(1), ButtonState::Press, 0);

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

	const float cameraSpeed = 0.20f;

	const float axis = Input::getAxis(0, 0);
	Camera::mainCamera->translate(Vector3(axis, 0.0f, 0.0f) * cameraSpeed);

	if (Input::getKey(Key::LEFT))
	{
		Camera::mainCamera->translate(Vector3(-1.0f, 0.0f, 0.0f) * cameraSpeed);
	}

	if (Input::getKey(Key::RIGHT))
	{
		Camera::mainCamera->translate(Vector3(1.0f, 0.0f, 0.0f) * cameraSpeed);
	}

	if (Input::getKey(Key::UP))
	{
		Camera::mainCamera->translate(Vector3(0.0f, 1.0f, 0.0f) * cameraSpeed);
	}

	if (Input::getKey(Key::DOWN))
	{
		Camera::mainCamera->translate(Vector3(0.0f, -1.0f, 0.0f) * cameraSpeed);
	}
}

void RocketMenGame::terminate()
{
	if(Camera::mainCamera) delete Camera::mainCamera;
}

void RocketMenGame::onPlayerJoin(int32_t playerId)
{
	Character* character = new Character();

	character->getTransform().setLocalPosition( Vector2(-3.f, 3.f));
	character->setSprite("demoTexture");
	character->posessbyPlayer(playerId);
	
	EntityManager::instantiateEntity(character);
}
