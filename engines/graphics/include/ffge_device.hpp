#pragma once

#include "ffge_physicalDevice.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace FluffyFox::Graphics
{
	class Device
	{
		public:
			Device(PhysicalDevice &pd);
			~Device();

			Device(const Device &) = delete;
			Device &operator=(const Device &) = delete;

		private:
			VkDevice m_device;
			PhysicalDevice &m_physicalDevice;
			VkQueue m_graphicsQueue;

			void initLogicDevice(QueueFamilyIndices &indices);
			void initGraphicsQueue(QueueFamilyIndices &indices);
	};
}
