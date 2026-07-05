#pragma once

#include "hashing/hasher.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>

namespace fluffy_fox
{
	template<typename Signature>
	class MemberFunction;

	template<typename T, typename... Args>
	class MemberFunction<T(Args...)>
	{
		private:
			const std::function<T(Args...)> func_;
			const std::uint64_t obj_;

		public:
			template<typename O>
			MemberFunction(T (O::*member_func)(Args...), O* obj) :
				func_([obj, member_func](Args... args) -> T { return (obj->*member_func)(std::forward<Args>(args)...); }),
				obj_((uint64_t)(void *)obj)
			{
			}

			T operator()(Args&&... args) const
			{
				return func_(std::forward<Args>(args)...);
			}

			uint64_t get_obj() const
			{
				return obj_;
			}

			bool operator==(const MemberFunction& other) const
			{
				return (func_.target_type().name() == other.func_.target_type().name()) && (obj_ == other.obj_);
			}

			std::size_t hash() const
			{
				std::size_t seed = 0;
				hash_combine(seed, func_.target_type().name());
				hash_combine(seed, obj_);
				return seed;
			}
	};
}

namespace std
{
	template<typename Signature>
	struct hash<fluffy_fox::MemberFunction<Signature>>
	{
		std::size_t operator()(const fluffy_fox::MemberFunction<Signature>& func) const
		{
			return func.hash();
		}
	};
}
