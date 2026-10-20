#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "engine/component.hpp"

namespace fluffy_fox
{
	class CameraComponent : public Component
	{
		private:
			float field_of_view_;
			float aspect_ratio_;
			float near_plane_;
			float far_plane_;

			glm::mat4 view_matrix_;

			mutable glm::mat4 projection_matrix_;
			mutable bool projection_dirty_;

		public:
			CameraComponent();

			// set_perspective(float fov, float aspect_ratio, float near_plane, float far_plane)
			// get_view_matrix() const
			// get_projection_matrix() const
	};
}
