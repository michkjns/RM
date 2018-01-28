
#pragma once

class Shader;
class TileMap;

class TileRenderer
{
public:
	TileRenderer();
	~TileRenderer();

	bool initialize();

	void render(const TileMap& tileMap, const glm::mat4& projectionMatrix);

private:
	GLuint  m_VAO;
	GLuint  m_VBO;

	std::string m_tilemap;
	Shader*     m_tileShader;
};