
#include <core/action.h>

#include <includes.h>

using namespace input;

Action::Action() :
	m_type(0)
{
}

void Action::set(size_t hash, bool state)
{
	m_type  = hash;
	m_state = state;
}

void Action::set(const std::string& name, bool state)
{
	m_type  = std::hash<std::string>()(name);
	m_state = state;
}

void Action::set(const std::string& name, float m_value)
{
	m_type  = std::hash<std::string>()(name);
	m_value = m_value;
}

size_t Action::getHash() const
{
	return m_type;
}

bool input::operator==(const Action& a, const std::string& b)
{
	return (a.m_type == std::hash<std::string>()(toLower(b)));
}

bool input::operator!= (const Action &a, const std::string &b)
{
	return !(a == b);
}