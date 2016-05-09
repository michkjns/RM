
#pragma once

#include <string>
#include <functional>

namespace input
{
	class Action
	{
	public:
		Action();
		~Action() {}

		void set(size_t hash, bool state);
		void set(const std::string& name, bool state);
		void set(const std::string& name, float value);

		size_t getHash() const;

	protected:
		size_t m_type;
		union
		{
			bool  m_state;
			float m_value;
		};

	public:
		friend bool operator== (const Action &a, const std::string &b);
		friend bool operator!= (const Action &a, const std::string &b);
	};

}; // namespace input
