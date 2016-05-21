
#include <core/transform.h>

#include <physics/rigidbody.h>

Transform::Transform() :
	m_localScale(1.0f),
	m_localAngle(0.0f),
	m_isDirty(false),
	m_parent(nullptr),
	m_rigidbody(nullptr)
{
}

Transform::~Transform()
{
}

void Transform::setLocalPosition(const Vector2& position)
{
	if (m_rigidbody)
	{
		return m_rigidbody->setPosition(position);
	}

	m_localPosition = position;
	m_isDirty = true;
}

Vector2 Transform::getLocalPosition() const
{
	if (m_rigidbody)
	{
		return m_rigidbody->getPosition();
	}

	return m_localPosition;
}

void Transform::setLocalRotation(float angle)
{
	if (m_rigidbody)
	{
		return m_rigidbody->setAngle(angle);
	}

	m_localAngle = angle;
	m_isDirty = true;
	
}

float Transform::getLocalRotation() const
{
	if (m_rigidbody)
	{
		return m_rigidbody->getAngle();
	}
	return m_localAngle;
}

Vector2 Transform::getWorldPosition()
{
	if (m_parent)
	{
		return m_parent->getWorldPosition() + m_localPosition;
	}
	else
		return getLocalPosition();
}

float Transform::getWorldRotation()
{
	if (m_parent)
	{
		return m_parent->getWorldRotation() + m_localAngle;
	}
	else
		return getLocalRotation();
}

void Transform::setScale(Vector2 scale)
{
	m_localScale = scale;
	m_isDirty = true;
}

Vector2 Transform::getScale()
{
	glm::mat4 localMat = getLocalMatrix();
	return Vector2(localMat[0].x, localMat[1].y);
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
	if (m_rigidbody)
	{
		return glm::translate(m_localMatrix, Vector3(getLocalPosition(), 0.0f)) *
			glm::mat4_cast(glm::angleAxis(getLocalRotation(), Vector3(0.0f, 0.0f, 1.0f))) *
			glm::scale(Vector3(m_localScale, 1.0f));
	}
	else if (m_isDirty)
	{
		updateLocalMatrix();
	}
	return m_localMatrix;
}

glm::mat4 Transform::getWorldMatrix()
{
	if (m_parent != nullptr)
	{
		return m_parent->getWorldMatrix() * getLocalMatrix();
	}
	return getLocalMatrix();
}

bool Transform::isDirty() const
{
	return m_isDirty;
}

void Transform::setRigidbody(Rigidbody* rb)
{
	m_rigidbody = rb;
}

void Transform::updateLocalMatrix()
{
	m_isDirty = false;

	if (m_rigidbody)
	{
		m_localMatrix = glm::translate(m_localMatrix, Vector3(getLocalPosition(), 0.0f)) *
			glm::mat4_cast(glm::angleAxis(getLocalRotation(), Vector3(0.0f, 0.0f, 1.0f))) *
			glm::scale(Vector3(m_localScale, 1.0f));
	}
	else
	{
		m_localMatrix = glm::translate(m_localMatrix, Vector3(m_localPosition, 0.0f)) *
			glm::mat4_cast(glm::angleAxis(m_localAngle, Vector3(0.0f, 0.0f, 1.0f))) *
			glm::scale(Vector3(m_localScale, 1.0f));
	}
}
