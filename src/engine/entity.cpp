#include "engine/entity.hpp"

using namespace fluffy_fox;

Entity::Entity(const std::string& name) :
	name_(name),
	active_(true)
{ }

const std::string& Entity::get_name() const
{
	return name_;
}

bool Entity::is_active() const
{
	return active_;
}

void Entity::set_active(bool is_active)
{
	active_ = is_active;
}

void Entity::initialize()
{
	for (auto& component : components_)
	{
		component->initialize();
	}
}

void Entity::update(float delta_time)
{
	if (!active_) return;

	for (auto& component : components_)
	{
		component->update(delta_time);
	}
}

void Entity::render()
{
	if (!active_) return;

	for (auto& component : components_)
	{
		component->render();
	}
}
