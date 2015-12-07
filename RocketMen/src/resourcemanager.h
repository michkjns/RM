
#pragma once

#include "texture.h"
#include "shader.h"

#include <map>

class ResourceManager
{
public:
	static Shader LoadShader(const char* vertexShaderFile
							 , const char* fragmentShaderFile
							 , const char* name);
	static Shader& GetShader(std::string name);

	static Texture LoadTexture(const char* file, bool alpha);
	static Texture& GetTexture(std::string name);

	static void Clear();

private:
	ResourceManager() {}
	
	static std::string loadShaderFromFile(const char* shaderFile);
	
	static std::map<std::string, Shader> m_shaders;
	static std::map<std::string, Texture> m_textures;
};