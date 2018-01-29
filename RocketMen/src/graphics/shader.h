
#pragma once

#include <common.h>

class Shader
{
public:
	enum class Type
	{
		FRAGMENT_SHADER = GL_FRAGMENT_SHADER,
		VERTEX_SHADER   = GL_VERTEX_SHADER,
	//	GEOMETRY_SHADER = GL_GEOMETRY_SHADER
	};

	static void unbindCurrentShader();

public:
	Shader();
	~Shader();

	void use();
	bool isUsed() const;
	bool isInitialized() const;
	bool compile( const std::string& vertexShader, 
				 const std::string& fragmentShader);

	/** Clears the isInitialized flag, to prevent from the destructor deleting
		the gl shaders. */
	void destroy();

	void setFloat(const char* name, float value);
	void setInt(const char* name, int value);
	void setVec2f(const char* name, const glm::vec2& vector);
	void setVec3f(const char* name, const glm::vec3& vector);
	void setVec4f(const char* name, const glm::vec4& vector);
	void setMatrix4(const char* name, const glm::mat4& matrix);

private:
	bool m_isInitialized;
	GLuint m_glProgramID;
	GLuint m_glShader[2];

	static Shader* s_currentShader;

};