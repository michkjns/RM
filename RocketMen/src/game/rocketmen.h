
#pragma once

#include <core/game.h>

class CommandLineOptions;

namespace rm
{
	class RocketMenGame : public Game
	{
	public:
		DECLARE_GAME_INFO("RocketMen", "0.0");
		RocketMenGame() {}
		~RocketMenGame() {}

	public:
		void initialize(const CommandLineOptions& options) override;
		void terminate()                     override;
		void onPlayerJoin(int16_t playerId)  override;
		void onPlayerLeave(int16_t playerId) override;
	};

}; // namespace rm
