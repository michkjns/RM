
#include <graphics/shader.h>

#include <core/debug.h>
#include <graphics/check_gl_error.h>

#include <GLFW/glfw3.h>

#include <fstream>
#include <iostream>

Shader* Shader::s_currentShader = nullptr;

void Shader::unbindCurrentShader()
{
	s_currentShader = nullptr;
	glUseProgram(0);
}

Shader::Shader() :
	m_isInitialized(false),
	m_glProgramID(0)
{
	m_glShader[0] = 0;
	m_glShader[1] = 0;
}

Shader::~Shader()
{
}

void Shader::use()
{
	if (!m_isInitialized)
	{
		LOG_ERROR("Shader::use() : This shader has not been sucessfully initialized!\n");
		return;
	}

	if (s_currentShader != this)
	{
		s_currentShader = this;
		glUseProgram(m_glProgramID);
	}
}

bool Shader::isUsed() const
{
	return (this == s_currentShader);
}

bool Shader::isInitialized() const
{
	return m_isInitialized;
}

bool Shader::compile(const std::string& vertexShader, const std::string& fragmentShader)
{
	const std::string sources[] = { vertexShader, fragmentShader };
	const Shader::Type type[] = { Type::VERTEX_SHADER, Type::FRAGMENT_SHADER };

	for (int i = 0; i < 2; i++)
	{
		const GLchar* source[] = { sources[i].c_str() };

		/* Create shader object. */
		m_glShader[i] = glCreateShader((GLenum)type[i]);

		/* Load the shader source for the shader object. */
		glShaderSource(m_glShader[i], 1, source, NULL);

		/* Compile the shader. */
		glCompileShader(m_glShader[i]);

		/* Check for errors */
		GLint compileStatus;
		glGetShaderiv(m_glShader[i], GL_COMPILE_STATUS, &compileStatus);
		if (compileStatus != GL_TRUE)
		{
			GLint logLength;
			glGetShaderiv(m_glShader[i], GL_INFO_LOG_LENGTH, &logLength);
			GLchar* infoLog = new GLchar[logLength];
			glGetShaderInfoLog(m_glShader[i], logLength, NULL, infoLog);
			LOG_ERROR("Shader Compilation error: %s", infoLog);

			std::cerr << infoLog << std::endl;

			delete[] infoLog;

			switch (type[i])
			{
				case Type::FRAGMENT_SHADER: LOG_ERROR("Failed to compile fragment shader");	break;
				case Type::VERTEX_SHADER:	LOG_ERROR("Failed to compile vertex shader ");	break;
				//case Type::GEOMETRY_SHADER: LOG_ERROR("Failed to compile geometry shader '%s'", shaderFile);		break;
			}
			ASSERT(false, "Invalid shader type");
			return false;
			/// Compile error!
		}
	}

	GLuint program = glCreateProgram();

	/* Attach the appropriate shader objects. */
	for (GLuint shader : m_glShader)
	{
		glAttachShader(program, shader);
	}

	/* Link the program */
	glLinkProgram(program);

	/* Check the link status. */
	GLint linkStatus;
	glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
	if (linkStatus != GL_TRUE)
	{
		GLint logLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
		GLchar* infoLog = new GLchar[logLength];

		glGetProgramInfoLog(program, logLength, NULL, infoLog);

		printf("%s\n", infoLog);

		delete[] infoLog;
		return false;
	}

	m_glProgramID = program;

	for (GLuint shader : m_glShader)
	{
		glDetachShader(m_glProgramID, shader);
		glDeleteShader(shader);
	}
	checkGL();


	m_isInitialized = true;
	return true;
}


void Shader::destroy()
{
	if (m_isInitialized)
	{
		glDeleteShader(m_glShader[0]);
		glDeleteShader(m_glShader[1]);
	}

	m_isInitialized = false;
	m_glShader[0] = 0;
	m_glShader[1] = 0; 
}

// TODO Prevent uploading identical values
void Shader::setFloat(const char* name, float value)
{
	glUniform1f(glGetUniformLocation(m_glProgramID, name), value);
}

void Shader::setInt(const char* name, int value)
{
	glUniform1i(glGetUniformLocation(m_glProgramID, name), value);
}

void Shader::setVec2f(const char* name, const glm::vec2& vector)
{
	glUniform2f(glGetUniformLocation(m_glProgramID, name), vector.x, vector.y);
}

void Shader::setVec3f(const char* name, const glm::vec3& vector)
{
	glUniform3f(glGetUniformLocation(m_glProgramID, name), vector.x, vector.y, vector.z);
}

void Shader::setVec4f(const char* name, const glm::vec4& vector)
{
	glUniform4f(glGetUniformLocation(m_glProgramID, name), vector.x, vector.y, vector.z, vector.w);
}

void Shader::setMatrix4(const char* name, const glm::mat4& matrix)
{
	glUniformMatrix4fv(glGetUniformLocation(m_glProgramID, name), 1, GL_FALSE, glm::value_ptr(matrix));
}
