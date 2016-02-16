
#pragma once

#include "includes.h"

#include "game_time.h"

static const uint32_t g_defaultWidth = 640;
static const uint32_t g_defaultHeight = 480;
static const uint32_t g_defaultPort = 4321;
static const char*    g_localHost = "127.0.0.1";

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

	Client*		m_client;
	Game*		m_game;
	Renderer*	m_renderer;
	Server*		m_server;
	Window*		m_window;
	Time		m_gameTime;
	uint64_t	m_timestep;
};