
#pragma once

#include <graphics/texture.h>
#include <graphics/shader.h>
#include <graphics/tilemap.h>

#include <map>

class ResourceManager
{
public:
	static Shader* loadShader(const char* vertexShaderFile,
	                          const char* fragmentShaderFile,
	                          const char* name);

	static Shader* getShader(const std::string& name);

	static Texture* createTexture(const void* imageData,
	                              uint32_t    width, 
	                              uint32_t    height,
	                              const char* name);

	static Texture* loadTexture(const char* file,
								const char* name);

	static Texture* getTexture(const std::string& name);

	static Tilemap* loadTilemap(const char* file,
	                            const char* sheetName,
	                            const char* name);

	static Tilemap* getTileMap(const char* name);

	static void clearTileMaps();
	static void clearTextures();
	static void clearShaders();
	static void clear();

	static bool fileExists(const std::string& name)
	{
		FILE* file;
		fopen_s(&file, name.c_str(), "r");
		if (file)
		{
			fclose(file);
			return true;
		}

		return false;
	}

	static std::string getFileExtension(const std::string& filename)
	{
		if (filename.find_last_of(".") != std::string::npos)
			return filename.substr(filename.find_last_of(".") + 1);

		return "";
	}

private:
	ResourceManager() {}

	static std::string readFileToString(const char* shaderFile);
	
	static std::map<std::string, Shader*>  m_shaders;
	static std::map<std::string, Texture*> m_textures;
	static std::map<std::string, Tilemap*> m_tileMaps;
};