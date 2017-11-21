
#pragma once

class Game;

class GameState
{
public:
	GameState() {};
	virtual ~GameState() {};

	virtual void initialize(Game* game) = 0;
	virtual void destroy(Game* game)    = 0;
	virtual void tick(Game* game)       = 0;
	virtual void render(Game* game)     = 0;

protected:

};