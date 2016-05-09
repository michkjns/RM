
#pragma once

#ifdef _DEBUG
#define checkGL() checkOpenGLError( __FUNCSIG__, __FILE__, __LINE__ )
#else
#define checkGL() ((void*)0) // Do nothing in release builds.
#endif

void checkOpenGLError( const char* msg, const char* file, int line );
