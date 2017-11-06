
#include <core/action.h>

#include <common.h>
#include <utility.h>

using namespace input;

Action::Action() :
	m_type(0)
{
}

void Action::set(size_t hash, bool state)
{
	m_type  = hash;
	m_state = state;
//	m_isLocalOnly = isLocalOnly;
}

void input::Action::set(size_t hash, float value)
{
	m_type  = hash;
	m_value = value;
}

void Action::set(const std::string& name, bool state)
{
	m_type  = std::hash<std::string>()(name);
	m_state = state;
}

void Action::set(const std::string& name, float value)
{
	m_type  = std::hash<std::string>()(name);
	m_value = value;
}

size_t Action::getHash() const
{
	return m_type;
}

float Action::getValue() const
{
	return m_value;
}

bool input::operator==(const Action& a, const Action& b)
{
	return (a.getHash() == b.getHash() && a.getValue() == b.getValue());
}

bool input::operator!=(const Action& a, const Action& b)
{
	return !(a == b);
}

bool input::operator==(const Action& a, const std::string& b)
{
	return (a.m_type == std::hash<std::string>()(toLower(b)));
}

bool input::operator!= (const Action &a, const std::string &b)
{
	return !(a == b);
}