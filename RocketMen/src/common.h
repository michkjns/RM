
#pragma once

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

#ifdef _DEBUG
#define DEBUG_ONLY(x) x
#else
#define DEBUG_ONLY(x) 
#endif

#ifdef assert
#undef assert
#endif

#define _assert(expression) (!!(expression)) || \
    (_wassert(_CRT_WIDE(#expression), _CRT_WIDE(__FILE__), (unsigned)(__LINE__)), 0)

#if defined __GNUC__
#define LIKELY(EXPR)  __builtin_expect(!!(EXPR), 1)
#else
#define LIKELY(EXPR)  (!!(EXPR))
#endif

#define ensure(X) (LIKELY(!!(X)) || (__debugbreak(), !!(X)))

#define assert(X) (bool)(X ? true : _assert(false), false)
