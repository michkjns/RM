
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
	void initializeWindow(const char* name);
	void initializeInput();
	void initializeGraphics();

	bool loadResources();

	Game*     m_game;
	Renderer* m_renderer;
	Window*   m_window;
	Time      m_gameTime;
	Physics*  m_physics;
	bool      m_enableDebugDraw;
};
