
#pragma once

#include <common.h>

//namespace physics {
	class Rigidbody;
//};

class Transform
{
public:
	Transform();
	~Transform();

	void    setLocalPosition(const Vector2& position);
	Vector2 getLocalPosition() const;

	void  setLocalRotation(float angle);
	float getLocalRotation() const;

	Vector2 getWorldPosition();
	float   getWorldRotation();

	void setScale(Vector2 scale);
	Vector2 getScale();

	void       parentTo(Transform* parent);
	Transform* getParent() const;

	glm::mat4 getLocalMatrix();
	glm::mat4 getWorldMatrix();

	bool isDirty() const;

	void setRigidbody(/*physics::*/Rigidbody* rb);

private:
	void updateLocalMatrix();

	Vector2    m_localPosition;
	Vector2    m_localScale;
	glm::mat4  m_localMatrix;
	float      m_localAngle;
	bool       m_isDirty;
	Transform* m_parent;

	/*physics::*/Rigidbody* m_rigidbody;
};

