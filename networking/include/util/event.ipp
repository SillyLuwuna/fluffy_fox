// #pragma once
// #include "util/event.hpp"
//
// using namespace util;
//
// template<typename ...ArgType>
// Event<ArgType...>::Event() :
// 	m_subscribers(),
// 	m_subscribers_lazy(),
// 	m_num_deletions(0)
// {
//
// }
//
// template<typename ...ArgType>
// void Event<ArgType...>::subscribe(MemberFunction<void(ArgType...)> fun)
// {
// 	if (m_subscribers.contains(fun))
// 	{
// 		return;
// 	}
//
// 	m_subscribers.emplace(fun);
// 	m_subscribers_lazy.emplace_back(fun);
// }
//
// template<typename ...ArgType>
// void Event<ArgType...>::unsubscribe(MemberFunction<void(ArgType...)> fun)
// {
// 	if (!m_subscribers.contains(fun))
// 	{
// 		return;
// 	}
//
// 	m_subscribers.erase(fun);
// 	m_num_deletions++;
//
// 	clean();
// 	// clean_if_necessary();
// }
//
// template<typename ...ArgType>
// void Event<ArgType...>::clean()
// {
// 	std::vector<MemberFunction<void(ArgType...)>> new_vect;
// 	new_vect.reserve(m_subscribers_lazy.size());
//
// 	for(int i = 0; i < m_subscribers_lazy.size(); i++)
// 	{
// 		MemberFunction<void(ArgType...)> curr_subscriber = m_subscribers_lazy[i];
// 		if (m_subscribers.contains(curr_subscriber))
// 		{
// 			new_vect.emplace_back(curr_subscriber);
// 		}
// 	}
//
// 	m_subscribers_lazy = new_vect;
// 	m_num_deletions = 0;
// }
//
// template<typename ...ArgType>
// void Event<ArgType...>::clean_if_necessary()
// {
// 	float ratio = m_num_deletions / m_subscribers_lazy.size();
// 	if (ratio >= 0.5)
// 	{
// 		clean();
// 	}
// }
//
// template<typename ...ArgType>
// void Event<ArgType...>::invoke(ArgType... arg)
// {
// 	for(int i = 0; i < m_subscribers_lazy.size(); i++)
// 	{
// 		MemberFunction<void(ArgType...)> curr_subscriber = m_subscribers_lazy[i];
// 		if (!m_subscribers.contains(curr_subscriber))
// 		{
// 			continue;
// 		}
// 		curr_subscriber(arg...);
// 	}
// }
//
// template<typename ...ArgType>
// Event<ArgType...>& Event<ArgType...>::operator+=(MemberFunction<void(ArgType...)> other)
// {
// 	subscribe(other);
// }
//
// template<typename ...ArgType>
// Event<ArgType...>& Event<ArgType...>::operator-=(MemberFunction<void(ArgType...)> other)
// {
// 	unsubscribe(other);
// }
