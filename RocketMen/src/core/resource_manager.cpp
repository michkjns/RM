
#include <core/debug.h>
#include <core/resource_manager.h>

#include <graphics/check_gl_error.h>
#include <graphics/renderer.h>

#include <assert.h>
#include <iostream>
#include <fstream>
#include <vector>

#include <GLFW/glfw3.h>
#include <SOIL.h>

std::map<std::string, Shader>	ResourceManager::m_shaders;
std::map<std::string, Texture>	ResourceManager::m_textures;
std::map<std::string, TileMap>  ResourceManager::m_tileMaps;

static Shader  dummyShader;
static Texture dummyTexture;

Shader& ResourceManager::loadShader(const char* vertexShaderFilePath, 
									const char* fragmentShaderFilePath, 
									const char* name)
{
	if (Renderer::get() == nullptr)
	{
		return dummyShader;
	}

	LOG_DEBUG("ResourceManager: Loading shader %s", name);

	std::string vertexShaderSource   = readFileToString(vertexShaderFilePath);
	std::string fragmentShaderSource = readFileToString(fragmentShaderFilePath);

	Shader shader;
	LOG_DEBUG("ResourceManager: Compiling shader %s", name);
	if (!shader.compile(vertexShaderSource, fragmentShaderSource))
	{
		LOG_ERROR("ResourceManager: Shader compilation failed");
		return dummyShader;
	}

	checkGL();
	m_shaders[name] = shader;

	return m_shaders[name];
}

Shader& ResourceManager::getShader(const std::string& name)
{
	return m_shaders[name];
}

Texture& ResourceManager::createTexture(const void* imageData, 
	                                    uint32_t    width,
	                                    uint32_t    height,
	                                    const char* name)
{
	assert(Renderer::get() != nullptr);

	LOG_INFO("ResourceManager: Creating texture %s", name);
	Texture texture;
	texture.generate(imageData, width, height);
	m_textures[name] = texture;

	return m_textures[name];
}

Texture& ResourceManager::loadTexture(const char* file, 
									  const char* name)
{
	assert(Renderer::get() != nullptr);

	LOG_DEBUG("ResourceManager: Loading texture %s", file);
	int width, height;
	unsigned char* imageData = SOIL_load_image( (std::string("../") + std::string(file)).c_str(), 
											   &width, &height, 0, SOIL_LOAD_RGBA );
	assert(imageData);
	Texture texture;
	if (imageData == nullptr)
	{
		LOG_ERROR("ResourceManager::LoadTexture: Failed to open texture %s", file);
		return dummyTexture;
	}
	else
	{
		texture.generate(imageData, width, height);
		checkGL();
		SOIL_free_image_data(imageData);
		m_textures[name] = texture;
	}
	return m_textures[name];
}

Texture& ResourceManager::getTexture(const std::string& name)
{
	return m_textures[name];
}

TileMap& ResourceManager::loadTilemap(const char* file, 
									  const char* sheetName, 
									  const char* name)
{
	assert(file != nullptr);
	assert(sheetName != nullptr);
	assert(name != nullptr);

	std::ifstream ifs;

	ifs.open(file);

	if (!ifs)
	{
		ifs.open((std::string("../") + std::string(file)).c_str());
		if (!ifs)
		{
			LOG_ERROR("TileMap: Failed to open file: %s", file);
			static TileMap dummy;
			return dummy;
		}
	}

	std::vector<char> mapInput;
	char inChar;
	while (ifs.good())
	{
		ifs >> inChar;
		ifs.ignore();
		mapInput.push_back(inChar);
	}
	ifs.close();
	mapInput.erase(mapInput.end()-1);

	assert(mapInput.size() > 0);

	std::string widthHeight(file);
	int32_t dotFirst = static_cast<int32_t>(widthHeight.find_first_of('.', 0));
	int32_t dotLast  = static_cast<int32_t>(widthHeight.find_first_of('.', dotFirst+1));
	widthHeight = widthHeight.substr(dotFirst, dotLast);
	std::string widthStr  = widthHeight.substr(1, widthHeight.find('x') - 1);
	std::string heightStr = widthHeight.substr(widthHeight.find('x') + 1, widthHeight.find_last_of('.') - 3);
	const uint32_t mapWidth = atoi(widthStr.c_str());
	const uint32_t mapHeight = atoi(heightStr.c_str());

	TileMap tileMap;
	const TileMapParam param = 
	{ 
		name,
		sheetName,
		mapInput.data(),
		mapWidth,
		mapHeight,
		2, 2,
		16 
	};

	assert(tileMap.initialize(param));

	m_tileMaps[name] = tileMap;
	return m_tileMaps[name];
}

TileMap& ResourceManager::getTileMap(const char* name)
{
	return m_tileMaps[name];
}

void ResourceManager::clearTileMaps()
{
	for (auto tm : m_tileMaps)
	{
		tm.second.destroy();
	}
	m_tileMaps.clear();
}

void ResourceManager::clearTextures()
{
	for (auto t : m_textures)
	{
		t.second.destroy();
	}
	m_textures.clear();
}

void ResourceManager::clearShaders()
{
	for (auto s : m_shaders)
	{
		s.second.destroy();
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
	std::ifstream ifs;

	ifs.open(filePath);

	if (!ifs)
	{
		ifs.open((std::string("../") + std::string(filePath)).c_str());
		if (!ifs)
		{
			LOG_ERROR("Shader: Failed to open file: %s", filePath);
			return std::string();
		}
	}

	std::string source(std::istreambuf_iterator<char>(ifs), (std::istreambuf_iterator<char>()));
	ifs.close();

	return source;
}