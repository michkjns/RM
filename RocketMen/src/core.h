
#pragma once

#include "includes.h"

#define DEF_WIDTH 640
#define DEF_HEIGHT 480

class Game;
class Renderer;
class Window;
class Server;
class Client;

class Core
{
public:
	Core();
	~Core();
	bool initialize(Game* game, int argc, char* argv[]);
	void run();
	void destroy();

private:
	bool loadResources();

	Client* m_client;
	Game* m_game;
	Renderer* m_renderer;
	Server* m_server;
	Window* m_window;
	uint64_t m_timestep;
};