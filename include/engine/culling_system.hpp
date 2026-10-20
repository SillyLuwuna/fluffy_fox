#pragma once

#include "engine/entity.hpp"
#include <vector>

namespace fluffy_fox
{
	class CullingSystem
	{
		private:
			// Camera* camera_;
			std::vector<Entity*> visible_entities_;

		public:
			// explicit CullingSystem(Camera* camera);
			//
			// void set_camera(Camera* camera);
			void cull_scene(const std::vector<Entity*>& all_entities);
			const std::vector<Entity*>& get_visible_entities() const;
	};
}
