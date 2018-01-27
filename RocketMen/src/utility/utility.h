
#pragma once

#include <common.h>

#include <algorithm>

inline std::string toLower(const std::string& string)
{
	std::string out(string);
	std::transform(out.begin(), out.end(), out.begin(), ::tolower);
	return out;
}

inline std::string toUpper(const std::string& str)
{
	std::string out(str);
	std::transform(out.begin(), out.end(), out.begin(), ::toupper);
	return out;
}

inline uint32_t toABGR(const Color& color)
{
	uint32_t red = static_cast<uint32_t>(color.x * 255.0f);
	uint32_t green = static_cast<uint32_t>(color.y * 255.0f);
	uint32_t blue = static_cast<uint32_t>(color.z * 255.0f);
	uint32_t alpha = static_cast<uint32_t>(color.w * 255.0f);
	return (alpha << 24) + (blue << 16) + (green << 8) + red;
}

inline uint32_t toABGR(const iColor& color)
{
	uint32_t red = static_cast<uint32_t>(color.x);
	uint32_t green = static_cast<uint32_t>(color.y);
	uint32_t blue = static_cast<uint32_t>(color.z);
	uint32_t alpha = static_cast<uint32_t>(color.w);
	return (alpha << 24) + (blue << 16) + (green << 8) + red;
}

inline Color toColor(uint32_t abgr)
{
	const float a = ((abgr >> 24) & 0xFF) / 255.0f;
	const float b = ((abgr >> 16) & 0xFF) / 255.0f;
	const float g = ((abgr >> 8) & 0xFF) / 255.0f;
	const float r = (abgr & 0xFF) / 255.0f;
	return Color(r, g, b, a);
}

inline iColor toiColor(uint32_t abgr)
{
	const uint32_t a = ((abgr >> 24) & 0xFF);
	const uint32_t b = ((abgr >> 16) & 0xFF);
	const uint32_t g = ((abgr >> 8) & 0xFF);
	const uint32_t r = (abgr & 0xFF);
	return iColor(r, g, b, a);
}

inline int32_t roundTo(int32_t value,  int32_t to)
{
	return (value + to - 1) & ~ (to - 1);
}

template<typename T>
std::string to_binary_string(T value)
{
	std::size_t sz = sizeof(value) * CHAR_BIT;
	std::string ret(sz, ' ');
	while (sz--)
	{
		ret[sz] = '0' + (value & 1);
		value >>= 1;
	}

	return ret;
}

template<typename Iter, typename UnaryPredicate>
typename std::iterator_traits<Iter>::value_type findPtrByPredicate(Iter begin, Iter end, UnaryPredicate predicate)
{
	auto it = std::find_if(begin,  end, predicate);
	if (it != end)
	{
		return *it;
	}

	return nullptr;
}

inline const char* glErrorToString(GLenum error)
{
#define CASE_RETURN_STRING(name) \
	case name: return #name

	switch (error)
	{
		CASE_RETURN_STRING(GL_INVALID_ENUM);
		CASE_RETURN_STRING(GL_INVALID_VALUE);
		CASE_RETURN_STRING(GL_INVALID_OPERATION);
		CASE_RETURN_STRING(GL_STACK_OVERFLOW);
		CASE_RETURN_STRING(GL_STACK_UNDERFLOW);
		CASE_RETURN_STRING(GL_OUT_OF_MEMORY);
	default:
		return "Unknown Error";
	}
#undef CASE_RETURN_STRING
}