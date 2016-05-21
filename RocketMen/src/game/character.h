
#pragma once

#include <core/entity.h>

namespace rm
{

	class Character : public Entity
	{
	public:
		Character();
		virtual ~Character();

		virtual void initialize();
		virtual void update(float deltaTime)      override;
		virtual void fixedUpdate(float deltaTime) override;

		virtual void startContact(Entity* other) override;
		virtual void endContact(Entity* other)   override;

		Rigidbody* getRigidbody() const;

	private:
		Rigidbody* m_rigidbody;
	};

}; // namespace rm