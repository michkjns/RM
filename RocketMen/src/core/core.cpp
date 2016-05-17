
#include <core/core.h>

#include <graphics/check_gl_error.h>
#include <client.h>
#include <core/game.h>
#include <core/resource_manager.h>
#include <core/window.h>
#include <entity.h>
#include <graphics/renderer.h>
#include <network/address.h>
#include <physics.h>
#include <server.h>
#include <time.h>

#include <assert.h>
#include <cstdint>

using namespace network;

Core::Core() :
	m_client(nullptr),
	m_game(nullptr),
	m_renderer(nullptr),
	m_server(nullptr),
	m_window(nullptr),
	m_input(nullptr),
	m_physics(nullptr),
	m_timestep(33333ULL * 2)
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
	assert(game != nullptr);
	
	bool runDedicated = false;
	for (int i = 0; i < argc; i++)
	{
		const char* arg = argv[i];
		if (strcmp(arg, "-d") == 0 || strcmp(arg, "--dedicated") == 0)
		{
			runDedicated = true;
		}
		if (strcmp(arg, "--debug") == 0)
		{
			Debug::setVerbosity(Debug::Verbosity::LEVEL_DEBUG);
			LOG_INFO("Debug logging enabled");
		}
	}

	m_game = game;

	LOG_INFO("Core: Creating window..");
	m_window = Window::create();
	m_window->initialize(g_defaultWidth, g_defaultHeight);
	
	if (runDedicated)
	{
		LOG_INFO("Core: Creating server..");
		m_server = new Server(m_gameTime, m_game);
		if (m_server->initialize())
		{
			LOG_INFO("Server: Server succesfully initialized");
		}
		m_server->host(g_defaultPort);
	}
	else
	{
		LOG_INFO("Core: Creating renderer..");
		m_renderer = Renderer::get();
		m_renderer->initialize(m_window);

		LOG_INFO("Core: Creating client..");
		m_client = new Client(m_gameTime, m_game);
		m_client->initialize();
	}

	if (!loadResources())
	{
		LOG_ERROR("Core: Loading resources has failed");
	}

	LOG_INFO("Core: Initializing input");
	m_input = Input::create();
	m_input->initialize(m_window);

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
	return true;
}

void Core::run()
{
	LOG_INFO("Core: Initializing game..");
	m_game->initialize();

	if (m_client)
	{
		m_client->connect(network::Address(g_localHost, g_defaultPort));
	}

	LOG_DEBUG("Core: Entering main loop..");
	while (!m_window->pollEvents())
	{
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
			m_renderer->render();
		}
		
		m_game->update(m_gameTime);
		m_physics->step(m_timestep * 0.00001f);

		m_window->swapBuffers();
		m_input->update();
		Entity::flushEntities();
	}

	LOG_DEBUG("Core: main loop ended..");
}

void Core::destroy()
{
	LOG_INFO("Core: Shutting down..");

	LOG_INFO("Core: Terminating game..");
	m_game->terminate();

	if (m_renderer != nullptr)
	{
		LOG_INFO("Core: Terminating renderer..");
		m_renderer->destroy();
	}

	delete m_physics;
	m_input->destroy();
	LOG_INFO("Core: Terminating window..");
	m_window->terminate();

}
