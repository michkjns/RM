
#pragma once

#include <common.h>

struct TileMapParam
{
	const char* name;
	const char* tileSheetName;
	char*       tiles;
	uint32_t    mapWidth;
	uint32_t    mapHeight;
	uint32_t    sheetWidth;
	uint32_t    sheetHeight;
	uint32_t    tileSize;
};

class TileMap
{
public:
	TileMap();
	~TileMap();

	bool initialize(const TileMapParam& parameters);        

	void createTexture();

	std::string getName()          const { return m_name; }
	uint32_t    getTileSize()      const { return m_tileSize; }
	uint32_t    getMapWidth()      const { return m_mapWidth; }
	uint32_t    getMapHeight()     const { return m_mapHeight; }
	uint32_t    getSheetWidth()    const { return m_sheetWidth; }
	uint32_t    getSheetHeight()   const { return m_sheetHeight; }
	char*       getMap()           const { return m_map; }
	bool        isInitalized()     const { return m_isInitialized; }
	std::string getTileSheetName() const { return m_tileSheetName; }

	void destroy();

private:
	std::string m_name;
	uint32_t    m_tileSize;
	uint32_t    m_mapWidth;
	uint32_t    m_mapHeight;
	uint32_t    m_sheetWidth;
	uint32_t    m_sheetHeight;
	std::string m_tileSheetName;
	char*       m_map;
	bool        m_isInitialized;
};
