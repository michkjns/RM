
#include <includes.h>
#include <graphics/renderer.h>

#include <core/entity.h>
#include <core/resource_manager.h>
#include <core/window.h>
#include <graphics/camera.h>
#include <graphics/check_gl_error.h>
#include <graphics/sprite_renderer.h>
#include <graphics/tile_renderer.h>
#include <physics.h>
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
	Vector2 Renderer_impl::getScreenSize() const;

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
	//renderUI();
}

void Renderer_impl::renderSprites()
{
	for (const auto& it : Entity::getList())
	{
		if (it->getSpriteName().empty())
			continue;

		if (Camera::mainCamera)
		{
			Texture& texture = ResourceManager::getTexture(it->getSpriteName());

			glm::mat4 origin = glm::translate(it->getTransform().getWorldMatrix(), Vector3(-.5f, -.50f, 0.0f));

			m_spriteRenderer.render(origin,
									Camera::mainCamera->getProjectionMatrix()
									* Camera::mainCamera->getViewMatrix(),
									it->getSpriteName());
		}
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

void Renderer::drawPolygon(const Vector2* vertices, int32_t vertexCount, const Color& color, bool screenSpace)
{
	Shader::unbindShader();
	GLfloat glverts[16];
	glVertexPointer(2, GL_FLOAT, 0, glverts);
	glEnableClientState(GL_VERTEX_ARRAY);

	for (int i = 0; i < vertexCount; i++) 
	{
		Vector2 view = Renderer::get()->getScreenSize();
		Vector4 xy = (screenSpace) ? Vector4(vertices[i], 0.0f, 0.0f)
			: Camera::mainCamera->getProjectionMatrix() * Camera::mainCamera->getViewMatrix() *
			Vector4(vertices[i], 1.0f, 1.0f);

		glverts[i * 2]     = xy.x;
		glverts[i * 2 + 1] = xy.y;
	}

	glColor4f(color.r, color.g, color.b, 0.3f);
	glDrawArrays(GL_TRIANGLE_FAN, 0, vertexCount);

	glLineWidth(1);
	glColor4f(1, 0, 1, 1);
	glDrawArrays(GL_LINE_LOOP, 0, vertexCount);
	glDisableClientState(GL_VERTEX_ARRAY);
}

Vector2 Renderer_impl::getScreenSize() const
{ 
	return Vector2(m_window->getWidth(), m_window->getHeight());
}

Renderer* Renderer::get()
{
	if (g_singleton == nullptr)
	{
		g_singleton = new Renderer_impl();
	}

	return g_singleton;
}
