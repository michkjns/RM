
#pragma once

class Texture
{
public:
	Texture();
	~Texture();
	
	void bind();
	bool isBound();

private:
	GLuint m_id;
	bool m_isBound;
};
