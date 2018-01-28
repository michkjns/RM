
#include <graphics/renderer.h>

#include <core/debug.h>
#include <core/entity.h>
#include <core/entity_manager.h>
#include <core/game.h>
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
	Renderer_glfw() {}
	~Renderer_glfw() {};

	bool initialize() override;
	void initializeGLBuffers();
	void destroy() override {};

	void render(const RenderContext& context, bool debugDrawEnabled) override;

	void renderDebug(const RenderContext& context);

	void clearScreenBuffers();

	void drawPolygon(const Vector2* vertices,
	                 int32_t vertexCount,
	                 const Color& color,
	                 bool screenSpace = false) override;

	void drawLineSegment(const LineSegment& segment, const Color& color) override;

private:
	void renderSprites(const Camera& camera);

	SpriteRenderer m_spriteRenderer;
	TileRenderer   m_tileRenderer;
	GLuint         m_lineVAO;
	GLuint         m_lineVBO;
};

bool Renderer_glfw::initialize()
{
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

	initializeGLBuffers();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return true;
}

void Renderer_glfw::initializeGLBuffers()
{
	const float lineVerts[] = {
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
}

void Renderer_glfw::render(const RenderContext& context, bool enableDebugRendering)
{
	clearScreenBuffers();

	if (Camera* camera = context.game.getMainCamera())
	{
		m_currentCamera = camera;
		camera->updateViewMatrix();

		m_tileRenderer.render(ResourceManager::getTileMap("testmap"),
			camera->getProjectionMatrix() * camera->getViewMatrix());

		renderSprites(*camera);

		if (enableDebugRendering)
		{
			renderDebug(context);
		}

		m_currentCamera = nullptr;
	}
}

void Renderer_glfw::renderDebug(const RenderContext& context)
{
	for (auto& it : EntityManager::getEntities())
	{
		it->debugDraw();
	}

	if (context.physics != nullptr)
	{
		context.physics->drawDebug();
	}
}

void Renderer_glfw::clearScreenBuffers()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer_glfw::renderSprites(const Camera& camera)
{
	for (const auto& it : EntityManager::getEntities())
	{
		if (!it->getSpriteName().empty())
		{
			glm::mat4 origin = glm::translate(it->getTransform().getWorldMatrix(), Vector3(-.5f, -.50f, 0.0f));

			m_spriteRenderer.render(origin,
				camera.getProjectionMatrix() * camera.getViewMatrix(),
				it->getSpriteName());
		}
	}
}

void Renderer_glfw::drawPolygon(const Vector2* vertices, int32_t vertexCount, const Color& color, bool inScreenSpace)
{
	assert(m_currentCamera != nullptr);

	Shader::unbindCurrentShader();

	GLfloat glverts[16];
	glVertexPointer(2, GL_FLOAT, 0, glverts);
	glEnableClientState(GL_VERTEX_ARRAY);

	for (int i = 0; i < vertexCount; i++) 
	{
		Vector4 xy = (inScreenSpace) ? Vector4(vertices[i], 0.0f, 0.0f)
			: m_currentCamera->getProjectionMatrix() * m_currentCamera->getViewMatrix()	* Vector4(vertices[i], 1.0f, 1.0f);

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

void Renderer_glfw::drawLineSegment(const LineSegment& segment, const Color& color)
{
	assert(m_currentCamera != nullptr);

	Shader& lineShader = ResourceManager::getShader("line_shader");
	lineShader.use();
	
	glm::mat4 projectionMatrix = (segment.renderSpace == RenderSpace::ScreenSpace) ? glm::mat4()
	                            : m_currentCamera->getProjectionMatrix()
	                              * m_currentCamera->getViewMatrix();
	                                          
	glm::mat4 modelMatrix(1.f);
	modelMatrix = glm::translate(glm::mat4(), Vector3(segment.pointA, 0.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(segment.pointB - segment.pointA, 1.f));

	lineShader.setMatrix4("model", modelMatrix);
	lineShader.setMatrix4("projection", projectionMatrix);
	lineShader.setVec4f("lineColor", color);

	glBindVertexArray(m_lineVAO);
	glDrawArrays(GL_LINES, 0, 2);
	glBindVertexArray(0);

	checkGL();
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
