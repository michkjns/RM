#pragma once

class Shader;
class SpriteRenderer
{
public:
	SpriteRenderer();
	~SpriteRenderer();

	bool initialize();

	void render(const glm::mat4& modelMatrix
				, const glm::mat4& projectionMatrix);

private:
	GLuint m_VAO;
	GLuint m_VBO;
	GLfloat m_vertices;

	Shader* m_spriteShader;
};
