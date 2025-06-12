#include "ffge_device.hpp"
#include <stdexcept>

namespace FluffyFox::Graphics
{
	Device::Device(PhysicalDevice &pd) : m_physicalDevice(pd)
	{
		QueueFamilyIndices indices = m_physicalDevice.findQueueFamilies();
		initLogicDevice(indices);
		initGraphicsQueue(indices);
	}

	void Device::initLogicDevice(QueueFamilyIndices &indices)
	{

		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
		queueCreateInfo.queueCount = 1;
		float queuePriority = 1.0f;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = &queueCreateInfo;
		createInfo.queueCreateInfoCount = 1;

		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = 0;

		if (vkCreateDevice(m_physicalDevice.getHandle(), &createInfo, nullptr, &m_device) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create logical device!");
		}
	}

	void Device::initGraphicsQueue(QueueFamilyIndices &indices)
	{
		vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
	}

	Device::~Device()
	{
		vkDestroyDevice(m_device, nullptr);
	}
}
