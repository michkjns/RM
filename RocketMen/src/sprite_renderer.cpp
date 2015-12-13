
#include "includes.h"

#include "sprite_renderer.h"
#include "resource_manager.h"

SpriteRenderer::SpriteRenderer()
: m_spriteShader(nullptr)
{

}

SpriteRenderer::~SpriteRenderer()
{
	glDeleteVertexArrays(1, &m_VAO);
}

void SpriteRenderer::initialize()
{
	/* Configure VAO/VBO */

	GLfloat vertices[] = {
		// Pos      // Tex
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f
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

	m_spriteShader = &ResourceManager::GetShader("spriteShader");
}

void SpriteRenderer::render(const glm::mat4& modelMatrix)
{
	assert(m_spriteShader);
	if (m_spriteShader != nullptr)
	{
		m_spriteShader->use();
		m_spriteShader->setMatrix4("model", modelMatrix);

		glActiveTexture(GL_TEXTURE0);
		//texture.bind();

		glBindVertexArray(m_VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}
}
