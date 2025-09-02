#pragma once


// #include "ffge_physicalDevice.hpp"
#include <vector>
#include <memory>
#include <string>
#include "ffge_window.hpp"
#include <algorithm>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#ifdef __INTELLISENSE__
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

namespace FluffyFox::Graphics
{
	class Engine
	{
		public:
			Engine();

			Engine(const Engine &) = delete;
			Engine &operator=(const Engine &) = delete;

			void run();

		private:
			// constants
			const uint32_t WIDTH = 800;
			const uint32_t HEIGHT = 600;
			const std::string TITLE = "Vulkan";
			const std::string APP_NAME = "Fluffy Graphics Engine";
			const uint32_t APP_VERSION = VK_MAKE_VERSION(0, 0, 0);
			const std::vector<char const*> VALIDATION_LAYERS = {
				"VK_LAYER_KHRONOS_validation"
			};
			const std::vector<const char*> DEVICE_EXTENSIONS= {
				vk::KHRSwapchainExtensionName,
				vk::KHRSpirv14ExtensionName,
				vk::KHRSynchronization2ExtensionName,
				vk::KHRCreateRenderpass2ExtensionName
			};
			#ifdef NDEBUG
			static constexpr bool enableValidationLayers = false;
			#else
			static constexpr bool enableValidationLayers = true;
			#endif

			// member variables
			std::unique_ptr<Window> m_window;
			vk::raii::Context m_context;
			vk::raii::Instance m_instance;
			vk::raii::DebugUtilsMessengerEXT m_debugMessenger;
			vk::raii::PhysicalDevice m_physicalDevice;

			// functions
			static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
				vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT * pCallbackData, void*);
			void initWindow();
			void initVulkan();
			void mainLoop();
			void createInstance();
			void setupDebugMessenger();
			void pickPhysicalDevice();
			bool isDeviceSuitable(const vk::raii::PhysicalDevice &device) const;
			uint32_t findQueueFamilies(const VkPhysicalDevice &device) const;
			std::vector<const char*> getRequiredExtensions();
	};
}
