
#pragma once

#include <core/entity.h>

class Character : public Entity
{
public:
	Character();
	virtual ~Character();

	virtual void initialize();
	virtual void update(float deltaTime) override;
	virtual void fixedUpdate()           override;

private:
	Rigidbody* m_rigidbody;
};

