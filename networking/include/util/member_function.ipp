#pragma once
#include "util/member_function.hpp"
#include "util/hasher.hpp"

using namespace util;


template<typename T, typename... Args>
template<typename O>
MemberFunction<T(Args...)>::MemberFunction(T (O::*member_func)(Args...), O* obj) :
	m_func([obj, member_func](Args... args) -> T {
		return (obj->*member_func)(std::forward<Args>(args)...);
	}),
	m_obj((uint64_t)(void *)obj)
{
}

template<typename T, typename... Args>
T MemberFunction<T(Args...)>::operator()(Args&&... args) const
{
	return m_func(std::forward<Args>(args)...);
}

template<typename T, typename... Args>
uint64_t MemberFunction<T(Args...)>::get_obj() const
{
	return m_obj;
}

template<typename T, typename... Args>
bool MemberFunction<T(Args...)>::operator==(const MemberFunction& other) const
{
	return (m_func.target_type().name() == other.m_func.target_type().name()) && (m_obj == other.m_obj);
}

template<typename T, typename... Args>
size_t MemberFunction<T(Args...)>::hash() const
{
	size_t seed = 0;
	hash_combine(seed, m_func.target_type().name());
	hash_combine(seed, m_obj);
	return seed;
}
