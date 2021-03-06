
#include <common.h>
#include <graphics/sprite_renderer.h>

#include <graphics/check_gl_error.h>
#include <core/resource_manager.h>

SpriteRenderer::SpriteRenderer()
{
	ASSERT(initialize(), "Unexpected error initializing SpriteRenderer");
}

SpriteRenderer::~SpriteRenderer()
{
	glDeleteVertexArrays(1, &m_VAO);
}

bool SpriteRenderer::initialize()
{
	/* Configure VAO/VBO */
	const GLfloat vertices[] = {
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

	checkGL();

	return true;
}

void SpriteRenderer::render(const glm::mat4& modelMatrix, 
	const glm::mat4& projectionMatrix, 
	const std::string& texture)
{
	if (Shader* shader = ResourceManager::getShader("sprite_shader"))
	{
		checkGL();

		shader->use();
		shader->setMatrix4("model", modelMatrix);
		shader->setMatrix4("projection", projectionMatrix);
		shader->setVec3f("spriteColor", glm::vec3(1.0f));

		glActiveTexture(GL_TEXTURE0);
		ResourceManager::getTexture(texture)->bind();
		shader->setInt("image", 0);
		checkGL();

		glBindVertexArray(m_VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		checkGL();
	}
}
