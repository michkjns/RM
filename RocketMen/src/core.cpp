
#include "includes.h"

#include "core.h"
#include "client.h"
#include "game.h"
#include "window.h"
#include "renderer.h"
#include "resource_manager.h"
#include "server.h"
#include "time.h"

#include <assert.h>
#include <stdint.h>

using namespace network;

Core* Core::g_singleton;

class Core_impl : public Core
{
public:
	Core_impl();
	~Core_impl();
	bool initialize(Game* game, int argc, char* argv[] ) override;
	void run() override;
	void destroy() override;

private:
	bool loadResources();

	Renderer* m_renderer;
	Window* m_window;
	Game* m_game;
	Server* m_server;
	Client* m_client;
	uint64_t m_timestep;

};

Core_impl::Core_impl()
	: m_game(nullptr)
	, m_client(nullptr)
	, m_renderer(nullptr)
	, m_server(nullptr)
	, m_timestep(33333ULL)
	, m_window(nullptr)
{

}

Core_impl::~Core_impl()
{
	if(m_client) delete m_client;
	if(m_server) delete m_server;

	if(m_renderer) delete m_renderer;

	delete m_window;
}

Core* Core::get()
{
	if (g_singleton == nullptr)
	{
		g_singleton = new Core_impl();
	}
	return g_singleton;
}

bool Core_impl::initialize(Game* game, int argc, char* argv[])
{
	assert(game != nullptr);
	
	bool runDedicated = false;
	for (int i = 0; i < argc; i++)
	{
		const char* arg = argv[i];
		if (strcmp(arg, "-d") == 0 || strcmp(arg, "-dedicated") == 0)
		{
			runDedicated = true;
		}
		else if (strcmp(arg, "-debug") == 0)
		{
			Debug::setVerbosity(Debug::EVerbosity::DEBUG);
			LOG_INFO("Debug logging enabled");
		}
	}

	m_game = game;

	LOG_INFO("Core: Creating window..");
	m_window = Window::create();
	m_window->initialize(DEF_WIDTH, DEF_HEIGHT);
	
	LOG_INFO("Core: Creating server..");
	m_server = Server::create();
	m_server->initialize();

	if (!runDedicated)
	{
		LOG_INFO("Core: Creating renderer..");
		m_renderer = Renderer::get();
		m_renderer->initialize(Renderer::EProjectionMode::ORTOGRAPHIC_PROJECTION, m_window);

		LOG_INFO("Core: Creating client..");
		m_client = Client::create();
		m_client->initialize();
	}

	if (!loadResources())
	{
		LOG_ERROR("Core: Loading resources has failed");
	}


	return true;
}

bool Core_impl::loadResources()
{
	ResourceManager::LoadShader("data/shaders/basicSpriteVertexShader.vert"
								, "data/shaders/basicSpriteFragmentShader.frag"
								, "spriteShader");

	return true;
}

void Core_impl::run()
{
	LOG_INFO("Core: Initializing game..");
	m_game->initialize();

	Time gameTime;
	uint64_t updatedTime = 0ULL;

	LOG_DEBUG("Core: Entering main loop..");
	while (!m_window->pollEvents())
	{
		gameTime.update();

		/****************
		/** Server Update */
		if (m_server)
		{
			m_server->tick(gameTime);
			uint64_t currentTime = gameTime.getMicroSeconds();

			/** Fixed timestep simulation */
			while (currentTime - updatedTime > m_timestep)
			{
				m_game->fixedUpdate(m_timestep);
				updatedTime += m_timestep;
			}
		}
		
		m_game->update(gameTime);

		/****************/
		/** Client Update */
		if (m_client)
		{
			m_client->tick();
			m_renderer->render();
		}

		m_window->swapBuffers();
	}

	LOG_DEBUG("Core: main loop ended..");
}

void Core_impl::destroy()
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
