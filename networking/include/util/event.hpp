#pragma once

#include "util/member_function.hpp"
#include <cstdint>
#include <unordered_set>

namespace util
{
	template <typename ...Args>
	using EventFunction = MemberFunction<void(Args...)>;

	template<typename ...Args>
	class Event
	{
	private:
		std::unordered_set<EventFunction<Args...>> m_subscribers;

	public:
		Event() :
			m_subscribers()
		{
		}

		void subscribe(EventFunction<Args...> fun)
		{
			m_subscribers.emplace(fun);
		}

		void unsubscribe(EventFunction<Args...> fun)
		{
			m_subscribers.erase(fun);
		}

		void invoke(Args... arg)
		{
			for (const EventFunction<Args...>& func : m_subscribers)
			{
				func(std::move(arg...));
			}
		}

		void operator+=(EventFunction<Args...> other)
		{
			subscribe(other);
		}

		void operator-=(EventFunction<Args...> other)
		{
			unsubscribe(other);
		}
	};
}
