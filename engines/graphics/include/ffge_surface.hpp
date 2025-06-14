#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace FluffyFox::Graphics
{
	class Window;
	class Instance;

	class Surface
	{
		public:
			Surface(const Instance &instance, const Window &window);
			~Surface();

			Surface(const Surface &) = delete;
			Surface &operator=(const Surface &) = delete;

			inline const VkSurfaceKHR &getHandle() const
			{
				return m_surface;
			}

		private:
			VkSurfaceKHR m_surface;
			const Instance &m_instance;
	};
}
