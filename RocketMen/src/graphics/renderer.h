
#pragma once

#include <common.h>

class Window;

class Renderer
{
public:
	virtual ~Renderer() {};

	virtual bool initialize(Window* window) = 0;
	virtual void destroy() = 0;

	virtual void render() = 0;

	virtual void drawPolygon(const Vector2* vertices,
	                 int32_t vertexCount,
	                 const Color& color,
	                 bool screenSpace = false) = 0;
	
	virtual void drawLineSegment(const Vector2& p1,
	                             const Vector2& p2,
	                             const Color& color,
	                             bool screenSpace = false) = 0;

	virtual	Vector2i getScreenSize() const = 0;

	static Renderer* create();
	static Renderer* get();
};