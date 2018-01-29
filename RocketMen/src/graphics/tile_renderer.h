
#pragma once

class Shader;
class Tilemap;

class TileRenderer
{
public:
	TileRenderer();
	~TileRenderer();

	bool initialize();

	void render(const Tilemap& tileMap, const glm::mat4& projectionMatrix);

private:
	GLuint  m_VAO;
	GLuint  m_VBO;

	std::string m_tilemap;
};