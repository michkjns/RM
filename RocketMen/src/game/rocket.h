
#pragma once

#include <core/entity.h>

class Rocket : public Entity
{
public:
	Rocket();
	Rocket(Vector2 direction, float accelerationPower);
	virtual ~Rocket();

	virtual void initialize();
	virtual void update(float deltaTime)      override;
	virtual void fixedUpdate(float deltaTime) override;

	void setDirection(Vector2 direction, float power);
//	Vector2 getDirection() const;

	void setAccelerationPower(float accelerationPower);
	float getAccelerationPower() const;

	virtual void startContact(Entity* other) override;
	virtual void endContact(Entity* other)   override;

private:
	Rigidbody* m_rigidbody;
	Vector2    m_direction;
	float      m_accelerationPower;

};