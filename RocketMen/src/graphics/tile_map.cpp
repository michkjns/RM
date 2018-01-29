
#include <graphics/tile_map.h>

#include <core/debug.h>
#include <core/resource_manager.h>
#include <graphics/texture.h>

TileMap::TileMap() :
	m_tileSheetName(""),
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

TileMap::~TileMap()
{

}

void TileMap::destroy()
{
	delete[] m_map;
}


bool TileMap::initialize(const TileMapParam& parameters)
{
	if (parameters.tiles == nullptr || parameters.name == nullptr)
	{
       	LOG_ERROR("TileMap::initialize: Invalid arguments.");
		return false;
	}

	const uint32_t numTiles = parameters.mapWidth * parameters.mapHeight;
	m_tileSheetName = parameters.tileSheetName;
	m_map = new char[numTiles];
	memcpy_s(m_map, numTiles, parameters.tiles, numTiles);
	
	m_name        = parameters.name;
	m_mapWidth    = parameters.mapWidth;
	m_mapHeight   = parameters.mapHeight;
	m_sheetWidth  = parameters.sheetWidth;
	m_sheetHeight = parameters.sheetHeight;
	m_tileSize    = parameters.tileSize;

	createTexture();
	
	m_isInitialized = true;
	return true;
}

void TileMap::createTexture()
{
	const uint32_t imageWidth = m_mapWidth * m_tileSize;
	const uint32_t imageHeight = m_mapHeight * m_tileSize;

	uint32_t* imageData = new uint32_t[imageWidth * imageHeight];
	for (uint32_t y = 0; y < m_mapHeight; y++)
	{
		for (uint32_t x = 0; x < m_mapWidth; x++)
		{
			imageData[x + y * imageWidth] = m_map[x + y * m_mapWidth] - '0';
		}
	}

	ResourceManager::createTexture(imageData, imageWidth, imageHeight, m_name.c_str());
	delete[] imageData;
}
