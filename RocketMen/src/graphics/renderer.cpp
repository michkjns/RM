
#include <includes.h>
#include <graphics/renderer.h>

#include <core/entity.h>
#include <graphics/camera.h>
#include <graphics/check_gl_error.h>
#include <graphics/sprite_renderer.h>
#include <graphics/tile_renderer.h>
#include <core/resource_manager.h>
#include <core/window.h>
#include <fstream>

#include <GLFW/glfw3.h>

Renderer* Renderer::g_singleton;

class Renderer_impl : public Renderer
{
public:
	Renderer_impl();
	~Renderer_impl();

	bool initialize(Window* window) override;
	void destroy() override;

	void render() override;

private:

	void renderSprites();
	void renderTiles();
	void renderUI();

	SpriteRenderer m_spriteRenderer;
	TileRenderer   m_tileRenderer;
	Window*        m_window;
};

Renderer_impl::Renderer_impl()
	: m_window(nullptr)
{
}

Renderer_impl::~Renderer_impl()
{
}

bool Renderer_impl::initialize(Window* window)
{
	assert(window != nullptr);

	m_window = window;

	if (!m_tileRenderer.initialize())
	{
		LOG_ERROR("Renderer: Tilemap::initialize Error");
		return false;
	}

	if (!m_spriteRenderer.initialize())
	{
		LOG_ERROR("Renderer: SpriteRenderer::initialize Error");
		return false;
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	checkGL();
	
	return true;
}

void Renderer_impl::destroy()
{
}

void Renderer_impl::render()
{
	if (Camera::mainCamera) {
		Camera::mainCamera->updateViewMatrix();
	}
	if (true)
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	renderTiles();
	renderSprites();
	renderUI();
}

void Renderer_impl::renderSprites()
{
	glm::mat4 modelMatrix = glm::mat4();
	glm::vec2 size(ResourceManager::getTexture("demoTexture").getWidth(), 
				   ResourceManager::getTexture("demoTexture").getHeight());
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(size, 1.0f));

	if (Camera::mainCamera) 
	{
		m_spriteRenderer.render(modelMatrix, Camera::mainCamera->getProjectionMatrix()
								* Camera::mainCamera->getViewMatrix());
	}
}

void Renderer_impl::renderTiles()
{
	if (Camera::mainCamera) 
	{
		m_tileRenderer.render(&ResourceManager::getTileMap("testmap"), Camera::mainCamera->getProjectionMatrix()
						 * Camera::mainCamera->getViewMatrix());
	}
}

void Renderer_impl::renderUI()
{
}

Renderer* Renderer::get()
{
	if (g_singleton == nullptr)
	{
		g_singleton = new Renderer_impl();
	}

	return g_singleton;
}
