#include "ffge_instance.hpp"
#include "ffge_window.hpp"
#include "ffge_surface.hpp"
#include <stdexcept>

namespace FluffyFox::Graphics
{
	Surface::Surface(const Instance &instance, const Window &window) : m_instance(instance)
	{
		if (glfwCreateWindowSurface(instance.getHandle(), window.getHandle(), nullptr, &m_surface) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create window surface!");
		}
	}

	Surface::~Surface()
	{
		vkDestroySurfaceKHR(m_instance.getHandle(), m_surface, nullptr); // must be done before instance destruction
	}
}
