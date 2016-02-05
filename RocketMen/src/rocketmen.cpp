
#include "includes.h"
#include "rocketmen.h"
#include "resource_manager.h"

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

}

void RocketMenGame::terminate()
{
}
