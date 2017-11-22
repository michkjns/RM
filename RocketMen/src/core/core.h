
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
class CommandLineOptions;

class Core
{
public:
	Core();
	~Core();
	void initialize(Game* game, const CommandLineOptions& options);
	void run();
	void destroy();

private:
	bool loadResources();
	void drawDebug();

	class Game*		 m_game;
	class Renderer*	 m_renderer;
	class Window*    m_window;
	Time             m_gameTime;
	class Physics*   m_physics;
	uint64_t         m_timestep;
	bool             m_enableDebugDraw;
};
