
#pragma once

#include <common.h>
#include <core/input.h>
#include <core/game_time.h>

static const Vector2i g_defaultResolution(640, 480);
static const Vector2i g_defaultWindowSize = g_defaultResolution;

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
	void drawDebug();

	class Game*		 m_game;
	class Renderer*	 m_renderer;
	class Window*    m_window;
	network::Client* m_client;
	network::Server* m_server;
	Time             m_gameTime;
	class Physics*   m_physics;
	uint64_t         m_timestep;
	bool             m_enableDebugDraw;
};
