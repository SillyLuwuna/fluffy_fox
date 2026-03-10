#pragma once
#include "util/named_function.hpp"

using namespace util;

template<typename T>
NamedFunction<T>::NamedFunction(const std::function<T>& func, const std::string& name) :
	m_func(func),
	m_name(name)
{

}

template<typename T>
const std::string& NamedFunction<T>::get_name() const
{
	return m_name;
}

template<typename T>
template<typename... Args>
typename std::result_of<std::function<T>(Args...)>::type
NamedFunction<T>::operator()(Args&&... args)
{
	return m_func(std::forward<Args>(args)...);
}

template<typename T>
bool NamedFunction<T>::operator==(const NamedFunction& other) const
{
	return m_name == other.m_name;
}

template<typename T>
std::size_t NamedFunctionHash::operator()(const NamedFunction<T>& func) const
{
	return std::hash<std::string>{}(func.get_name());
}
