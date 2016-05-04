
#include <includes.h>
#include <input.h>
#include <game/rocketmen.h>
#include <resource_manager.h>


using namespace input;

bool RocketMenGame::initialize()
{
	ResourceManager::loadTexture("data/textures/demoTexture.png", "demoTexture", Texture::EBlendMode::MODE_OPAQUE);

	return false;
}

void RocketMenGame::fixedUpdate(uint64_t timestep)
{

}

void RocketMenGame::update(const Time& time)
{
	const float deltaTime = time.getDeltaSeconds();


	if (Input::getKey(Key::A))
	{
		LOG_DEBUG("getKey A");
	}

	if (Input::getKeyDown(Key::S))
	{
		LOG_DEBUG("getKeyDown S");
	}

	if (Input::getMouseDown(MouseButton::MOUSE_LEFT))
	{
		LOG_DEBUG("getMouseDown LEFT %f, %f", Input::getMousePosition().x,
				  Input::getMousePosition().y);
	}

	if (Input::getMouse(MouseButton::MOUSE_RIGHT))
	{
		LOG_DEBUG("getMouse RIGHT %f, %f", Input::getMousePosition().x,
				  Input::getMousePosition().y);
	}
}

void RocketMenGame::terminate()
{
}
