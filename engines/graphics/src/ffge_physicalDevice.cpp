#include "ffge_surface.hpp"
#include <iostream>
#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "ffge_physicalDevice.hpp"

namespace FluffyFox::Graphics
{
	PhysicalDevice::PhysicalDevice(VkPhysicalDevice pd)
		: m_physicalDevice(pd),
		m_properties{},
		m_features{}
	{
		if (pd != VK_NULL_HANDLE)
			cacheValues();
	}

	void PhysicalDevice::cacheValues()
	{
		vkGetPhysicalDeviceProperties(m_physicalDevice, &m_properties);
		vkGetPhysicalDeviceFeatures(m_physicalDevice, &m_features);
	}

	PhysicalDevice::PhysicalDevice() : PhysicalDevice(VK_NULL_HANDLE) { }

	PhysicalDevice::~PhysicalDevice() { }

	QueueFamilyIndices PhysicalDevice::findQueueFamilies(const Surface &surface) const
	{
		std::vector<VkQueueFamilyProperties> queueFamilies = getQueueFamilyProperties();

		int i = 0;
		QueueFamilyIndices indices;
		for (const VkQueueFamilyProperties queueFamily : queueFamilies)
		{
			if (hasGraphicsQueue(queueFamily)) { indices.graphicsFamily = i; }

			if (hasPresentQueue(i, surface)) { indices.presentFamily = i; }

			if (indices.isComplete()) { break; }
			i++;
		}

		return indices;
	}

	bool PhysicalDevice::hasPresentQueue(int index, const Surface &surface) const
	{
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, index, surface.getHandle(), &presentSupport);
		return presentSupport;
	}

	bool PhysicalDevice::hasGraphicsQueue(const VkQueueFamilyProperties &queueFamily) const
	{
		return queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT;
	}

	std::vector<VkQueueFamilyProperties> PhysicalDevice::getQueueFamilyProperties() const
	{
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilies.data());
		return queueFamilies;
	}

	std::vector<VkExtensionProperties> PhysicalDevice::getAvailableExtensions() const
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, availableExtensions.data());

		return availableExtensions;
	}
}
