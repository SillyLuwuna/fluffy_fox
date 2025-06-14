#pragma once

#include <optional>
#include <string>
#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace FluffyFox::Graphics
{
	class Surface;

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete()
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	// Must not hold any "real" data; Efficient copy/move
	class PhysicalDevice
	{
		public:
			explicit PhysicalDevice(VkPhysicalDevice pd);
			PhysicalDevice();
			~PhysicalDevice();

			// PhysicalDevice(const PhysicalDevice &) = delete;
			// PhysicalDevice &operator=(const PhysicalDevice &) = delete;

			// Move semantics
			// PhysicalDevice(PhysicalDevice &&) = default;
			// PhysicalDevice &operator=(PhysicalDevice&&) = default;

			inline const VkPhysicalDevice &getHandle() const
			{
				return m_physicalDevice;
			}

			QueueFamilyIndices findQueueFamilies(const Surface &surface) const;

			inline bool isNull() const
			{
				return m_physicalDevice == VK_NULL_HANDLE;
			}

			inline bool isDedicatedGPU() const
			{
				return m_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
			}

			inline bool supportsGeometryShaders() const
			{
				return m_features.geometryShader;
			}

			inline std::string name() const
			{
				return m_properties.deviceName;
			}

			inline uint32_t id() const
			{
				return m_properties.deviceID;
			}


		private:
			VkPhysicalDevice m_physicalDevice; // not a reference so that it can be null. No ownership, just a representation
			VkPhysicalDeviceProperties m_properties;
			VkPhysicalDeviceFeatures m_features;

			std::vector<VkQueueFamilyProperties> getQueueFamilyProperties() const;

			void cacheValues();
			bool hasPresentQueue(int index, const Surface &surface) const;
			bool hasGraphicsQueue(const VkQueueFamilyProperties &queueFamily) const;
	};
}
