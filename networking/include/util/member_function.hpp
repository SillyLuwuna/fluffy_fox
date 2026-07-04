#pragma once

#include "util/hasher.hpp"

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
			MemberFunction(T (O::*member_func)(Args...), O* obj) :
				m_func([obj, member_func](Args... args) -> T { return (obj->*member_func)(std::forward<Args>(args)...); }),
				m_obj((uint64_t)(void *)obj)
			{
			}

			T operator()(Args&&... args) const
			{
				return m_func(std::forward<Args>(args)...);
			}

			uint64_t get_obj() const
			{
				return m_obj;
			}

			bool operator==(const MemberFunction& other) const
			{
				return (m_func.target_type().name() == other.m_func.target_type().name()) && (m_obj == other.m_obj);
			}

			size_t hash() const
			{
				size_t seed = 0;
				hash_combine(seed, m_func.target_type().name());
				hash_combine(seed, m_obj);
				return seed;
			}
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

// #include "member_function.ipp"
