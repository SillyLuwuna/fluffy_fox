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

	const VkPhysicalDevice &PhysicalDevice::getHandle() const
	{
		return m_physicalDevice;
	}

	QueueFamilyIndices PhysicalDevice::findQueueFamilies() const
	{
		QueueFamilyIndices indices;

		std::vector<VkQueueFamilyProperties> queueFamilies = getQueueFamilyProperties();

		int i = 0;
		for (const VkQueueFamilyProperties queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}

			if (indices.isComplete())
			{
				break;
			}
			i++;
		}

		return indices;
	}

	std::vector<VkQueueFamilyProperties> PhysicalDevice::getQueueFamilyProperties() const
	{
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilies.data());
		return queueFamilies;
	}
}
