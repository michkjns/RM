
#pragma once

#include <network/session.h> 
#include <core/game_state.h>

class CommandLineOptions;

namespace network {
	class Address;
}; // namespace network

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

		void parseCommandLineOptions(Game* game, const CommandLineOptions& options);
		void hostAndJoin(Game* game);
		void host(Game* game);
		void join(Game* game, const network::Address& address);

	private:
		void onSessionJoinCallback(Game* game, JoinSessionResult result);

		//Game* m_game;
		bool  m_locked;
	};
}; //namespace rm