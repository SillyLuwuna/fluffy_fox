#pragma once

#include "events/event.hpp"
#include <mutex>

namespace fluffy_fox
{
	template<typename ...Args>
	class ThreadSafeEvent : public Event<Args...>
	{
	private:
		std::mutex event_mutex_;

	public:
		ThreadSafeEvent() :
			Event<Args...>(),
			event_mutex_()
		{
		}

		void subscribe(EventFunction<Args...>&& fun) override
		{
			std::lock_guard lock(event_mutex_);

			Event<Args...>::subscribe(std::move(fun));
		}

		virtual void unsubscribe(EventFunction<Args...>&& fun) override
		{
			std::lock_guard lock(event_mutex_);

			Event<Args...>::unsubscribe(std::move(fun));
		}

		// does not guarantee order of execution
		virtual void invoke(Args... arg) override
		{
			std::lock_guard lock(event_mutex_);

			Event<Args...>::invoke(std::forward<Args>(arg)...);
		}
	};
}
