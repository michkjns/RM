
#pragma once

#include <game/game_state.h>

class MenuState : public GameState
{
public:
	MenuState();
	virtual ~MenuState() override;

	virtual void initialize(Game* game) override;
	virtual void destroy(Game* game) override;
	virtual void tick(Game* game) override;
	virtual void render(Game* game) override;
};