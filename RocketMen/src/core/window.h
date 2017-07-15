
#pragma once

#include <common.h>

class Window
{
public:
	Window() = default;
	virtual ~Window() {};

	virtual bool initialize(const char* title, Vector2i size) = 0;
	virtual void terminate() = 0;
	virtual void setTitle(const char* title) = 0;

	virtual void swapBuffers() = 0;
	virtual bool pollEvents() = 0;

	virtual Vector2i getSize() const = 0;
	virtual uint32_t getWidth() const = 0;
	virtual uint32_t getHeight() const = 0;

	virtual void onResize(struct GLFWwindow* window, Vector2i newSize) = 0;
	virtual struct GLFWwindow* getGLFWwindow() const = 0;

	static Window* create();
};
