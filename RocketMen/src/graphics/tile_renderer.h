
#pragma once

#include <graphics/tile_map.h>

//namespace graphics {
	class Shader;
//};//
class TileRenderer
{
public:
	TileRenderer();
	~TileRenderer();

	bool initialize();

	void render(TileMap* tileMap,
				const glm::mat4& projectionMatrix);

private:
	GLuint  m_VAO;
	GLuint  m_VBO;
	GLfloat m_vertices;

	std::string m_tilemap;
	Shader*     m_tileShader;
};