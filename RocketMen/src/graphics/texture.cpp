
#include "texture.h"

#include <core/debug.h>

Texture* Texture::s_currentTexture = nullptr;

Texture::Texture(const void* imageData, uint32_t width, uint32_t height) :
	m_width(0),
	m_height(0)
{
	ASSERT(imageData != nullptr);
	ASSERT(width > 0, "Texture width must be > 0");
	ASSERT(height > 0, "Texture height must be > 0");

	glGenTextures(1, &m_id);
	glBindTexture(GL_TEXTURE_2D, m_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Clamping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	// Filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture()
{
}

void Texture::bind()
{
	if(s_currentTexture != this)
	{
		s_currentTexture = this;
		glBindTexture(GL_TEXTURE_2D, m_id);
	}
}

void Texture::unbind()
{
	s_currentTexture = nullptr;
	glBindTexture(GL_TEXTURE_2D, 0);
}

bool Texture::isBound() const
{
	return (s_currentTexture == this);
}

uint32_t Texture::getWidth() const
{
	return m_width;
}

uint32_t Texture::getHeight() const
{
	return m_height;
}

void Texture::destroy()
{
	if (m_id != 0)
	{
		glDeleteTextures(1, &m_id);
		m_id = 0;
	}
}
