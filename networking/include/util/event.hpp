// #pragma once
//
// #include "util/named_function.hpp"
// #include <cstdint>
// #include <unordered_set>
//
// namespace util
// {
// 	template<typename ...ArgType>
// 	class Event
// 	{
// 		private:
// 			std::unordered_set<MemberFunction<void(ArgType...)>, NamedFunctionHash> m_subscribers;
// 			std::vector<MemberFunction<void(ArgType...)>> m_subscribers_lazy;
// 			uint16_t m_num_deletions;
//
// 			void clean();
// 			void clean_if_necessary();
//
// 		public:
// 			Event();
//
// 			void subscribe(MemberFunction<void(ArgType...)> fun);
// 			void unsubscribe(MemberFunction<void(ArgType...)> fun);
// 			void invoke(ArgType... arg);
//
// 			Event& operator+=(MemberFunction<void(ArgType...)> other);
// 			Event& operator-=(MemberFunction<void(ArgType...)> other);
// 	};
// }
//
// #include "util/event.ipp"
