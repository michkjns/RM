
#include "includes.h"
#include "texture.h"

Texture* Texture::s_currentTexture = nullptr;

Texture::Texture()
{
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

bool Texture::generate(unsigned char * imageData, uint32_t width, uint32_t height)
{
	glGenTextures(1, &m_id);
	glBindTexture(GL_TEXTURE_2D, m_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);
	glGenerateMipmap(GL_TEXTURE_2D);

	//glGetError() /** @todo	Error checking here */
	m_width = width;
	m_height = height;
	return true;
}

uint32_t Texture::getWidth() const
{
	return m_width;
}

uint32_t Texture::getHeight() const
{
	return m_height;
}
