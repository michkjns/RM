
#pragma once

class Texture
{
public:
	Texture();
	~Texture();
	
	void bind();
	bool isBound() const;
	void unbind();

	bool generate(unsigned char* imageData, uint32_t width, uint32_t height);

	uint32_t getWidth() const;
	uint32_t getHeight() const;

private:
	GLuint m_id;
	uint32_t m_width;
	uint32_t m_height;

	static Texture* s_currentTexture;
};
