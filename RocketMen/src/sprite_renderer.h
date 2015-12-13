#pragma once

class Shader;
class SpriteRenderer
{
public:
	SpriteRenderer();
	~SpriteRenderer();

	void initialize();

	void render(const glm::mat4& modelMatrix);

private:
	GLuint m_VAO;
	GLuint m_VBO;
	GLfloat m_vertices;

	Shader* m_spriteShader;
};
