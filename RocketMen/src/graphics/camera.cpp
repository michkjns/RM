
#include <graphics/camera.h>
#include <graphics/renderer.h>

using namespace graphics;

Camera* Camera::mainCamera;

Camera::Camera(ProjectionMode projection, float width, float height, int32_t pixelsPerMeter) :
	m_position(0),
	m_scale(1.f),
	m_rotation(),
	m_viewMatrix(1),
	m_isDirty(false),
	m_pixelsPerMeter(pixelsPerMeter),
	m_projSize(width / pixelsPerMeter, height / pixelsPerMeter)
{
	switch (projection)
	{
		case ProjectionMode::ORTOGRAPHIC_PROJECTION:
		{
			width /= pixelsPerMeter;
			height /= pixelsPerMeter;
			m_projectionMatrix = glm::ortho(-width  / 2.0f, 
											width   / 2.0f, 
											-height / 2.0f, 
											height  / 2.0f, 
											-1.0f, 1.0f);
			break;
		}
		case ProjectionMode::PERSPECTIVE_PROJECTION:
		{
			// TODO(Support perspective projection?)
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
	const Vector2 ratio = Vector2(1.f / (m_pixelsPerMeter * m_scale.x),
								  -1.f / (m_pixelsPerMeter * m_scale.y));
	const Vector2 view = Renderer::get()->getScreenSize();
	const Vector2 worldPos = Vector2(screenPoint.x * ratio.x - (view.x * ratio.x / 2.f),
									 screenPoint.y * ratio.y - (view.y * ratio.y / 2.f))
		 + Vector2(m_position.x, m_position.y) / 2.f;
	return worldPos;
}
