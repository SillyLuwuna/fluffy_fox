#include "engine/components/transform_component.hpp"

using namespace fluffy_fox;

TransformComponent::TransformComponent() :
	position_(glm::vec3(0.0f)),
	rotation_(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)),
	scale_(glm::vec3(1.0f)),
	transform_matrix_(glm::mat4(1.0f)),
	transform_dirty_(true)
{ }

void TransformComponent::set_position(const glm::vec3& position)
{
	position_ = position;
	transform_dirty_ = true;
}

void TransformComponent::set_rotation(const glm::quat& rotation)
{
	rotation_ = rotation;
	transform_dirty_ = true;
}

void TransformComponent::set_scale(const glm::vec3& scale)
{
	scale_ = scale;
	transform_dirty_ = true;
}

const glm::vec3& TransformComponent::get_position() const
{
	return position_;
}

const glm::quat& TransformComponent::get_rotation() const
{
	return rotation_;
}

const glm::vec3& TransformComponent::get_scale() const
{
	return scale_;
}

glm::mat4 TransformComponent::get_transform_matrix() const
{
	if (transform_dirty_)
	{
		update_transform_matrix();
		transform_dirty_ = false;
	}

	return transform_matrix_;
}

void TransformComponent::update_transform_matrix() const
{
	glm::mat4 translation_matrix = glm::translate(glm::mat4(1.0f), position_);
	glm::mat4 rotation_matrix = glm::mat4_cast(rotation_);
	glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0f), scale_);

	transform_matrix_ = translation_matrix * rotation_matrix * scale_matrix;
}
