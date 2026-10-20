#include "engine/components/camera_component.hpp"

using namespace fluffy_fox;

CameraComponent::CameraComponent() :
	// TODO make into actual constructor
	field_of_view_(45.0f),
	aspect_ratio_(16.0f / 9.0f),
	near_plane_(0.1f),
	far_plane_(1000.0f),
	view_matrix_(glm::mat4(1.0f)),
	projection_matrix_(1.0f),
	projection_dirty_(true)
{ }
