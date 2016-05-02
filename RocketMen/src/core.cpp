
#include "core.h"

#include "address.h"
#include "check_gl_error.h"
#include "client.h"
#include "game.h"
#include "renderer.h"
#include "resource_manager.h"
#include "server.h"
#include "time.h"
#include "window.h"

#include <assert.h>
#include <stdint.h>

using namespace network;

Core::Core() :
	m_client(nullptr),
	m_game(nullptr),
	m_renderer(nullptr),
	m_server(nullptr),
	m_window(nullptr),
	m_timestep(33333ULL)
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
		m_server = new Server(m_gameTime);
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
		m_renderer->initialize(Renderer::ProjectionMode::ORTOGRAPHIC_PROJECTION, m_window);

		LOG_INFO("Core: Creating client..");
		m_client = new Client(m_gameTime);
		m_client->initialize();
	}

	if (!loadResources())
	{
		LOG_ERROR("Core: Loading resources has failed");
	}

	return true;
}

bool Core::loadResources()
{

	ResourceManager::loadShader("data/shaders/basicSpriteVertexShader.vert",
								"data/shaders/basicSpriteFragmentShader.frag",
								"spriteShader");


	return true;
}

void Core::run()
{
	LOG_INFO("Core: Initializing game..");
	m_game->initialize();
	checkGL();

	uint64_t updatedTime = 0ULL;

	if (m_client)
	{
		m_client->connect(network::Address(g_localHost, g_defaultPort));
	}

	LOG_DEBUG("Core: Entering main loop..");
	checkGL();
	while (!m_window->pollEvents())
	{
		checkGL();
		m_gameTime.update();
		const uint64_t currentTime = m_gameTime.getMicroSeconds();
		const float deltaTime = m_gameTime.getDeltaSeconds();

		/****************
		/** Server Update */
		if (m_server)
		{
			m_server->update();
		}
		m_game->update(m_gameTime);

		/****************/
		/** Client Update */
		if (m_client)
		{
			m_client->update();
			m_renderer->render();
		}

		/** Fixed timestep simulation */
		while (currentTime - updatedTime > m_timestep)
		{
			m_game->fixedUpdate(m_timestep);
			updatedTime += m_timestep;
		}

		m_window->swapBuffers();
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

	LOG_INFO("Core: Terminating window..");
	m_window->terminate();

}
