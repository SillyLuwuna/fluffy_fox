#pragma once

#include <memory>
#include <string>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace FluffyFox::Graphics
{
	class Surface;
	class Instance;

	class Window
	{
		public:
			Window(const int width, const int height, const std::string &title);
			~Window();

			Window(const Window &) = delete;
			Window &operator=(const Window &) = delete;

			bool shouldClose();

			inline GLFWwindow *getHandle() const
			{
				return m_window;
			}

			std::unique_ptr<Surface> createSurface(Instance &instance);

		private:
			const int m_width;
			const int m_height;
			std::string m_title;
			GLFWwindow *m_window;

			void initWindow();
	};
}
