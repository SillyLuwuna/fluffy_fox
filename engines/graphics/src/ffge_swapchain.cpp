#include "ffge_swapchain.hpp"
#include "ffge_device.hpp"
#include "ffge_physicalDevice.hpp"
#include "ffge_queueFamilyIndices.hpp"
#include "ffge_swapchainSupportDetails.hpp"
#include "ffge_window.hpp"
#include "ffge_surface.hpp"
#include <algorithm>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace FluffyFox::Graphics
{
	Swapchain::Swapchain(const PhysicalDevice &physicalDevice, const Surface &surface, const Window &window, const Device &device)
		: m_device(device)
	{
		SwapchainSupportDetails swapchainSupport = physicalDevice.querySwapchainSupport(surface);

		VkSurfaceFormatKHR surfaceFormat = chooseSwapchainSurfaceFormat(swapchainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapchainPresentMode(swapchainSupport.presentModes);
		VkExtent2D extent = chooseSwapchainExtent(swapchainSupport.capabilities, window);

		uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
		if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount)
		{
			// maxImageCount == 0 means there is no maximum
			imageCount = swapchainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface.getHandle();
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // INFO might have to be used VK_IMAGE_USAGE_TRANSFER_DST_BIT for post-processing

		QueueFamilyIndices indices = physicalDevice.findQueueFamilies(surface);
		uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};
		if (indices.graphicsFamily != indices.presentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(m_device.getHandle(), &createInfo, nullptr, &m_swapchain) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create swapchain!");
		}
	}

	Swapchain::~Swapchain()
	{
		vkDestroySwapchainKHR(m_device.getHandle(), m_swapchain, nullptr);
	}

	VkSurfaceFormatKHR Swapchain::chooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
	{
		for (const auto &availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		// INFO might wanna choose between the available color formats instead of choosing an arbitrary one
		return availableFormats[0];
	}

	VkPresentModeKHR Swapchain::chooseSwapchainPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
	{
		for (const auto &availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D Swapchain::chooseSwapchainExtent(const VkSurfaceCapabilitiesKHR &capabilities, const Window &window)
	{
		VkExtent2D result;
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			result = capabilities.currentExtent;
		}
		else
		{
			int width, height;
			glfwGetFramebufferSize(window.getHandle(), &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			result = actualExtent;
		}
		return result;
	}
}
