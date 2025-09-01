#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace FluffyFox::Graphics
{
	class DebugMessenger
	{
		public:
			DebugMessenger(const VkInstance &instance);
			~DebugMessenger();

			DebugMessenger(const DebugMessenger &) = delete;
			DebugMessenger &operator=(const DebugMessenger &) = delete;

			void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) const;

			inline const VkDebugUtilsMessengerEXT &getHandle() const
			{
				return m_debugMessenger;
			}

		private:
			VkDebugUtilsMessengerEXT m_debugMessenger;
			const VkInstance &m_instance; // since the instance is guaranteed to outlive the debug messenger by the engine

			static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
				VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
				VkDebugUtilsMessageTypeFlagsEXT messageType,
				const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
				void *pUserData);
			VkResult createDebugUtilsMessengerEXT(
				VkInstance instance,
				const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
				const VkAllocationCallbacks *pAllocator,
				VkDebugUtilsMessengerEXT *pDebugMessenger);
			static void destroyDebugUtilsMessengerEXT(
				VkInstance instance,
				VkDebugUtilsMessengerEXT debugMessenger,
				const VkAllocationCallbacks *pAllocator);
	};
}
