#pragma once

#include "includes.h"

using namespace glm;

class Camera
{
public:
	Camera();
	~Camera();

	mat4 getViewMatrix() const;
	void translate(vec3 translation);

private:
	mat4 m_viewMatrix;
};