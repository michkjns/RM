
#include "includes.h"
#include "resourcemanager.h"

#include <assert.h>
#include <iostream>
#include <fstream>

//#define GLFW_INCLUDE_GLCOREARB

#include <GLFW/glfw3.h>

std::map<std::string, Shader> ResourceManager::m_shaders;
std::map<std::string, Texture> ResourceManager::m_textures;

Shader ResourceManager::LoadShader(const char* vertexShaderFile, const char* fragmentShaderFile, const char* name)
{
	LOG_INFO("ResourceManager: Loading shader %s", name);
	std::string vertexShaderSource = loadShaderFromFile(vertexShaderFile);
	std::string fragmentShaderSource = loadShaderFromFile(fragmentShaderFile);

	Shader shader = Shader();
	LOG_INFO("ResourceManager: Compiling shader %s", name);
	if (!shader.compile(vertexShaderSource, fragmentShaderSource))
	{
		LOG_ERROR("ResourceManager: Shader compilation failed");
		return shader;
	}

	m_shaders[name] = shader;

	return shader;
}

Shader& ResourceManager::GetShader(std::string name)
{
	return m_shaders[name];
}

Texture ResourceManager::LoadTexture(const char* file, bool alpha)
{
	LOG_INFO("ResourceManager: Loading texture %s", file);
	return Texture();
}

Texture& ResourceManager::GetTexture(std::string name)
{
	return m_textures[name];
}

void ResourceManager::Clear()
{
	m_shaders.clear();
	m_textures.clear();
}

std::string ResourceManager::loadShaderFromFile(const char* shaderFile)
{
	std::ifstream ifs;

	/* Load the shader. */
	ifs.open(shaderFile);

	if (!ifs)
	{
		ifs.open((std::string("../") + std::string(shaderFile)).c_str());
		if (!ifs)
		{
			LOG_ERROR("Shader: Failed to open file: %s", shaderFile);
			return std::string();
		}
	}

	std::string source(std::istreambuf_iterator<char>(ifs), (std::istreambuf_iterator<char>()));
	ifs.close();


	return source;
}