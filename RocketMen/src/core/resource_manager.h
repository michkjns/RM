
#pragma once

//#include <common.h>
#include <graphics/texture.h>
#include <graphics/shader.h>
#include <graphics/tile_map.h>

#include <map>

class ResourceManager
{
public:
	static Shader& loadShader(const char* vertexShaderFile,
	                          const char* fragmentShaderFile,
	                          const char* name);

	static Shader& getShader(std::string name);

	static Texture& createTexture(const void*        imageData,
	                              uint32_t           width, 
	                              uint32_t           height,
	                              const char*        name);

	static Texture& loadTexture(const char* file, 
								const char* name);

	static Texture& getTexture(std::string name);

	static TileMap& loadTilemap(const char* file,
	                            const char* sheetName,
	                            const char* name);

	static TileMap& getTileMap(const char* name);

	static void clearTileMaps();
	static void clearTextures();
	static void clearShaders();
	static void clear();

private:
	ResourceManager() {}

	static std::string readFileToString(const char* shaderFile);
	
	static std::map<std::string, Shader>  m_shaders;
	static std::map<std::string, Texture> m_textures;
	static std::map<std::string, TileMap> m_tileMaps;
};