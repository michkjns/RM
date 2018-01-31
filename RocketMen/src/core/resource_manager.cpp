
#include <core/debug.h>
#include <core/resource_manager.h>
#include <graphics/check_gl_error.h>
#include <graphics/renderer.h>
#include <utility/utility.h>

#include <array>
#include <assert.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>

#include <GLFW/glfw3.h>
#include <SOIL.h>

std::map<std::string, Shader*>	ResourceManager::m_shaders;
std::map<std::string, Texture*>	ResourceManager::m_textures;
std::map<std::string, Tilemap*> ResourceManager::m_tilemaps;

Shader* ResourceManager::loadShader(const char* vertexShaderFilePath,
									const char* fragmentShaderFilePath, 
									const char* name)
{
	assert(Renderer::get() != nullptr);

	if (Shader* existingShader = getShader(name))
	{
		LOG_WARNING("ResourceManager::loadShader: a shader named %s already exists, loading skipped");
		return existingShader;
	}

	LOG_DEBUG("ResourceManager: Loading shader %s", name);
	std::string vertexShaderSource   = readFileToString(vertexShaderFilePath);
	std::string fragmentShaderSource = readFileToString(fragmentShaderFilePath);

	Shader* shader = new Shader();

	LOG_DEBUG("ResourceManager: Compiling shader %s", name);
	if (shader->compile(vertexShaderSource, fragmentShaderSource))
	{
		checkGL();
		m_shaders[name] = shader;
		return m_shaders[name];
	}

	LOG_ERROR("ResourceManager: Shader compilation failed");
	return nullptr;
}

Shader* ResourceManager::getShader(const std::string& name)
{
	auto shaderEntry = m_shaders.find(name);
	if (shaderEntry != m_shaders.end())
	{
		assert(shaderEntry->second != nullptr);
		return shaderEntry->second;
	}

	return nullptr;
}

Texture* ResourceManager::createTexture(const void* imageData, 
	                                    uint32_t    width,
	                                    uint32_t    height,
	                                    const char* name)
{
	assert(Renderer::get() != nullptr);
	assert(imageData != nullptr);
	assert(width > 0);
	assert(height > 0);
	assert(name != nullptr);

	if (Texture* existingTexture = getTexture(name))
	{
		LOG_WARNING("ResourceManager::loadTexture: a texture named %s already exists, loading skipped");
		return existingTexture;
	}

	LOG_INFO("ResourceManager: Creating texture %s", name);
	Texture* texture = new Texture(imageData, width, height);
	checkGL();

	m_textures[name] = texture;
	return texture;
}

Texture* ResourceManager::loadTexture(const char* filename, 
									  const char* name)
{
	assert(Renderer::get() != nullptr);
	assert(filename != nullptr);
	assert(name != nullptr);

	if (Texture* existingTexture = getTexture(name))
	{
		LOG_WARNING("ResourceManager::loadTexture: a texture named %s already exists, loading skipped");
		return existingTexture;
	}

	LOG_DEBUG("ResourceManager: Loading texture %s", filename);
	const std::array<const char*, 5> allowedExtensions = { "bmp", "tga", "dds", "png", "jpg" };
	
	if ( std::find(allowedExtensions.begin(), allowedExtensions.end(), toLower(getFileExtension(filename))) == allowedExtensions.end())
	{
		LOG_ERROR("ResourceManager::loadTilemap: unsupported file extension: %s", filename);
		return nullptr;
	}

	int width, height;
	unsigned char* imageData = SOIL_load_image( (std::string("../") + std::string(filename)).c_str(), 
											   &width, &height, 0, SOIL_LOAD_RGBA );
	if (imageData != nullptr)
	{
		Texture* result = createTexture(imageData, width, height, name);
		SOIL_free_image_data(imageData);
		return result;
	}

	LOG_ERROR("ResourceManager::LoadTexture: Failed to load texture data %s", filename);
	return nullptr;
}

Texture* ResourceManager::getTexture(const std::string& name)
{
	auto textureEntry = m_textures.find(name);
	if (textureEntry != m_textures.end())
	{
		assert(textureEntry->second != nullptr);
		return textureEntry->second;
	}

	return nullptr;
}

Tilemap* ResourceManager::loadTilemap(const char* filename, 
									  const char* sheetName, 
									  const char* name)
{
	assert(filename != nullptr);
	assert(sheetName != nullptr);
	assert(name != nullptr);

	if (getFileExtension(filename) != "csv")
	{
		LOG_ERROR("ResourceManager::loadTilemap: unsupported file extension: %s", filename);
		return nullptr;
	}

	std::string tileData = readFileToString(filename);
	assert(tileData.size() > 0);

	const Vector2i dimensions = Tilemap::getDimensionsFromFilename(filename);
	if (dimensions.x > 0 && dimensions.y > 0)
	{
		const TilemapParam param =
		{
			name,
			tileData,
			dimensions,
			16,
			sheetName
		};

		Tilemap* tilemap = new Tilemap(param);
		if (tilemap->getMap() != nullptr)
		{
			m_tilemaps[name] = tilemap;
			return tilemap;
		}
		else
		{
			LOG_ERROR("ResourceManager::loadTilemap: an unexpected error occured initializing %s", filename);
			delete tilemap;
			return nullptr;
		}
	}

	LOG_ERROR("ResourceManager::loadTilemap: Error reading tilemap dimensions from %s", filename);
	return nullptr;
}

Tilemap* ResourceManager::getTileMap(const char* name)
{
	return m_tilemaps[name];
}

void ResourceManager::clearTileMaps()
{
	for (auto tm : m_tilemaps)
	{
		delete tm.second;
	}

	m_tilemaps.clear();
}

void ResourceManager::clearTextures()
{
	for (auto textureIter : m_textures)
	{
		textureIter.second->destroy();
		delete textureIter.second;
	}

	m_textures.clear();
}

void ResourceManager::clearShaders()
{
	for (auto shaderIter : m_shaders)
	{
		shaderIter.second->destroy();
		delete shaderIter.second;
	}

	m_shaders.clear();
}

void ResourceManager::clear()
{
	clearTextures();
	clearShaders();
	clearTileMaps();
}

std::string ResourceManager::readFileToString(const char* filePath)
{
	std::ifstream filestream;

	filestream.open(filePath);

	if (!filestream)
	{
		filestream.open((std::string("../") + std::string(filePath)).c_str());
		if (!filestream)
		{
			LOG_ERROR("File not found: %s", filePath);
			return std::string();
		}
	}

	std::string fileContent(std::istreambuf_iterator<char>(filestream), (std::istreambuf_iterator<char>()));
	filestream.close();

	if (fileContent.back() == '\n')
	{
		fileContent.pop_back();
	}

	return fileContent;
}