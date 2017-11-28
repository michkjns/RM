
#pragma once

#include <common.h>

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

	void    setLinearVelocity(const Vector2& vel);
	Vector2 getLinearVelocity() const;

	void applyLinearImpulse(const Vector2& force, const Vector2& position);
	
	Vector2 getWorldCenter() const;
	
	float getMass() const;

	RigidbodyImpl* getImpl() const;

private:
	RigidbodyImpl* m_impl;

public:
	template<typename Stream>
	bool serializeFull(Stream& stream);

	template<typename Stream>
	bool serialize(Stream& stream);
};

inline bool operator==(const Rigidbody&a, const Rigidbody& b)
{
	return (a.getImpl() == b.getImpl());
}

template<typename Stream>
bool Rigidbody::serializeFull(Stream& stream)
{
	return ensure(serialize(stream));
}

template<typename Stream>
bool Rigidbody::serialize(Stream& stream)
{
	Vector2 velocity;
	if (Stream::isWriting)
	{
		velocity = getLinearVelocity();
	}

	if (!serializeVector2(stream, velocity))
		return ensure(false);

	if (Stream::isReading)
		setLinearVelocity(velocity);

	return true;
}