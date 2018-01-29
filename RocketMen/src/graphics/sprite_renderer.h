
#pragma once

class Shader;
class SpriteRenderer
{
public:
	SpriteRenderer();
	~SpriteRenderer();


	void render(const glm::mat4& modelMatrix,
				const glm::mat4& projectionMatrix,
				const std::string& texture);

private:
	bool initialize();

	GLuint m_VAO;
	GLuint m_VBO;
};
