
#pragma once

#include <common.h>

class Camera;
class Window;

enum class RenderSpace
{
	ScreenSpace,
	WorldSpace
};

struct RenderContext
{
	class Physics* physics;
	const class Game& game;
};

struct LineSegment
{
	Vector2     pointA;
	Vector2     pointB;
	RenderSpace renderSpace;
};

class Renderer
{
public:
	Renderer() : m_currentCamera(nullptr) {}
	virtual ~Renderer() {};

	virtual bool initialize() = 0;
	virtual void destroy() = 0;

	virtual void render(const RenderContext& context, bool debugDrawEnabled) = 0;

	virtual void drawPolygon(const Vector2* vertices,
	                 int32_t vertexCount,
	                 const Color& color,
	                 bool screenSpace = false) = 0;
	
	virtual void drawLineSegment(const LineSegment& lineSegment, const Color& color ) = 0;

	Camera* getCurrentCamera() const { return m_currentCamera; };

	static Renderer* create();
	static Renderer* get();

protected:
	Camera* m_currentCamera;
};