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

		ResourceHandle(const ResourceHandle& other);
		ResourceHandle& operator=(const ResourceHandle& other);
		~ResourceHandle();

		ResourceHandle(ResourceHandle&& other) :
			resource_id_(std::move(other.resource_id_)),
			resource_manager_(other.resource_manager_)
		{ }

		ResourceHandle& operator=(ResourceHandle&& other)
		{
			resource_id_ = std::move(other.resource_id_);
			resource_manager_ = other.resource_manager_;
			return *this;
		}

		ResourceHandle& operator=(std::nullptr_t null);

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

	template<typename T>
	ResourceHandle<T>::ResourceHandle(const ResourceHandle<T>& other) :
		resource_id_(other.resource_id_),
		resource_manager_(other.resource_manager_)
	{
		if (!resource_manager_) return;
		resource_manager_->increment_ref_count<T>(resource_id_);
	}

	template<typename T>
	ResourceHandle<T>& ResourceHandle<T>::operator=(const ResourceHandle<T>& other)
	{
		resource_id_ = other.resource_id_;
		resource_manager_ = other.resource_manager_;

		if (!resource_manager_) return *this;
		resource_manager_->increment_ref_count<T>(resource_id_);

		return *this;
	}

	template<typename T>
	ResourceHandle<T>& ResourceHandle<T>::operator=(std::nullptr_t)
	{
		if (!resource_manager_) return *this;

		resource_manager_->release<T>(resource_id_);

		return *this;
	}
}
