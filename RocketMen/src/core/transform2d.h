
#pragma once

#include <common.h>
#include <core/debug.h>
#include <physics/rigidbody.h>

class Transform2D
{
public:
	Transform2D();
	~Transform2D();

	void    setLocalPosition(const Vector2& position);
	Vector2 getLocalPosition() const;

	void  setLocalRotation(float angle);
	float getLocalRotation() const;

	Vector2 getWorldPosition() const;
	float   getWorldRotation() const;

	void    setScale(Vector2 scale);
	Vector2 getScale();

	void         attach(Transform2D* parent);
	Transform2D* getParent() const;

	glm::mat4 getLocalMatrix();
	glm::mat4 getWorldMatrix();

	bool isDirty() const;

	void setRigidbody(Rigidbody* rigidBody);

private:
	void updateLocalMatrix();

	Vector2      m_localPosition;
	Vector2      m_localScale;
	glm::mat4    m_localMatrix;
	float        m_localAngle;
	bool         m_isDirty;
	Transform2D* m_parent;

	Rigidbody* m_rigidbody;

public:
	/** Serialize complete object */
	template<typename Stream>
	bool serializeFull(Stream& stream);

	/** Serialize commonly updated variables */
	template<typename Stream>
	bool serialize(Stream& stream);
};

template<typename Stream>
inline bool Transform2D::serializeFull(Stream& stream)
{
	return ensure(serialize(stream));
}

template<typename Stream>
inline bool Transform2D::serialize(Stream& stream)
{
	Vector2 position = getLocalPosition();
	if (!serializeVector2(stream, position))
		return ensure(false);
	if (Stream::isReading)
		setLocalPosition(position);

	bool hasRigidbody = m_rigidbody != nullptr;
	serializeBool(stream, hasRigidbody);
	if (hasRigidbody)
	{
		assert(m_rigidbody != nullptr);
		ensure(m_rigidbody->serialize(stream));
	}

	return true;
}
