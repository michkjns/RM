
#pragma once

#include <common.h>

class TileMap
{
public:
	TileMap();
	~TileMap();

	bool initialize(const std::string& tileSheet,
					char*       map,
					const char* name,
					uint32_t    mapWidth,
					uint32_t    mapHeight,
					uint32_t    sheetWidth,
					uint32_t    sheetHeight,
					uint32_t    tileSize);

	std::string getName()        const;
	uint32_t    getTileSize()    const;
	uint32_t    getMapWidth()    const;
	uint32_t    getMapHeight()   const;
	uint32_t    getSheetWidth()  const;
	uint32_t    getSheetHeight() const;
	char*       getMap()         const;
	bool        isInitalized()   const;

	std::string getTileSheetName() const;
	void        destroy();

private:
	std::string m_name;
	uint32_t    m_tileSize;
	uint32_t    m_mapWidth;
	uint32_t    m_mapHeight;
	uint32_t    m_sheetWidth;
	uint32_t    m_sheetHeight;
	std::string m_tileSheet;
	char*       m_map;
	bool        m_isInitialized;
};
