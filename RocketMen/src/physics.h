
#pragma once

#include <physics/rigidbody.h>

#include <cstdint>
#include <vector>

namespace physics
{
	static const uint32_t s_maxRigidbodies = 64;
};

//==============================================================================

class Physics
{
public:
	void initialize();
	void step(float timestep);

	static Rigidbody* createRigidbody();
	static bool  destroyRigidbody(Rigidbody* rb);

private:
	uint32_t m_rigidbodyIDCounter;
	std::vector<Rigidbody*> m_rigidbodies;
};