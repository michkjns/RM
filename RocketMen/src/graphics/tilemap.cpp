
#include <graphics/tilemap.h>

#include <core/debug.h>
#include <core/resource_manager.h>
#include <graphics/texture.h>

Tilemap::Tilemap(const TilemapParam& parameters) :
	m_name(parameters.name),
	m_tileSize(parameters.tileSize),
	m_width(parameters.dimensions.x),
	m_height(parameters.dimensions.y),
	m_tileSheetName(parameters.tileSheetName),
	m_map(nullptr)
{
	ASSERT(parameters.name != nullptr, "Tilemap name cannot be null");
	ASSERT(parameters.tileSheetName != nullptr, "Tilesheet name Cannot be null");
	ASSERT(parameters.tileData.size() > 0, "tileData cannot be empty");
	ASSERT(m_width > 0, "Tilemap width cannot be 0");
	ASSERT(m_height > 0, "Tilemap height cannot be 0");

	std::string tiles;
	for (char character : parameters.tileData)
	{
		if (character != ',' && character != '\n')
		{
			tiles += character;
		}
	}

	const int32_t numTiles = m_width * m_height;
	if (numTiles == tiles.size())
	{
		m_map = new char[numTiles];
		memcpy_s(m_map, numTiles, tiles.data(), numTiles);

		createTexture();
	}
	else
	{
		LOG_ERROR("Tilemap::Tilemap: Error loading tiles (%s)", m_name.c_str());
	}
}

Tilemap::~Tilemap()
{
	delete[] m_map;
}

Vector2i Tilemap::getDimensionsFromFilename(const char* filename)
{
	ASSERT(filename != nullptr);
	std::string widthHeight(filename);

	const int32_t dotFirst = static_cast<int32_t>(widthHeight.find_first_of('.', 0));
	const int32_t dotLast = static_cast<int32_t>(widthHeight.find_first_of('.', dotFirst + 1));
	widthHeight = widthHeight.substr(dotFirst, dotLast);

	std::string widthStr = widthHeight.substr(1, widthHeight.find('x') - 1);
	std::string heightStr = widthHeight.substr(widthHeight.find('x') + 1, widthHeight.find_last_of('.') - 3);

	return Vector2i(atoi(widthStr.c_str()), atoi(heightStr.c_str()));
}

void Tilemap::createTexture()
{
	const uint32_t imageWidth = m_width * m_tileSize;
	const uint32_t imageHeight = m_height * m_tileSize;

	uint32_t* imageData = new uint32_t[imageWidth * imageHeight];
	for (uint32_t y = 0; y < m_height; y++)
	{
		for (uint32_t x = 0; x < m_width; x++)
		{
			imageData[x + y * imageWidth] = m_map[x + y * m_width] - '0';
		}
	}

	ResourceManager::createTexture(imageData, imageWidth, imageHeight, m_name.c_str());
	delete[] imageData;
}
