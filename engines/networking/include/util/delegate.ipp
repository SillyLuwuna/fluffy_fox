#pragma once
#include "util/delegate.hpp"

using namespace util;

template<typename ArgType>
Delegate<ArgType>::Delegate() :
	m_subscribers(),
	m_subscribers_lazy(),
	m_num_deletions(0)
{

}

template<typename ArgType>
void Delegate<ArgType>::subscribe(NamedFunction<void(ArgType)> fun)
{
	if (m_subscribers.contains(fun))
	{
		return;
	}

	m_subscribers.emplace(fun);
	m_subscribers_lazy.emplace_back(fun);
}

template<typename ArgType>
void Delegate<ArgType>::unsubscribe(NamedFunction<void(ArgType)> fun)
{
	if (!m_subscribers.contains(fun))
	{
		return;
	}

	m_subscribers.erase(fun);
	m_num_deletions++;

	clean();
	// clean_if_necessary();
}

template<typename ArgType>
void Delegate<ArgType>::clean()
{
	std::vector<NamedFunction<void(ArgType)>> new_vect;
	new_vect.reserve(m_subscribers_lazy.size());

	for(int i = 0; i < m_subscribers_lazy.size(); i++)
	{
		NamedFunction<void(ArgType)> curr_subscriber = m_subscribers_lazy[i];
		if (m_subscribers.contains(curr_subscriber))
		{
			new_vect.emplace_back(curr_subscriber);
		}
	}

	m_subscribers_lazy = new_vect;
	m_num_deletions = 0;
}

template<typename ArgType>
void Delegate<ArgType>::clean_if_necessary()
{
	float ratio = m_num_deletions / m_subscribers_lazy.size();
	if (ratio >= 0.5)
	{
		clean();
	}
}

template<typename ArgType>
void Delegate<ArgType>::invoke(ArgType arg)
{
	for(int i = 0; i < m_subscribers_lazy.size(); i++)
	{
		NamedFunction<void(ArgType)> curr_subscriber = m_subscribers_lazy[i];
		if (!m_subscribers.contains(curr_subscriber))
		{
			continue;
		}
		curr_subscriber(arg);
	}
}

template<typename ArgType>
Delegate<ArgType>& Delegate<ArgType>::operator+=(NamedFunction<void(ArgType)> other)
{
	subscribe(other);
}

template<typename ArgType>
Delegate<ArgType>& Delegate<ArgType>::operator-=(NamedFunction<void(ArgType)> other)
{
	unsubscribe(other);
}
