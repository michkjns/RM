
#pragma once

#include <common.h>

struct TilemapParam
{
	const char* name;
	std::string tileData;
	Vector2i    dimensions;
	uint32_t    tileSize;
	const char* tileSheetName;
};

class Tilemap
{
public:
	static Vector2i getDimensionsFromFilename(const char* filename);
public:
	Tilemap(const TilemapParam& parameters);
	~Tilemap();
	
	std::string getName()          const { return m_name; }
	uint32_t    getTileSize()      const { return m_tileSize; }
	uint32_t    getMapWidth()      const { return m_width; }
	uint32_t    getMapHeight()     const { return m_height; }
	uint32_t    getSheetWidth()    const { return m_sheetWidth; }
	uint32_t    getSheetHeight()   const { return m_sheetHeight; }
	char*       getMap()           const { return m_map; }
	std::string getTileSheetName() const { return m_tileSheetName; }

private:
	void createTexture();

	std::string m_name;
	uint32_t    m_tileSize;
	uint32_t    m_width;
	uint32_t    m_height;
	uint32_t    m_sheetWidth;
	uint32_t    m_sheetHeight;
	std::string m_tileSheetName;
	char*       m_map;
};
