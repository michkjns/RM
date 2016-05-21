
#pragma once

#include <includes.h>

class RigidbodyImpl;
//
//namespace physics {
//	enum class BodyType
//	{
//		STATIC_BODY= 0,
//		KINEMATIC_BODY,
//		DYNAMIC_BODY
//	};
//};


class Rigidbody
{
public:
	Rigidbody();
	~Rigidbody();

	void    setPosition(const Vector2& position);
	Vector2 getPosition() const;

	void  setAngle(float angle);
	float getAngle() const;

	void setTransform(const Vector2& position, float angle);

	void    setLinearVelocity(Vector2& vel);
	Vector2 getLinearVelocity() const;

	void applyLinearImpulse(const Vector2& force, const Vector2& position);
	
	Vector2 getWorldCenter() const;
	
	float getMass() const;

	RigidbodyImpl* getImpl() const;

private:
	RigidbodyImpl* m_impl;
};

inline bool operator==(const Rigidbody&a, const Rigidbody& b)
{
	return (a.getImpl() == b.getImpl());
}