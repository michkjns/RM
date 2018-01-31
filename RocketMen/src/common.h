
#pragma once
#pragma warning(default:4061)
#pragma warning(default:4062)

#include <cstdint>
#include <stdio.h>
#include <string>

#define GLEW_STATIC
#include "GL/glew.h"

#define GLM_FORCE_RADIANS

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

const int32_t INDEX_NONE = -1;

using Vector2i = glm::ivec2;
using Vector2  = glm::vec2;
using Vector3  = glm::vec3;
using Vector4  = glm::vec4;
using Color    = glm::vec4;
using iColor   = glm::ivec4;
using Sequence = uint16_t;

#ifdef assert
	#undef assert
#endif

#ifdef _DEBUG
#define ASSERT(expression, ...) do { if (!(expression)) \
{ \
	( fprintf(stderr, "Assertion Failed: " __VA_ARGS__) && fprintf(stderr," at %s:%d\n",__FILE__,__LINE__));\
__debugbreak();  \
} } while (0) 
#else
#define ASSERT(test, msg, ...) ((void)0)
#endif

inline bool sequenceGreaterThan(Sequence s1, Sequence s2)
{
	return ((s1 > s2) && (s1 - s2 <= 32768)) ||
		((s1 < s2) && (s2 - s1 > 32768));
}

inline bool sequenceLessThan(Sequence s1, Sequence s2)
{
	return sequenceGreaterThan(s2, s1);
}

inline int32_t sequenceDifference(Sequence s1, Sequence s2)
{
	int32_t s1_32 = s1;
	int32_t s2_32 = s2;
	if (abs(s1_32 - s2_32) >= 32768)
	{
		if (s1_32 > s2_32)
		{
			s2_32 += 65536;
		}
		else
		{
			s1_32 += 65536;
		}
	}
	return s1_32 - s2_32;
}
