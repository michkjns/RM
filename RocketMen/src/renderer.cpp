
#include "includes.h"
#include "renderer.h"
#include "sprite_renderer.h"
#include "window.h"

#include <GLFW/glfw3.h>

Renderer* Renderer::g_singleton;

class Renderer_impl : public Renderer
{
public:
	Renderer_impl();
	~Renderer_impl();

	bool initialize(EProjectionMode projection, Window* window) override;
	void destroy() override;

	void render() override;

private:

	void renderSprites();
	void renderUI();

	SpriteRenderer m_spriteRenderer;
	Window* m_window;
	glm::mat4 BuildProjectionMatrix(EProjectionMode projection) const;
	glm::mat4 m_projectionMatrix;
	EProjectionMode m_EProjectionMode;

};

Renderer_impl::Renderer_impl()
	: m_window(nullptr)
{
}

Renderer_impl::~Renderer_impl()
{
}

bool Renderer_impl::initialize(EProjectionMode projection, Window* window)
{
	assert(window != nullptr);

	m_window = window;
	m_EProjectionMode = projection;
	m_projectionMatrix = BuildProjectionMatrix(m_EProjectionMode);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		LOG_ERROR("Renderer: Failed to initialize GLEW");
		glfwTerminate();
		exit(-1);
	}

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
	{
		LOG_ERROR("Renderer: OpenGL Error: %d", error);
	}

	return true;
}

void Renderer_impl::destroy()
{

}

void Renderer_impl::render()
{
	renderSprites();
	renderUI();
}

void Renderer_impl::renderSprites()
{
	m_spriteRenderer.render(glm::mat4());

	return;
}

void Renderer_impl::renderUI()
{
}

glm::mat4 Renderer_impl::BuildProjectionMatrix(EProjectionMode projection) const
{
	glm::mat4 matrix;
	switch (projection)
	{
		case EProjectionMode::ORTOGRAPHIC_PROJECTION:
		{
			matrix = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f);
			break;
		}
		case EProjectionMode::PERSPECTIVE_PROJECTION:
		{
			// TODO(Support perspective projection)
			assert(false);
			break;
		}
	}
	return matrix;
}

Renderer* Renderer::get()
{
	if (g_singleton == nullptr)
	{
		g_singleton = new Renderer_impl();
	}

	return g_singleton;
}
