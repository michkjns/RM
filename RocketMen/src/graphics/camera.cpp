
#include <graphics/camera.h>

using namespace graphics;

Camera* Camera::mainCamera;

Camera::Camera(ProjectionMode projection, float width, float height) :
	m_viewport(0, 0, width, height),
	m_position(0),
	m_rotation(),
	m_viewMatrix(1),
	m_isDirty(false),
	m_focalDepth(1.f)
{
	switch (projection)
	{
		case ProjectionMode::ORTOGRAPHIC_PROJECTION:
		{
			m_projectionMatrix = glm::ortho(0.0f, width, 0.0f, height, -1.0f, 1.0f);
			break;
		}
		case ProjectionMode::PERSPECTIVE_PROJECTION:
		{
			// TODO(Support perspective projection)
			assert(false);
			break;
		}
	}
}

Camera::~Camera()
{
}

glm::mat4 Camera::getViewMatrix()
{
	updateViewMatrix();
	return m_viewMatrix;
}

glm::mat4 Camera::getProjectionMatrix() const
{
	return m_projectionMatrix;
}

void Camera::updateViewMatrix()
{
	if (m_isDirty)
	{
		glm::mat4 translate = glm::translate(-m_position);
		glm::mat4 rotate    = glm::transpose(glm::toMat4(m_rotation));

		m_viewMatrix = rotate * translate;
		m_isDirty = false;
	}
}

void Camera::translate(glm::vec3 translation)
{
	m_position += translation;
	m_isDirty = true;
}

void Camera::rotate(const glm::quat& rotation)
{
	m_rotation = m_rotation * rotation;
	m_isDirty = true;
}

void Camera::rotate(const glm::vec3& rotation)
{
	rotate(glm::quat(rotation));
	m_isDirty = true;
}

void Camera::setPosition(const glm::vec3& pos)
{
	m_position = pos;
	m_isDirty = true;
}

void Camera::setRotation(const glm::quat& rot)
{
	m_rotation = rot;
	m_isDirty = true;
}

void Camera::setEulerAngles(const glm::vec3& eulerAngles)
{
	m_rotation = glm::quat(glm::radians(eulerAngles));
	m_isDirty = true;
}

void Camera::setViewport(int32_t x, int32_t y, int32_t width, int32_t height)
{
	m_viewport = glm::vec4(x, y, width, height);
	glViewport(x, y, width, height);
	m_isDirty = true;
}

float Camera::getFocalDepth() const
{
	return m_focalDepth;
}

glm::vec3 Camera::getPosition() const
{
	return m_position;
}

glm::quat Camera::getRotation() const
{
	return m_rotation;
}

glm::vec3 Camera::getEulerAngles() const
{
	return glm::degrees(glm::eulerAngles(m_rotation));
}

glm::vec4 Camera::getViewport() const
{
	return m_viewport;
}
