#pragma once

#include <fluff/types.hpp>
using namespace fluff::types;

#include "engine/component_type_id_system.hpp"

namespace fluffy_fox
{
	class Entity;

	class Component
	{
	public:
		enum class State
		{
			Uninitialized,
			Initializing,
			Active,
			Destroying,
			Destroyed
		};

	private:
		State state_;
		Entity* owner_;

	public:
		Component();
		virtual ~Component();

		void initialize();
		void destroy();
		bool is_active() const;

		void set_owner(Entity* entity);
		Entity* get_owner() const;

		template<typename T>
		static u64 get_type_id()
		{
			return ComponentTypeIDSystem::get_type_id<T>();
		}
	
	protected:
		virtual void on_initialize();
		virtual void on_destroy();
		virtual void update(float delta_time);
		virtual void render();

		friend class Entity; // allow Entity to call protected methods
	};
}
