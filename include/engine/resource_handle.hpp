#pragma once

#include <string>
namespace fluffy_fox
{
	class ResourceManager;

	template<typename T>
	class ResourceHandle
	{
	private:
		std::string resource_id_;
		ResourceManager* resource_manager_;

	public:
		ResourceHandle() :
			resource_id_(""),
			resource_manager_(nullptr)
		{ }

		ResourceHandle(const std::string& id, ResourceManager* manager) :
			resource_id_(id),
			resource_manager_(manager)
		{ }

		~ResourceHandle();

		T* get() const;
		bool is_valid() const;

		const std::string& get_id() const
		{
			return resource_id_;
		}

		T* operator->() const
		{
			return get();
		}

		T& operator*() const
		{
			return *get();
		}

		operator bool() const
		{
			return is_valid();
		}
	};
}

#include "engine/resource_manager.hpp"

namespace fluffy_fox
{
	template<typename T>
	T* ResourceHandle<T>::get() const
	{
		if (!resource_manager_) return nullptr;
		return resource_manager_->get_resource<T>(resource_id_);
	}

	template<typename T>
	bool ResourceHandle<T>::is_valid() const
	{
		return resource_manager_ && resource_manager_->has_resource<T>(resource_id_);
	}

	template<typename T>
	ResourceHandle<T>::~ResourceHandle()
	{
		if (!resource_manager_) return;
		resource_manager_->release<T>(resource_id_);
	}
}
