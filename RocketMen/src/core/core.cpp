
#include <core/core.h>

#include <core/debug.h>
#include <core/game.h>
#include <core/resource_manager.h>
#include <core/window.h>
#include <core/entity.h>
#include <core/entity_manager.h>
#include <graphics/check_gl_error.h>
#include <graphics/renderer.h>
#include <network/network.h>
#include <network/address.h>
#include <network/client.h>
#include <network/server.h>
#include <physics/physics.h>
#include <time.h>

extern "C" void crcInit(void);

using namespace network;

Core::Core() :
	m_game(nullptr),
	m_renderer(nullptr),
	m_window(nullptr),
	m_client(nullptr),
	m_server(nullptr),
	m_physics(nullptr),		
	m_timestep(33333ULL / 2),
	m_enableDebugDraw(true)
{
}

Core::~Core()
{
}

bool Core::initialize(Game* game, int argc, char* argv[])
{
	assert(game != nullptr);

	crcInit();

	bool runDedicated = false;
	bool runListen = false;
	for (int i = 0; i < argc; i++)
	{
		const char* arg = argv[i];
		if (strcmp(arg, "-d") == 0 || strcmp(arg, "--dedicated") == 0)
		{
			runDedicated = true;
			runListen = false;
		}
		if (strcmp(arg, "-l") == 0 || strcmp(arg, "--listen") == 0)
		{
			runDedicated = false;
			runListen = true;
		}
		if (strcmp(arg, "-v") == 0 || strcmp(arg, "--verbosity") == 0)
		{
			if (i < argc - 1)
			{
				int32_t verbosity = atoi(argv[i + 1]);
				verbosity = std::max(0, std::min((int)Debug::Verbosity::Debug, verbosity));
				Debug::setVerbosity(static_cast<Debug::Verbosity>(verbosity));
			}
		}
	}

	m_game = game;

	if(!runDedicated || (runDedicated && Debug::getVerbosity() == Debug::Verbosity::Debug))
	{
		LOG_INFO("Core: Creating window..");
		m_window = Window::create();
		std::string windowTitle(m_game->getName());
		windowTitle.append(runDedicated ? " - Dedicated Server" : " - Client");
		m_window->initialize(windowTitle.c_str(), g_defaultWindowSize);
		
		LOG_INFO("Core: Creating renderer..");
		m_renderer = Renderer::create();
		m_renderer->initialize(m_window);
	}

	if (runDedicated || runListen)
	{
		LOG_INFO("Core: Creating server..");
		m_server = new Server(m_gameTime, m_game);
		Network::setServer(m_server);
		m_server->host(s_defaultServerPort);
	}

	if(!runDedicated)
	{
		LOG_INFO("Core: Creating client..");
		m_client = new Client(m_gameTime, m_game);
		m_client->setPort(s_defaultClientPort);
		Network::setClient(m_client);
		
		LOG_INFO("Core: Initializing input..");
		input::initialize(m_window);
	}

	if (!loadResources())
	{
		LOG_ERROR("Core: Loading resources has failed");
	}

	LOG_INFO("Core: Initializing physics");
	m_physics = new Physics();
	m_physics->initialize();

	return true;
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

void Core::drawDebug()
{
	for (auto& it : EntityManager::getEntities())
	{
		it->debugDraw();
	}

	m_physics->drawDebug();
}

void Core::run()
{
	LOG_INFO("Core: Initializing game..");
	m_game->initialize();

	float currTime = m_gameTime.getSeconds();
	float accumulator = 0.0f;
	const float fixedDeltaTime = m_timestep / 1000000.0f;
	float t = 0.0f;
	Sequence frameId = 0;

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

		/****************
		/** Server Update */
		if (m_server)
		{
			m_server->update();
		}

		/****************/
		/** Client Update */
		if (m_client)
		{
			m_client->update();
		}
		
		m_game->update(m_gameTime);
		for (auto& it : EntityManager::getEntities())
		{
			it->update(deltaTime);
		}

		const float newTime = m_gameTime.getSeconds();
		float frameTime = newTime - currTime;
		if (frameTime > 0.25f)
		{
			frameTime = 0.25f;
		}
		currTime = newTime;
		accumulator += frameTime;

		/****************/
		/** Simulation Loop */
		while (accumulator >= fixedDeltaTime)
		{
			if (m_client) 
			{
				m_client->simulate(frameId);
			}
			
			m_game->fixedUpdate(fixedDeltaTime);

			for (auto& it : EntityManager::getEntities())
			{
				if (it->isAlive())
				{
					it->fixedUpdate(fixedDeltaTime);
				}
			}	

			if (m_server)
			{
				m_physics->step(fixedDeltaTime);
			}

			t += fixedDeltaTime;
			accumulator -= fixedDeltaTime;
			frameId++;
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
			m_renderer->render();
			if (m_enableDebugDraw)
			{
				drawDebug();
			}
			m_window->swapBuffers();
		}
	}

	LOG_DEBUG("Core: main loop ended..");
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

	Network::setClient(nullptr);
	delete m_client;
	
	Network::setServer(nullptr);
	delete m_server;

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
