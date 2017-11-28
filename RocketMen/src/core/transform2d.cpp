
#include <core/transform2d.h>

Transform2D::Transform2D() :
	m_localScale(1.0f),
	m_localAngle(0.0f),
	m_isDirty(false),
	m_parent(nullptr),
	m_rigidbody(nullptr),
	m_localMatrix()
{
}

Transform2D::~Transform2D()
{
}

void Transform2D::setLocalPosition(const Vector2& position)
{
	if (m_rigidbody)
	{
		m_rigidbody->setPosition(position);
	}

	m_localPosition = position;
	m_isDirty = true;
}

Vector2 Transform2D::getLocalPosition() const
{
	if (m_rigidbody)
	{
		return m_rigidbody->getPosition();
	}

	return m_localPosition;
}

void Transform2D::setLocalRotation(float angle)
{
	if (m_rigidbody)
	{
		m_rigidbody->setAngle(angle);
	}

	m_localAngle = angle;
	m_isDirty = true;	
}

float Transform2D::getLocalRotation() const
{
	if (m_rigidbody)
	{
		return m_rigidbody->getAngle();
	}

	return m_localAngle;
}

Vector2 Transform2D::getWorldPosition() const
{
	if (m_parent)
	{
		return m_parent->getWorldPosition() + getLocalPosition();
	}
	else
	{
		return getLocalPosition();
	}
}

float Transform2D::getWorldRotation() const
{
	if (m_parent)
	{
		return m_parent->getWorldRotation() + getLocalRotation();
	}
	else
	{
		return getLocalRotation();
	}
}

void Transform2D::setScale(Vector2 scale)
{
	m_localScale = scale;
	m_isDirty = true;
}

Vector2 Transform2D::getScale()
{
	return m_localScale;
}

void Transform2D::attach(Transform2D* parent)
{
	m_parent = parent;
}

Transform2D* Transform2D::getParent() const
{
	return m_parent;
}

glm::mat4 Transform2D::getLocalMatrix()
{
	if (m_rigidbody)
	{
		return glm::translate(m_localMatrix, Vector3(getLocalPosition(), 0.0f)) *
			glm::mat4_cast(glm::angleAxis(getLocalRotation(), Vector3(0.0f, 0.0f, 1.0f))) *
			glm::scale(Vector3(m_localScale, 1.0f));
	}

	if (m_isDirty)
	{
		updateLocalMatrix();
	}
	 
	return m_localMatrix;
}

glm::mat4 Transform2D::getWorldMatrix()
{
	if (m_parent != nullptr)
	{
		return m_parent->getWorldMatrix() * getLocalMatrix();
	}

	return getLocalMatrix();
}

bool Transform2D::isDirty() const
{
	return m_isDirty;
}

void Transform2D::setRigidbody(Rigidbody* rigidbody)
{
	m_rigidbody = rigidbody;
	if (rigidbody)
	{
		if (m_parent)
		{
			m_rigidbody->setPosition(m_parent->getWorldPosition() + m_localPosition);
		}
		else
		{
			m_rigidbody->setPosition(m_localPosition);
		}
	}
}

void Transform2D::updateLocalMatrix()
{
	m_isDirty = false;

	m_localMatrix = glm::translate(glm::mat4(), Vector3(m_localPosition, 0.0f)) *
			glm::mat4_cast(glm::angleAxis(m_localAngle, Vector3(0.0f, 0.0f, 1.0f))) *
			glm::scale(Vector3(m_localScale, 1.0f));
}
