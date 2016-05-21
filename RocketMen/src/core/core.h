
#pragma once

#include <includes.h>
#include <core/input.h>
#include <game_time.h>

static const uint32_t g_defaultWidth  = 640;
static const uint32_t g_defaultHeight = 480;
static const uint32_t g_defaultPort   = 4321;
static const char*    g_localHost     = "127.0.0.1";

class Game;
class Renderer;
class Physics;
class Window;

namespace network {
	class Server;
	class Client;
}; // namespace network

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

	Game*		     m_game;
	Renderer*	     m_renderer;
	Window*          m_window;
	network::Client* m_client;
	network::Server* m_server;
	Time             m_gameTime;
	Input*           m_input;
	Physics*         m_physics;
	uint64_t         m_timestep;
};