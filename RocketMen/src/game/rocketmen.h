
#pragma once

#define GAME_VERSION 0

#include <core/game.h>

namespace rm
{
	class RocketMenGame : public Game
	{
	public:
		bool initialize() override;
		void fixedUpdate(uint64_t timestep) override;
		void update(const Time& time) override;
		void terminate() override;

	private:

	};

}; // namespace rm
