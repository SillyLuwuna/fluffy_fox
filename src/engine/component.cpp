#include "engine/component.hpp"

using namespace fluffy_fox;

Component::Component() :
	state_(State::Uninitialized),
	owner_(nullptr)
{ }

Component::~Component()
{
	if (state_ == State::Destroyed) return;

	on_destroy();
	state_ = State::Destroyed;
}

void Component::initialize()
{
	if (state_ != State::Uninitialized) return;

	state_ = State::Initializing;
	on_initialize();
	state_ = State::Active;
}

void Component::destroy()
{
	if (state_ != State::Active) return;

	state_ = State::Destroying;
	on_destroy();
	state_ = State::Destroyed;
}

bool Component::is_active() const
{
	return state_ == State::Active;
}

void Component::on_initialize() { }
void Component::on_destroy() { }
void Component::update(float delta_time) { }
void Component::render() { }

void Component::set_owner(Entity* entity)
{
	owner_ = entity;
}

Entity* Component::get_owner() const
{
	return owner_;
}
