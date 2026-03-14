#pragma once

#include "util/named_function.hpp"
#include <memory>
#include <unordered_set>

namespace util
{
	template<typename ...ArgType>
	class Event
	{
		private:
			std::unordered_set<NamedFunction<void(ArgType...)>, NamedFunctionHash> m_subscribers;
			std::vector<NamedFunction<void(ArgType...)>> m_subscribers_lazy;
			uint16_t m_num_deletions;

			void clean();
			void clean_if_necessary();

		public:
			Event();

			void subscribe(NamedFunction<void(ArgType...)> fun);
			void unsubscribe(NamedFunction<void(ArgType...)> fun);
			void invoke(ArgType... arg);

			Event& operator+=(NamedFunction<void(ArgType...)> other);
			Event& operator-=(NamedFunction<void(ArgType...)> other);
	};
}

#include "util/event.ipp"
