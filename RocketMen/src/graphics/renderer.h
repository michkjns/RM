
#pragma once

class Window;

class Renderer
{
public:
	enum class ProjectionMode
	{
		ORTOGRAPHIC_PROJECTION,
		PERSPECTIVE_PROJECTION
	};

public:
	virtual ~Renderer() {};

	virtual bool initialize(ProjectionMode projection, Window* window) = 0;
	virtual void destroy() = 0;

	virtual void render() = 0;

	static Renderer* get();

private:
	static Renderer* g_singleton;
};