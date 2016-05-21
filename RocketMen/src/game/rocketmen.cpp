
#include <includes.h>
#include <core/entity.h>
#include <core/input.h>
#include <core/resource_manager.h>
#include <game/character.h>
#include <game/rocketmen.h>
#include <graphics/camera.h>
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

	setTimestep(66666ULL);

	const int32_t pixelsPerMeter = 16;

	Camera::mainCamera = new Camera(graphics::ProjectionMode::ORTOGRAPHIC_PROJECTION,
									Renderer::get()->getScreenSize().x, 
									Renderer::get()->getScreenSize().y,
									pixelsPerMeter);
	Camera::mainCamera->setScale(Vector3(2.f));
//	Input::mapAction("PrintSomething", Key::D);

	Entity* character = new Character();
	character->initialize();
	m_character = dynamic_cast<Character*>(character);
	m_character->setSprite("demoTexture");
	m_character->getRigidbody()->setPosition(Vector2(-3.0f, 5.0f));

	Physics::generateWorld("testmap");
	//m_character->getTransform().setLocalPosition(Vector2(0.0f, 30.0f));
	return false;
}

void RocketMenGame::fixedUpdate(float deltaTime)
{

}

void RocketMenGame::update(const Time& time)
{
	const float deltaTime = time.getDeltaSeconds();

	const float cameraSpeed = 0.20f;

	if (Input::getMouse(MouseButton::MOUSE_LEFT))
	{
	}

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

	if (Input::getKey(Key::W))
	{
		Camera::mainCamera->scale(Vector3(-1.f) * cameraSpeed * 0.01f);
	}

	if (Input::getKey(Key::S))
	{
		Camera::mainCamera->scale(Vector3(1.0f) * cameraSpeed * 0.01f);
	}

	glm::vec2 vel = m_character->getRigidbody()->getLinearVelocity();

	if (Input::getKey(Key::A))
	{
		vel.x = -5.0f;
	}
	
	if (Input::getKey(Key::D))
	{
		vel.x = 5.0f;
	}

	m_character->getRigidbody()->setLinearVelocity(vel);

	if (Input::getKeyDown(Key::SPACE))
	{
		float impulse = m_character->getRigidbody()->getMass() * 10.0f;
		m_character->getRigidbody()->applyLinearImpulse(Vector2(0, impulse), m_character->getRigidbody()->getWorldCenter());
	}
	//Camera::mainCamera->setPosition(Vector3(m_character->getTransform().getWorldPosition(), 0.0f));
	//Camera::mainCamera->setPosition(Vector3(m_character->getRigidbody()->getPosition(), 0.0f));

//	LOG_DEBUG("%f, %f -- %f, %f", 
//			  m_character->getTransform().getWorldPosition().x,
//			  m_character->getTransform().getWorldPosition().y,
//			  m_character->getRigidbody()->getPosition().x,
//			  m_character->getRigidbody()->getPosition().y);
}

void RocketMenGame::terminate()
{
	delete Camera::mainCamera;
}
