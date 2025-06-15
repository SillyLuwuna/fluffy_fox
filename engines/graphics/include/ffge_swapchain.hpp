#pragma once

#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace FluffyFox::Graphics
{
	class Window;
	class Device;
	class Surface;
	class PhysicalDevice;

	class Swapchain
	{
		public:
			Swapchain(const PhysicalDevice &physicalDevice, const Surface &surface, const Window &window, const Device &device);
			~Swapchain();

			Swapchain(const Swapchain &) = delete;
			Swapchain &operator=(const Swapchain &) = delete;

		private:
			VkSurfaceFormatKHR chooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
			VkPresentModeKHR chooseSwapchainPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
			VkExtent2D chooseSwapchainExtent(const VkSurfaceCapabilitiesKHR &capabilities, const Window &window);
			VkSwapchainKHR m_swapchain;
			const Device &m_device;
	};
}
