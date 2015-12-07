
#pragma once

#define DEF_WIDTH 640
#define DEF_HEIGHT 480

class Game;

class Core
{
public:
	virtual ~Core() {};

	virtual bool initialize(Game* game, int argc, char* argv[]) = 0;
	virtual void run() = 0;
	virtual void destroy() = 0;


	static Core* get();

private:
	static Core* g_singleton;
};