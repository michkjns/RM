
#include "includes.h"
#include "resource_manager.h"

#include "check_gl_error.h"

#include <assert.h>
#include <iostream>
#include <fstream>

#include <GLFW/glfw3.h>
#include <SOIL.h>

std::map<std::string, Shader> ResourceManager::m_shaders;
std::map<std::string, Texture> ResourceManager::m_textures;

Shader dummyShader;

Shader& ResourceManager::LoadShader(const char* vertexShaderFile, const char* fragmentShaderFile, const char* name)
{
	LOG_INFO("ResourceManager: Loading shader %s", name);

	std::string vertexShaderSource = loadShaderFromFile(vertexShaderFile);
	std::string fragmentShaderSource = loadShaderFromFile(fragmentShaderFile);

	Shader shader = Shader();
	LOG_INFO("ResourceManager: Compiling shader %s", name);
	if (!shader.compile(vertexShaderSource, fragmentShaderSource))
	{
		LOG_ERROR("ResourceManager: Shader compilation failed");
		return dummyShader;
	}

	checkGL();
	m_shaders[name] = shader;
	
	shader.clear();
	return m_shaders[name];
}

Shader& ResourceManager::GetShader(std::string name)
{
	return m_shaders[name];
}

Texture& ResourceManager::LoadTexture(const char* file, std::string name
									 , bool alpha)
{
	LOG_INFO("ResourceManager: Loading texture %s", file);
	int width, height;
	unsigned char* imageData = SOIL_load_image((std::string("../") + std::string(file)).c_str(), &width, &height, 0, SOIL_LOAD_RGB);
	assert(imageData);
	Texture texture;
	if (imageData == nullptr)
	{
		LOG_ERROR("ResourceManager::LoadTexture: Failed to open texture %s", file);
	}
	else
	{
		checkGL();
		texture.generate(imageData, width, height);
		checkGL();
		SOIL_free_image_data(imageData);
		m_textures[name] = texture;
	}
	return m_textures[name];
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