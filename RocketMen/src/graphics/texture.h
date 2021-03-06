
#pragma once

#include <common.h>

class Texture
{
public:
	Texture(const void* imageData, uint32_t width, uint32_t height);
	~Texture();

	void bind();
	bool isBound() const;
	void unbind();

	uint32_t getWidth() const;
	uint32_t getHeight() const;

	void destroy();

private:
	GLuint   m_id;
	uint32_t m_width;
	uint32_t m_height;

	static Texture* s_currentTexture;
};