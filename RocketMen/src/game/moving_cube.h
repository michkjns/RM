
#pragma once

#include <core/entity.h>
#include <core/entity_factory.h>

namespace rm
{
	class MovingCube : public Entity
	{
	public:
		DECLARE_ENTITY(EntityType::MovingCube);

	public:
		MovingCube();
		virtual ~MovingCube();

		virtual void update(float deltaTime)      override;
		virtual void fixedUpdate(float deltaTime) override;
		virtual void debugDraw()                  override;

	private:
		float m_speed;
		float m_direction;

	public:
		/** Serialize complete object */
		template<typename Stream>
		bool serializeFull(Stream& stream);

		/** Serialize commonly updated variables */
		template<typename Stream>
		bool serialize(Stream& stream);
	};

};// namespace rm