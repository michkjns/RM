
#include <graphics/camera.h>
#include <graphics/renderer.h>

Camera::Camera(Vector2 viewportSize, int32_t pixelsPerMeter) :
	m_position(0),
	m_scale(1.f),
	m_rotation(),
	m_viewMatrix(),
	m_viewportSize(viewportSize),
	m_isDirty(false),
	m_pixelsPerMeter(pixelsPerMeter)
{
	assert(pixelsPerMeter != 0);

	m_projectionSize = Vector2(viewportSize.x / pixelsPerMeter, viewportSize.y / pixelsPerMeter);

	viewportSize.x /= pixelsPerMeter;
	viewportSize.y /= pixelsPerMeter;

	m_projectionMatrix = glm::ortho(-viewportSize.x / 2.0f, 
		                            viewportSize.x  / 2.0f, 
		                           -viewportSize.y  / 2.0f, 
		                            viewportSize.y  / 2.0f, 
		                            -1.0f, 1.0f);
}

Camera::~Camera()
{
}

glm::mat4 Camera::getViewMatrix() const
{
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
		glm::mat4 scale     = glm::scale(m_scale);

		m_viewMatrix = rotate * translate * scale;
		m_isDirty = false;
	}
}

void Camera::translate(Vector3 translation)
{
	m_position += translation;
	m_isDirty = true;
}

void Camera::rotate(const glm::quat& rotation)
{
	m_rotation = m_rotation * rotation;
	m_isDirty = true;
}

void Camera::scale(const Vector3& scale)
{
	m_scale   += scale;
	m_isDirty = true;
}

void Camera::rotate(const Vector3& rotation)
{
	rotate(glm::quat(rotation));
	m_isDirty = true;
}

void Camera::setPosition(const Vector3& pos)
{
	m_position = pos;
	m_isDirty = true;
}

void Camera::setRotation(const glm::quat& rot)
{
	m_rotation = rot;
	m_isDirty = true;
}

void Camera::setEulerAngles(const Vector3& eulerAngles)
{
	m_rotation = glm::quat(glm::radians(eulerAngles));
	m_isDirty = true;
}

void Camera::setScale(const Vector3& scale)
{
	m_scale = scale;
	m_isDirty = true;
}

int32_t Camera::getPixelsPerMeter() const
{
	return m_pixelsPerMeter;
}

Vector3 Camera::getPosition() const
{
	return m_position;
}

glm::quat Camera::getRotation() const
{
	return m_rotation;
}

Vector3 Camera::getEulerAngles() const
{
	return glm::degrees(glm::eulerAngles(m_rotation));
}

Vector2 Camera::screenToWorld(const Vector2& screenPoint)
{
	return (screenPoint - m_viewportSize / 2.f) / (float)m_pixelsPerMeter * Vector2(m_scale.x, -m_scale.y) + Vector2(m_position.x, m_position.y);
}
