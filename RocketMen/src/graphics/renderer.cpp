
#include <graphics/renderer.h>

#include <core/debug.h>
#include <core/entity.h>
#include <core/entity_manager.h>
#include <core/resource_manager.h>
#include <core/window.h>
#include <graphics/camera.h>
#include <graphics/check_gl_error.h>
#include <graphics/sprite_renderer.h>
#include <graphics/tile_renderer.h>
#include <physics/physics.h>

#include <GLFW/glfw3.h>

#include <fstream>

static Renderer* s_renderer;

class Renderer_glfw : public Renderer
{
public:
	Renderer_glfw();
	~Renderer_glfw();

	bool initialize(Window* window) override;
	void destroy() override;

	void render() override;
	void drawPolygon(const Vector2* vertices,
	                 int32_t vertexCount,
	                 const Color& color,
	                 bool screenSpace = false) override;

	void drawLineSegment(const Vector2& p1, const Vector2& p2,
		                 const Color& color, bool screenSpace) override;

	Vector2i Renderer_glfw::getScreenSize() const;

private:
	void renderSprites();
	void renderTiles();
	void renderUI();

	SpriteRenderer m_spriteRenderer;
	TileRenderer   m_tileRenderer;
	Window*        m_window;
	GLuint         m_lineVAO;
	GLuint         m_lineVBO;
};

Renderer_glfw::Renderer_glfw()
	: m_window(nullptr)
{
}

Renderer_glfw::~Renderer_glfw()
{
}

bool Renderer_glfw::initialize(Window* window)
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

	float lineVerts[] = {
		1.f, 1.f,
		0.f, 0.f
	};

	glGenVertexArrays(1, &m_lineVAO);
	glGenBuffers(1, &m_lineVBO);

	glBindBuffer(GL_ARRAY_BUFFER, m_lineVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lineVerts), lineVerts, GL_STATIC_DRAW);

	glBindVertexArray(m_lineVAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return true;
}

void Renderer_glfw::destroy()
{
}

void Renderer_glfw::render()
{
	if (Camera::mainCamera) 
	{
		Camera::mainCamera->updateViewMatrix();	

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		renderTiles();
		renderSprites();
	}
}

void Renderer_glfw::renderSprites()
{
	for (const auto& it : EntityManager::getEntities())
	{
		if (!it->getSpriteName().empty())
		{
			glm::mat4 origin = glm::translate(it->getTransform().getWorldMatrix(), Vector3(-.5f, -.50f, 0.0f));

			m_spriteRenderer.render(origin,
				Camera::mainCamera->getProjectionMatrix() * Camera::mainCamera->getViewMatrix(),
				it->getSpriteName());
		}
	}
}

void Renderer_glfw::renderTiles()
{
	m_tileRenderer.render(&ResourceManager::getTileMap("testmap"), 
                          Camera::mainCamera->getProjectionMatrix()
                          * Camera::mainCamera->getViewMatrix());
}

void Renderer_glfw::renderUI()
{
	// TODO implement renderUI
}

void Renderer_glfw::drawPolygon(const Vector2* vertices, int32_t vertexCount, const Color& color, 
                                bool screenSpace)
{
	Shader::unbindShader();
	GLfloat glverts[16];
	glVertexPointer(2, GL_FLOAT, 0, glverts);
	glEnableClientState(GL_VERTEX_ARRAY);

	for (int i = 0; i < vertexCount; i++) 
	{
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

void Renderer_glfw::drawLineSegment(const Vector2& p1, const Vector2& p2, const Color& color, 
	                                bool screenSpace)
{
	assert(Camera::mainCamera != nullptr);

	Shader& lineShader = ResourceManager::getShader("line_shader");
	lineShader.use();
	
	glm::mat4 projectionMatrix = (screenSpace) ? glm::mat4()
	                            : Camera::mainCamera->getProjectionMatrix()
	                              * Camera::mainCamera->getViewMatrix();
	                                          
	glm::mat4 modelMatrix(1.f);
	modelMatrix = glm::translate(glm::mat4(), Vector3(p1, 0.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(p2-p1, 1.f));

	lineShader.setMatrix4("model", modelMatrix);
	lineShader.setMatrix4("projection", projectionMatrix);
	lineShader.setVec4f("lineColor", color);

	glBindVertexArray(m_lineVAO);
	glDrawArrays(GL_LINES, 0, 2);
	glBindVertexArray(0);

	checkGL();
}

Vector2i Renderer_glfw::getScreenSize() const
{ 
	return m_window->getSize();
}

Renderer* Renderer::create()
{
	if (s_renderer == nullptr)
	{
		s_renderer = new Renderer_glfw();
	}

	return s_renderer;
}

Renderer* Renderer::get()
{
	return s_renderer;
}
