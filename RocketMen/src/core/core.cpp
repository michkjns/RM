
#include <core/core.h>

#include <graphics/check_gl_error.h>
#include <network/client.h>
#include <core/game.h>
#include <core/resource_manager.h>
#include <core/window.h>
#include <core/entity.h>
#include <graphics/renderer.h>
#include <network.h>
#include <network/address.h>
#include <physics.h>
#include <network/server.h>
#include <time.h>

#include <assert.h>
#include <cstdint>

extern "C" void crcInit(void);

using namespace network;

static bool s_enableDebugDraw = true;

Core::Core() :
	m_client(nullptr),
	m_game(nullptr),
	m_renderer(nullptr),
	m_server(nullptr),
	m_window(nullptr),
	m_input(nullptr),
	m_physics(nullptr),		
	m_timestep(33333ULL / 2),
	m_enableDebug(false)
{

}

Core::~Core()
{
	if(m_client)   delete m_client;
	if(m_server)   delete m_server;
	if(m_renderer) delete m_renderer;

	delete m_window;
}

bool Core::initialize(Game* game, int argc, char* argv[])
{
	if (game == nullptr) {
		LOG_ERROR("Core::initialize: game is null"); assert(false);	return false;
	}

	crcInit();

	bool runDedicated = false;
	bool enableDebug  = true;
	for (int i = 0; i < argc; i++)
	{
		const char* arg = argv[i];
		if (strcmp(arg, "-d") == 0 || strcmp(arg, "--dedicated") == 0)
		{
			runDedicated = true;
		}
		if (strcmp(arg, "--debug") == 0)
		{
			enableDebug = true;
		}
	}
#ifdef _DEBUG
	//m_enableDebug = true;
#endif

	m_game = game;

	if((!runDedicated) || (runDedicated && enableDebug))
	{
		LOG_INFO("Core: Creating window..");
		m_window = Window::create();
		m_window->initialize(g_defaultWidth, g_defaultHeight);

		LOG_INFO("Core: Creating renderer..");
		m_renderer = Renderer::create();
		m_renderer->initialize(m_window);
	}

	if (m_enableDebug)
	{
		LOG_INFO("Debug logging enabled");
		Debug::setVerbosity(Debug::Verbosity::LEVEL_DEBUG);
	}

	if (runDedicated)
	{
		LOG_INFO("Core: Creating server..");
		m_server = new Server(m_gameTime, m_game);
		if (m_server->initialize())
		{
			LOG_INFO("Server: Server succesfully initialized");
			Network::setServer(m_server);
		}
		m_server->host(g_defaultPort);
	}
	else
	{
		LOG_INFO("Core: Creating client..");
		m_client = new Client(m_gameTime, m_game);
		m_client->initialize();
		
		LOG_INFO("Core: Initializing input");
		m_input = Input::create();
		m_input->initialize(m_window);

		Network::setClient(m_client);
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
	for (auto& it : Entity::getList())
	{
		it->debugDraw();
	}

	m_physics->drawDebug();
}

void Core::run()
{
	uint64_t simulatedTime = 0;
	LOG_INFO("Core: Initializing game..");
	m_game->initialize();
	ActionBuffer* actions = nullptr;

	if (m_client)
	{
		m_client->connect(network::Address(g_localHost, g_defaultPort));
	}

	float currTime = m_gameTime.getSeconds();
	float accumulator = 0.0f;
	const float fixedDeltaTime = m_timestep / 1000000.0f;
	float t = 0.0f;

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
			actions = &Input::getActions();
			m_client->update();
		}
		
		m_game->update(m_gameTime);
		for (auto& it : Entity::getList())
		{
			it->update(deltaTime);
		}

		float newTime = m_gameTime.getSeconds();
		float frameTime = newTime - currTime;
		if (frameTime > 0.25f)
			frameTime = 0.25f;
		currTime = newTime;
		accumulator += frameTime;

		while (accumulator >= fixedDeltaTime)
		{
			if (m_client && actions) 
			{
				m_game->processActions(*actions);
				m_client->fixedUpdate(*actions);
			}
			
			m_game->fixedUpdate(fixedDeltaTime);

			for (auto& it : Entity::getList())
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
		}

		if (Input::getKeyDown(input::Key::NUM_1))
		{
			s_enableDebugDraw = !s_enableDebugDraw;
			LOG_INFO(s_enableDebugDraw? "Debug drawing enabled" : " Debug drawing disabled");
		}

		m_input->update();
		Entity::flushEntities();

		/****************/
		/** Render */
		if (m_renderer)
		{
			m_renderer->render();
			if (s_enableDebugDraw)
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

	LOG_INFO("Core: Terminating game..");
	m_game->terminate();

	LOG_INFO("Core: Killing entities..");
	Entity::killEntities();
	Entity::flushEntities();

	if (m_renderer != nullptr)
	{
		LOG_INFO("Core: Terminating renderer..");
		m_renderer->destroy();
	}

	LOG_INFO("Core: Cleaning up resources..");
	ResourceManager::clear();

	LOG_INFO("Core: Terminating physics..");
	Physics::destroyBodies();
	delete m_physics;

	LOG_INFO("Core: Terminating input system..");
	m_input->destroy();

	LOG_INFO("Core: Terminating window..");
	m_window->terminate();
}
