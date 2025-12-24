#pragma once


// #include "ffge_physicalDevice.hpp"
#include <vector>
#include <memory>
#include <string>
#include "ffge_window.hpp"
#include <algorithm>
#include <cstdint>
#include <limits>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// #ifdef __INTELLISENSE__
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
// #else
// import vulkan_hpp;
// #endif

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
			const std::vector<const char*> REQUIRED_DEVICE_EXTENSIONS = {
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
			vk::raii::Device m_device;
			vk::raii::Queue m_graphicsQueue;
			uint32_t m_graphicsFamily;
			vk::raii::Queue m_presentQueue;
			uint32_t m_presentFamily;
			vk::raii::SurfaceKHR m_surface;
			vk::SurfaceFormatKHR m_swapchainSurfaceFormat;
			vk::Extent2D m_swapchainExtent;
			vk::raii::SwapchainKHR m_swapchain;
			std::vector<vk::Image> m_swapchainImages;
			std::vector<vk::raii::ImageView> m_swapchainImageViews;

			// functions
			static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
				vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT * pCallbackData, void*);
			void initWindow();
			void initVulkan();
			void mainLoop();
			void createInstance();
			void setupDebugMessenger();
			void createSurface();
			void pickPhysicalDevice();
			void createLogicalDevice();
			bool isDeviceSuitable(const vk::raii::PhysicalDevice &device) const;
			uint32_t findQueueFamilies(const VkPhysicalDevice &device) const;
			std::vector<const char*> getRequiredExtensions();
			void createSwapchain();
			vk::SurfaceFormatKHR chooseSwapchainSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats);
			vk::PresentModeKHR chooseSwapchainPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes);
			vk::Extent2D chooseSwapchainSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities);
			void createImageViews();
			void createGraphicsPipeline();
	};
}
