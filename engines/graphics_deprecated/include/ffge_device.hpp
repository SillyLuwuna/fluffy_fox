#pragma once

#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace FluffyFox::Graphics
{
	class PhysicalDevice;
	class Surface;
	struct QueueFamilyIndices;

	class Device
	{
		public:
			Device(const PhysicalDevice &physicalDevice, const Surface &surface, const std::vector<const char *> &deviceExtensions);
			~Device();

			Device(const Device &) = delete;
			Device &operator=(const Device &) = delete;

			inline const VkDevice &getHandle() const
			{
				return m_device;
			}

		private:
			VkDevice m_device;
			const PhysicalDevice &m_physicalDevice;
			VkQueue m_graphicsQueue;
			VkQueue m_presentQueue;

			void initLogicDevice(const QueueFamilyIndices &indices, const std::vector<const char *> &deviceExtensions);
			void initGraphicsQueue(const QueueFamilyIndices &indices);
			void initPresentQueue(const QueueFamilyIndices &indices);
	};
}
