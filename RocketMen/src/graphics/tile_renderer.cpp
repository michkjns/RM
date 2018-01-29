
#include <common.h>

#include <graphics/tile_renderer.h>
#include <graphics/texture.h>

#include <graphics/check_gl_error.h>
#include <core/resource_manager.h>

TileRenderer::TileRenderer() :
	m_VAO(0),
	m_VBO(0)
{

}

TileRenderer::~TileRenderer()
{
	glDeleteVertexArrays(1, &m_VAO);
}

bool TileRenderer::initialize()
{
	const GLfloat vertices[] = {
		// Pos        // Tex
		-1.0f,  1.0f, 0.0f, 1.0f,
		 1.0f, -1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f, 0.0f,

		-1.0f,  1.0f, 0.0f, 1.0f,
		 1.0f,  1.0f, 1.0f, 1.0f,
		 1.0f, -1.0f, 1.0f, 0.0f
	};

	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(m_VAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return true;
}

void TileRenderer::render(const Tilemap& tileMap, const glm::mat4& projectionViewMatrix)
{
	if (Shader* shader = ResourceManager::getShader("tile_shader"))
	{
		checkGL();
		const glm::vec2 size(tileMap.getMapWidth() / 2.f,
			tileMap.getMapHeight() / 2.f);

		const glm::mat4 modelMatrix = glm::scale(glm::mat4(), glm::vec3(size, -1.0f));

		shader->use();
		shader->setMatrix4("model", modelMatrix);
		shader->setMatrix4("projection", projectionViewMatrix);

		shader->setVec4f("map_info", glm::vec4(tileMap.getTileSize(),
			tileMap.getMapWidth(),
			tileMap.getMapHeight(),
			0.0f));

		glActiveTexture(GL_TEXTURE0);
		ResourceManager::getTexture(tileMap.getName())->bind();
		shader->setInt("tile_map", 0);

		glActiveTexture(GL_TEXTURE1);
		ResourceManager::getTexture(tileMap.getTileSheetName())->bind();
		shader->setInt("tile_image", 1);
		checkGL();

		glBindVertexArray(m_VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		checkGL();
	}
}
