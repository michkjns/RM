
#pragma once

#include <includes.h>

class RigidbodyData;

class Rigidbody
{
public:
	Rigidbody();
	~Rigidbody();

	void setPosition(const glm::vec2& position);
	glm::vec2 getPosition() const;

	void  setAngle(float angle);
	float getAngle() const;

	void setTransform(const glm::vec2& position, float angle);

	void setLinearVelocity(glm::vec2& vel);
	glm::vec2 getLinearVelocity() const;

	RigidbodyData* getData() const;

private:
	RigidbodyData* m_body;
};

inline bool operator==(const Rigidbody&a, const Rigidbody& b)
{
	return (a.getData() == b.getData());
}