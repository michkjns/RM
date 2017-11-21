
#pragma once

#include <core/game_state_factory.h>
#include <game/menu_state.h>
#include <game/gameplay_state.h>
#include <game/game_state_ids.h>

namespace rm
{
	class RocketMenStateFactory : public GameStateFactory
	{
	public:
		virtual GameState* getState(uint32_t stateType) const override
		{
			switch (GameStateID(stateType))
			{
				case GameStateID::Menu:
				{
					return new MenuState();
				}
				case GameStateID::Gameplay:
				{
					return new GameplayState();
				}
			}

			return nullptr;
		}
	};

}; // namespace rm