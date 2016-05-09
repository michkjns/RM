
#pragma once

#include "debug.h"

#include <algorithm>
#include <stdio.h>
#include <string>

#define GLEW_STATIC
#include "GL/glew.h"

#define GLM_FORCE_RADIANS

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

using float2 = glm::vec2;

inline std::string toLower(const std::string& str)
{
	std::string out(str);
	std::transform(out.begin(), out.end(), out.begin(), ::tolower);
	return out;
}

inline std::string toUpper(const std::string& str)
{
	std::string out(str);
	std::transform(out.begin(), out.end(), out.begin(), ::toupper);
	return out;
}
