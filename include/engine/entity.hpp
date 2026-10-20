#pragma once

#include <memory>
#include <string>
#include <vector>
#include <fluff.hpp>
using namespace fluff::types;

#include "engine/component.hpp"

namespace fluffy_fox
{
	class Entity
	{
	private:
		std::string name_;
		bool active_;
		std::vector<std::unique_ptr<Component>> components_;
		std::unordered_map<u64, Component*> component_map_;

	public:
		explicit Entity(const std::string& name);

		const std::string& get_name() const;
		bool is_active() const;
		void set_active(bool is_active);

		void initialize();
		void update(float delta_time);
		void render();

		template<typename T, typename... Args>
		T* add_component(Args&&... args)
		{
			static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

			u64 component_type_id = Component::get_type_id<T>();

			auto it = component_map_.find(component_type_id);
			if (it != component_map_.end())
			{
				return static_cast<T*>(it->second);
			}

			std::unique_ptr<T> component = std::make_unique<T>(std::forward<Args>(args)...);
			component->set_owner(this);
			T* component_ptr = component.get();

			component_map_[component_type_id] = component_ptr;
			components_.push_back(std::move(component));

			return component_ptr;
		}

		template<typename T>
		T* get_component()
		{
			u64 component_type_id = Component::get_type_id<T>();

			auto it = component_map_.find(component_type_id);
			if (it != component_map_.end())
			{
				return static_cast<T*>(it->second);
			}

			return nullptr;
		}

		template<typename T>
		bool remove_component()
		{
			u64 component_type_id = Component::get_type_id<T>();

			auto it = component_map_.find(component_type_id);
			if (it == component_map_.end()) return false;

			Component* component_ptr = it->second;
			component_map_.erase(it);

			for (auto comp_it = components_.begin(); comp_it != components_.end(); comp_it++)
			{
				if (comp_it->get() == component_ptr)
				{
					components_.erase(comp_it);
					return true;
				}
			}

			return false;
		}
	};
}
