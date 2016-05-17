
#include <includes.h>
#include <core/input.h>
#include <core/resource_manager.h>
#include <entity.h>
#include <game/rocketmen.h>
#include <graphics/camera.h>
#include <physics.h>

using namespace input;
using namespace rm;

Entity* testEnt;

bool RocketMenGame::initialize()
{
	ResourceManager::loadTexture("data/textures/square.png", "demoTexture", 
								 Texture::BlendMode::MODE_OPAQUE);

	ResourceManager::loadTexture("data/textures/tilesheet.png", "tilesheet",
								 Texture::BlendMode::MODE_OPAQUE);

	ResourceManager::loadTilemap("data/testmap.8x8.csv", "tilesheet", "testmap");

	setTimestep(66666ULL);

	Camera::mainCamera = new Camera(graphics::ProjectionMode::ORTOGRAPHIC_PROJECTION,
									640.0f, 480.0f);
//	Input::mapAction("PrintSomething", Key::D);

	testEnt = Entity::create();
	testEnt->setRigidbody(Physics::createRigidbody());
	testEnt->getRigidbody()->setPosition(glm::vec2(0, 0));

	return false;
}

void RocketMenGame::fixedUpdate(uint64_t timestep)
{

}

void RocketMenGame::update(const Time& time)
{
	const float deltaTime = time.getDeltaSeconds();

	if (Input::getMouse(MouseButton::MOUSE_LEFT))
	{
	}

	if (Input::getKey(Key::LEFT))
	{
		Camera::mainCamera->translate(glm::vec3(-1.0f, 0.0f, 0.0f) * 5.0f);
	}

	if (Input::getKey(Key::RIGHT))
	{
		Camera::mainCamera->translate(glm::vec3(1.0f, 0.0f, 0.0f) * 5.0f);
	}

	if (Input::getKey(Key::UP))
	{
		Camera::mainCamera->translate(glm::vec3(0.0f, 1.0f, 0.0f) * 5.0f);
	}

	if (Input::getKey(Key::DOWN))
	{
		Camera::mainCamera->translate(glm::vec3(0.0f, -1.0f, 0.0f) * 5.0f);
	}

	glm::vec2 vel = testEnt->getRigidbody()->getLinearVelocity();

	if (Input::getKey(Key::A))
	{
		vel.x = -5.0f;
	}
	
	if (Input::getKey(Key::D))
	{
		vel.x = 5.0f;
	}

	testEnt->getRigidbody()->setLinearVelocity(vel);
}

void RocketMenGame::terminate()
{
	testEnt->destroy();
}
