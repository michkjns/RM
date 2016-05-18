
#include <core/transform.h>

Transform::Transform() :
	m_localScale(1.0f),
	m_localAngle(0.0f),
	m_isDirty(false),
	m_parent(nullptr)
{
}

Transform::~Transform()
{
}

void Transform::setLocalPosition(const Vector2& position)
{
	m_localPosition = position;
	m_isDirty = true;
}

Vector2 Transform::getLocalPosition() const
{
	return m_localPosition;
}

void Transform::setLocalRotation(float angle)
{
	m_localAngle = angle;
	m_isDirty = true;
}

float Transform::getLocalRotation() const
{
	return m_localAngle;
}

Vector2 Transform::getWorldPosition()
{
	if (m_parent)
	{
		return m_parent->getWorldPosition() + m_localPosition;
	}
	else
		return m_localPosition;
}

float Transform::getWorldRotation()
{
	if (m_parent)
	{
		return m_parent->getWorldRotation() + m_localAngle;
	}
	else
		return m_localAngle;
}

void Transform::parentTo(Transform* parent)
{
	m_parent = parent;
}

Transform* Transform::getParent() const
{
	return m_parent;
}

glm::mat4 Transform::getLocalMatrix()
{
	if (m_isDirty)
	{
		updateLocalMatrix();
	}
	return m_localMatrix;
}

glm::mat4 Transform::getWorldMatrix()
{
	if (m_parent != nullptr)
	{
		return getLocalMatrix();
	}
	return m_parent->getWorldMatrix() * getLocalMatrix();
}

bool Transform::isDirty() const
{
	return m_isDirty;
}

void Transform::updateLocalMatrix()
{
	m_localMatrix = glm::translate(m_localMatrix, Vector3(m_localPosition, 0.0f)) *
		glm::mat4_cast(glm::angleAxis(m_localAngle, Vector3(0.0f, 0.0f, 1.0f))) *
		glm::scale(Vector3(m_localScale, 1.0f));

	m_isDirty = false;
}
