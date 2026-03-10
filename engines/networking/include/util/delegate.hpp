#pragma once

#include "util/named_function.hpp"
#include <memory>
#include <unordered_set>

namespace util
{
	template<typename RetType, typename ArgType>
	class Delegate
	{
		private:
			std::unordered_set<NamedFunction<RetType(ArgType)>, NamedFunctionHash> m_subscribers;
			std::vector<NamedFunction<RetType(ArgType)>> m_subscribers_lazy;
			uint16_t m_num_deletions;

		public:
			Delegate();

			void subscribe(NamedFunction<RetType(ArgType)> fun);
			void unsubscribe(NamedFunction<RetType(ArgType)> fun);
			void invoke(ArgType arg);

			void clean();
			void clean_if_necessary();

			Delegate& operator+=(NamedFunction<RetType(ArgType)> other);
			Delegate& operator-=(NamedFunction<RetType(ArgType)> other);
	};
}

#include "util/delegate.ipp"
