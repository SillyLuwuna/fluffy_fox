#pragma once

#include <cstdint>
#include <functional>

namespace util
{
	template<typename Signature>
	class MemberFunction;

	template<typename T, typename... Args>
	class MemberFunction<T(Args...)>
	{
		private:
			const std::function<T(Args...)> m_func;
			const std::uint64_t m_obj;

		public:
			template<typename O>
			MemberFunction(T (O::*member_func)(Args...), O* obj);

			uint64_t get_obj() const;

			T operator()(Args&&... args) const;

			bool operator==(const MemberFunction& other) const;
			size_t hash() const;
	};
}

namespace std
{
	template<typename Signature>
	struct hash<util::MemberFunction<Signature>>
	{
		std::size_t operator()(const util::MemberFunction<Signature>& func) const
		{
			return func.hash();
		}
	};
}

#include "member_function.ipp"
