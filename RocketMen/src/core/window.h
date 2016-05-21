
#pragma once

#include <cstdint>

class Window
{
public:
	virtual ~Window() {};

	virtual bool initialize(uint32_t width, uint32_t height) = 0;
	virtual void terminate() = 0;

	virtual void swapBuffers() = 0;
	virtual bool pollEvents() = 0;

	virtual uint32_t getWidth() const = 0;
	virtual uint32_t getHeight() const = 0;

	virtual void* getGLFWwindow() const = 0;

	//virtual int32_t getFrameWidth()  const = 0;

	static Window* create();
};
