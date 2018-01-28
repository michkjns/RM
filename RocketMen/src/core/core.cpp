
#include <core/core.h>

#include <core/debug.h>
#include <core/game.h>
#include <core/resource_manager.h>
#include <core/window.h>
#include <core/entity.h>
#include <core/entity_manager.h>
#include <graphics/camera.h>
#include <graphics/check_gl_error.h>
#include <graphics/renderer.h>
#include <network/network.h>
#include <network/address.h>
#include <network/local_client.h>
#include <network/server.h>
#include <physics/physics.h>
#include <time.h>
#include <utility/commandline_options.h>

extern "C" void crcInit(void);

using namespace network;

Core::Core() :
	m_game(nullptr),
	m_renderer(nullptr),
	m_window(nullptr),
	m_physics(nullptr),		
	m_enableDebugDraw(true)
{
}

Core::~Core()
{
}

void Core::initialize(Game* game, const CommandLineOptions& options)
{
	assert(game != nullptr);
	m_game = game;

	crcInit();

	const bool isDedicatedServer = options.isSet("--dedicated");
	const bool isHeadless = isDedicatedServer && Debug::getVerbosity() != Debug::Verbosity::Debug;
	
	if(!isHeadless)
	{
		std::string windowTitle(m_game->getName());
		windowTitle += (isDedicatedServer ? " - Dedicated Server" : " - Client");

		initializeWindow(windowTitle.c_str());
		initializeInput();
		initializeGraphics();
	}

	if (!loadResources())
	{
		LOG_ERROR("Core: Loading resources has failed");
	}

	LOG_INFO("Core: Initializing physics");
	m_physics = new Physics();
	m_physics->initialize();

	LOG_INFO("Core: Initializing game..");
	const GameContext context = { options, m_window };
	m_game->initialize(context);
}

void Core::initializeWindow(const char* name)
{
	LOG_INFO("Core: Creating window..");
	m_window = Window::create();
	m_window->initialize(name, g_defaultWindowSize);
}

void Core::initializeGraphics()
{
	LOG_INFO("Core: Creating renderer..");
	m_renderer = Renderer::create();
	m_renderer->initialize();
}

void Core::initializeInput()
{
	LOG_INFO("Core: Initializing input..");
	input::initialize(m_window);
}

bool Core::loadResources()
{
	ResourceManager::loadShader("data/shaders/sprite_shader.vert",
								"data/shaders/sprite_shader.frag",
								"sprite_shader");

	ResourceManager::loadShader("data/shaders/tile_shader.vert",
								"data/shaders/tile_shader.frag",
								"tile_shader");

	ResourceManager::loadShader("data/shaders/line_shader.vert",
								"data/shaders/line_shader.frag",
								"line_shader");
	return true;
}

void Core::run()
{
	assert(m_game != nullptr);

	float currentTime = m_gameTime.getSeconds();
	float accumulator = 0.0f;
	const float fixedDeltaTime = m_game->getTimestep() / 1000000.0f;
	float t = 0.0f;
	Sequence frameCounter = 0;

	LOG_DEBUG("Core: Entering main loop..");
	bool exit = false;
	while (!exit)
	{
		if (m_window)
		{
			exit = m_window->pollEvents();
		}
		m_gameTime.update();

		const float deltaTime = m_gameTime.getDeltaSeconds();
		m_game->update(m_gameTime);

		for (auto& it : EntityManager::getEntities())
		{
			it->update(deltaTime);
		}

		const float newTime = m_gameTime.getSeconds();
		float frameTime = newTime - currentTime;
		if (frameTime > 0.25f)
		{
			frameTime = 0.25f;
		}
		currentTime = newTime;
		accumulator += frameTime;

		/****************/
		/** Simulation Loop */
		if (m_game->isSessionActive())
		{
			while (accumulator >= fixedDeltaTime)
			{
				m_game->tick(fixedDeltaTime, frameCounter, m_physics);

				t += fixedDeltaTime;
				accumulator -= fixedDeltaTime;
				frameCounter++;
			}
		}

		if (input::getKeyDown(input::Key::NUM_1))
		{
			m_enableDebugDraw = !m_enableDebugDraw;
			LOG_INFO(m_enableDebugDraw ? "Debug drawing enabled" : "Debug drawing disabled");
		}

		input::update();
		EntityManager::flushEntities();

		/****************/
		/** Render */
		if (m_renderer)
		{
			const RenderContext renderContext = { m_physics, *m_game };
			m_renderer->render(renderContext, m_enableDebugDraw);
			m_window->swapBuffers();
		}
	}

	LOG_DEBUG("Core: main loop ended");
}

void Core::destroy()
{
	LOG_INFO("Core: Shutting down..");

	LOG_INFO("Core: Cleaning up entities..");
	EntityManager::killEntities();
	EntityManager::flushEntities();

	LOG_INFO("Core: Terminating physics..");
	Physics::destroyBodies();
	delete m_physics;

	LOG_INFO("Core: Cleaning up resources..");
	ResourceManager::clear();

	if (m_renderer != nullptr)
	{
		LOG_INFO("Core: Terminating renderer..");
		m_renderer->destroy();
		delete m_renderer;
	}

	LOG_INFO("Core: Terminating game..");
	m_game->terminate();

	LOG_INFO("Core: Terminating window..");
	m_window->terminate();

	delete m_window;
}
