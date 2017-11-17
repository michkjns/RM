
#pragma once

#include <string>
#include <functional>

namespace input
{
	enum ButtonState : unsigned char
	{
		Release = 0,
		Press,
		Repeat,
	};

	class Action
	{
	public:
		Action();
		~Action() {}

		void set(size_t hashedName, ButtonState inputEvent);
		void set(size_t hashedName, float value);
		void set(const std::string& name, ButtonState inputEvent);
		void set(const std::string& name, float value);

		size_t getHash() const;
		float  getValue() const;

	protected:
		size_t m_hashedName;

		union
		{
			ButtonState m_inputEvent;
			float m_value;
		};

	public:
		friend bool operator== (const Action& a, const Action& b);
		friend bool operator!= (const Action& a, const Action& b);
		friend bool operator== (const Action& a, const std::string& b);
		friend bool operator!= (const Action& a, const std::string& b);
	};

}; // namespace input
