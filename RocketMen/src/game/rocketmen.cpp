
#include <includes.h>
#include <core/input.h>
#include <core/resource_manager.h>
#include <game/character.h>
#include <game/rocket.h>
#include <game/rocketmen.h>
#include <graphics/camera.h>
#include <network.h>
#include <physics.h>

using namespace input;
using namespace rm;

bool RocketMenGame::initialize()
{
	ResourceManager::loadTexture("data/textures/square.png", "demoTexture", 
	                             Texture::BlendMode::MODE_OPAQUE);

	ResourceManager::loadTexture("data/textures/tilesheet.png", "tilesheet",
	                             Texture::BlendMode::MODE_OPAQUE);

	ResourceManager::loadTilemap("data/testmap.16x16.csv", "tilesheet", "testmap");

	setTimestep(33333ULL / 2);

	const int32_t pixelsPerMeter = 16;
	Physics::generateWorld("testmap");

	if (Renderer::get() != nullptr)
	{
		Camera::mainCamera = new Camera(graphics::ProjectionMode::ORTOGRAPHIC_PROJECTION,
		                                Renderer::get()->getScreenSize().x,
		                                Renderer::get()->getScreenSize().y,
		                                pixelsPerMeter);
		Camera::mainCamera->setScale(Vector3(2.f));

	}

	if (Network::isClient())
	{
		Input::mapAction("Fire", MouseButton::MOUSE_LEFT);
		Network::setLocalPlayers(1);
	}

	CharacterFactory::initialize();
	RocketFactory::initialize();

	return true;
}

void RocketMenGame::fixedUpdate(float deltaTime)
{

}

void RocketMenGame::update(const Time& time)
{
	const float deltaTime = time.getDeltaSeconds();

	const float cameraSpeed = 0.20f;

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
	delete Camera::mainCamera;
}

void RocketMenGame::onPlayerJoin()
{
	Character* character = new Character();

	character->getTransform().setLocalPosition( Vector2(-3.f, 3.f));
	character->setSprite("demoTexture");
	
	character->initialize(true);
}
