
#pragma once

class Window;

class Renderer
{
public:
	virtual ~Renderer() {};

	virtual bool initialize(Window* window) = 0;
	virtual void destroy() = 0;

	virtual void render() = 0;

	void drawPolygon(const Vector2* vertices,
					 int32_t vertexCount,
					 const Color& color,
					 bool screenSpace = false);

	virtual	Vector2 getScreenSize() const = 0;

	static Renderer* get();

private:
	static Renderer* g_singleton;
};