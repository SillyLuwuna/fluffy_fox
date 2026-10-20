#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "engine/component.hpp"

namespace fluffy_fox
{
	class TransformComponent : public Component
	{
		private:
			glm::vec3 position_;
			glm::quat rotation_;
			glm::vec3 scale_;

			mutable glm::mat4 transform_matrix_;
			mutable bool transform_dirty_;

			void update_transform_matrix() const;

		public:
			TransformComponent();

			void set_position(const glm::vec3& position);
			void set_rotation(const glm::quat& rotation);
			void set_scale(const glm::vec3& scale);
			const glm::vec3& get_position() const;
			const glm::quat& get_rotation() const;
			const glm::vec3& get_scale() const;
			glm::mat4 get_transform_matrix() const;
	};
}
