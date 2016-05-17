
#pragma once

class Window;

class Renderer
{
public:
	virtual ~Renderer() {};

	virtual bool initialize(Window* window) = 0;
	virtual void destroy() = 0;

	virtual void render() = 0;

	static Renderer* get();

private:
	static Renderer* g_singleton;
};