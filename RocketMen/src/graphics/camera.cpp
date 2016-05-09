
#include <graphics/camera.h>

Camera::Camera()
{
}

Camera::~Camera()
{
}

mat4 Camera::getViewMatrix() const
{
	return m_viewMatrix;
}

void Camera::translate(vec3 translation)
{
	m_viewMatrix = glm::translate(m_viewMatrix, translation);
}
