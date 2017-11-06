
#include <core/debug.h>
#include <graphics/check_gl_error.h>

void checkOpenGLError( const char* /*msg*/, const char* file, int line )
{
    GLenum errorCode = GL_NO_ERROR;
    while ( ( errorCode = glGetError() ) != GL_NO_ERROR )
    {
		LOG_ERROR("OpenGL Error: %d, %s line %d", errorCode, file, line);

    }
}