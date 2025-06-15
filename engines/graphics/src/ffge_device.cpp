#include "ffge_surface.hpp"
#include "ffge_device.hpp"
#include "ffge_physicalDevice.hpp"
#include <stdexcept>
#include <set>

namespace FluffyFox::Graphics
{
	Device::Device(const PhysicalDevice &physicalDevice, const Surface &surface, const std::vector<const char *> &deviceExtensions) : m_physicalDevice(physicalDevice)
	{
		QueueFamilyIndices indices = m_physicalDevice.findQueueFamilies(surface);
		initLogicDevice(indices, deviceExtensions);
		initGraphicsQueue(indices);
		initPresentQueue(indices);
	}

	void Device::initLogicDevice(const QueueFamilyIndices &indices, const std::vector<const char *> &deviceExtensions)
	{
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()}; // ensures there are no duplicates if they are the same queue (extremely likely)

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;

			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (vkCreateDevice(m_physicalDevice.getHandle(), &createInfo, nullptr, &m_device) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create logical device!");
		}
	}

	void Device::initGraphicsQueue(const QueueFamilyIndices &indices)
	{
		vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
	}

	void Device::initPresentQueue(const QueueFamilyIndices &indices)
	{
		vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);
	}

	Device::~Device()
	{
		vkDestroyDevice(m_device, nullptr);
	}
}
