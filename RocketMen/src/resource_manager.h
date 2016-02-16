
#pragma once

#include "texture.h"
#include "shader.h"

#include <map>

class ResourceManager
{
public:
	static Shader& loadShader(const char* vertexShaderFile
							 , const char* fragmentShaderFile
							 , const char* name);

	static Shader& getShader(std::string name);

	static Texture& loadTexture(const char* file, std::string name
								, Texture::EBlendMode blendMode 
									= Texture::EBlendMode::MODE_OPAQUE);

	static Texture& getTexture(std::string name);

	static void clear();

private:
	ResourceManager() {}

	static std::string loadShaderFromFile(const char* shaderFile);
	
	static std::map<std::string, Shader>	m_shaders;
	static std::map<std::string, Texture>	m_textures;
};