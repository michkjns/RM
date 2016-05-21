
#include <includes.h>

#include <graphics/tile_map.h>

#include <graphics/texture.h>
#include <core/resource_manager.h>

#include <assert.h>

TileMap::TileMap() :
	m_tileSheet(""),
	m_map(nullptr),
	m_name(""),
	m_mapWidth(0),
	m_mapHeight(0),
	m_sheetWidth(0),
	m_sheetHeight(0),
	m_tileSize(0),
	m_isInitialized(false)
{
}

bool TileMap::initialize(std::string tileSheet,
				 char*       map,
				 const char* name,
				 uint32_t    mapWidth,
				 uint32_t    mapHeight,
				 uint32_t    sheetWidth,
				 uint32_t    sheetHeight,
				 uint32_t    tileSize)
{
	if (map == nullptr || name == nullptr)
	{
		LOG_ERROR("TileMap::initialize: Invalid arguments.");
		assert(map != nullptr); assert(name != nullptr);
		return false;
	}
	const uint32_t numTiles = mapWidth * mapHeight;
	m_tileSheet   = tileSheet;
	m_map         = new char[numTiles];
	memcpy_s(m_map, numTiles, map, numTiles);
	
	m_name        = name;
	m_mapWidth    = mapWidth;
	m_mapHeight   = mapHeight;
	m_sheetWidth  = sheetWidth;
	m_sheetHeight = sheetHeight;
	m_tileSize    = tileSize;

	const uint32_t imgWidth  = mapWidth  * tileSize;
	const uint32_t imgHeight = mapHeight * tileSize;

	uint32_t* image = new uint32_t[imgWidth * imgHeight];
	for (uint32_t y = 0; y < mapHeight; y++)
	{
		for (uint32_t x = 0; x < mapWidth; x++)
		{
			image[x + y * imgWidth] = map[x + y * mapWidth] - '0' ;
		}
	}

 	ResourceManager::createTexture(image, imgWidth, imgHeight, name);
	delete[] image;
	m_isInitialized = true;
}

TileMap::~TileMap()
{

}

std::string TileMap::getName() const
{
	return m_name;
}

uint32_t TileMap::getTileSize() const
{
	return m_tileSize;
}

uint32_t TileMap::getMapWidth() const
{
	return m_mapWidth;
}

uint32_t TileMap::getMapHeight() const
{
	return m_mapHeight;
}

uint32_t TileMap::getSheetWidth() const
{
	return m_sheetWidth;
}

uint32_t TileMap::getSheetHeight() const
{
	return m_sheetHeight;
}

char* TileMap::getMap() const
{
	return m_map;
}

bool TileMap::isInitalized() const
{
	return m_isInitialized;
}

std::string TileMap::getTileSheetName() const
{
	return m_tileSheet;
}

void TileMap::destroy()
{
	delete[] m_map;
}
