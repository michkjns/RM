
#pragma once

#include <core/game.h>

#undef GAME_VERSION
#undef GAME_NAME
#define GAME_VERSION 0.0
#define GAME_NAME    "RocketMen"

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
		void onPlayerJoin()               override;

	private:
		Character* m_character;
	};

}; // namespace rm
