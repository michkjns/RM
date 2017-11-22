
#pragma once

#include <game/game_session.h> 
#include <core/game_state.h>

namespace rm
{
	class MenuState : public GameState
	{
	public:
		MenuState();
		virtual ~MenuState() override;

		virtual void initialize(Game* game)                 override;
		virtual void enter(Game* game)                      override;
		virtual void destroy(Game* game)                    override;
		virtual void update(Game* game, const Time& time)   override;
		virtual void tick(Game* game, float fixedDeltaTime) override;
		virtual void render(Game* game)                     override;

	private:
		void singlePlayer(Game* game);
		void host(Game* game);
		void join(Game* game);

		void onResult(SessionResult result);

		Game* m_game;
		bool  m_locked;
	};
}; //namespace rm