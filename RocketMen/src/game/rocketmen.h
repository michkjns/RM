
#pragma once

#define GAME_VERSION 0

#include <core/game.h>


namespace rm
{
	class Character;
	class RocketMenGame : public Game
	{
	public:
		bool initialize()                 override;
		void fixedUpdate(float deltaTime) override;
		void update(const Time& time)     override;
		void terminate()                  override;

	private:
		Character* m_character;
	};

}; // namespace rm
