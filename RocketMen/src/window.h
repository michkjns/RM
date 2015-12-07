
#pragma once

class Window
{
public:
	virtual ~Window() {};

	virtual bool initialize(unsigned int width, unsigned int height) = 0;
	virtual void terminate() = 0;

	virtual void swapBuffers() = 0;
	virtual bool pollEvents() = 0;

	virtual unsigned int getWidth() const = 0;
	virtual unsigned int getHeight() const = 0;

	static Window* create();

};