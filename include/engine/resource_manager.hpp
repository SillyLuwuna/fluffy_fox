#pragma once

#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <fluff/types.hpp>
using namespace fluff::types;

#include "engine/resource.hpp"

namespace fluffy_fox
{
	template <typename T>
	class ResourceHandle;

	class ResourceManager
	{
	private:
		struct ResourceData
		{
			std::shared_ptr<Resource> resource;
			i32 ref_count;
		};

		std::unordered_map<
			std::type_index,
			std::unordered_map<std::string, ResourceData>
		> resources_;

		template <typename T>
		T* get_resource(const std::string& resource_id)
		{
			auto& type_resources = resources_[std::type_index(typeid(T))];
			auto it = type_resources.find(resource_id);

			if (it != type_resources.end())
			{
				return static_cast<T*>(it->second.resource.get());
			}

			return nullptr;
		}

		template <typename T>
		void release(const std::string& resource_id);

		template <typename T>
		void increment_ref_count(const std::string& resource_id)
		{
			resources_[std::type_index(typeid(T))][resource_id].ref_count++;
		}

		// FIXME use this in destructor
		void unload_all()
		{
			for (auto& [type, type_resources] : resources_)
			{
				for (auto& [id, resource_data] : type_resources)
				{
					resource_data.resource->unload();
				}
				type_resources.clear();
			}
		}

		template <typename T>
		friend class ResourceHandle;

	public:
		~ResourceManager()
		{
			unload_all();
		}

		template <typename T, typename... Args>
		ResourceHandle<T> load(const std::string& resource_id, Args&&... args);

		// template <typename T>
		// ResourceHandle<T> get(const std::string& resource_id);

		template <typename T>
		bool has_resource(const std::string& resource_id)
		{
			return resources_[std::type_index(typeid(T))].contains(resource_id);
		}
	};
}

#include "engine/resource_handle.hpp"

namespace fluffy_fox
{
	template <typename T, typename... Args>
	ResourceHandle<T> ResourceManager::load(const std::string& resource_id, Args&&... args)
	{
		static_assert(std::is_base_of<Resource, T>::value, "T must derive from Resource");

		auto& type_resources = resources_[std::type_index(typeid(T))];
		auto it = type_resources.find(resource_id);

		if (it != type_resources.end())
		{
			type_resources[resource_id].ref_count++;
			return ResourceHandle<T>(resource_id, this);
		}

		std::shared_ptr<T> resource = std::make_shared<T>(resource_id, std::forward<Args>(args)...);
		if (!resource->load())
		{
			return ResourceHandle<T>();
		}

		type_resources[resource_id].resource = resource;
		type_resources[resource_id].ref_count = 1;

		return ResourceHandle<T>(resource_id, this);
	}

	template <typename T>
	void ResourceManager::release(const std::string& resource_id)
	{
		auto& type_resources = resources_[std::type_index(typeid(T))];
		auto it = type_resources.find(resource_id);

		if (it == type_resources.end()) return;

		ResourceData& resource_data = it->second;

		resource_data.ref_count--;

		if (resource_data.ref_count > 0) return;

		resource_data.resource->unload();
		type_resources.erase(it);
	}

	// template <typename T>
	// ResourceHandle<T> ResourceManager::get(const std::string& resource_id)
	// {
	// 	static_assert(std::is_base_of<Resource, T>::value, "T must derive from Resource");
	//
	// 	auto& type_resources = resources_[std::type_index(typeid(T))];
	// 	auto it = type_resources.find(resource_id);
	//
	// 	if (it == type_resources.end())
	// 	{
	// 		return ResourceHandle<T>();
	// 	}
	//
	// 	type_resources[resource_id].ref_count++;
	// 	return ResourceHandle<T>(resource_id, this);
	//
	// 	return nullptr;
	// }
}
