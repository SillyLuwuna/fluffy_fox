#pragma once

#include "ffge_physicalDevice.hpp"
#include <memory>
#include <vector>
#include <string>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace FluffyFox::Graphics
{
	class Device;
	class Surface;
	class Window;
	class Instance;
	class Swapchain;

	class Engine
	{
		public:
			Engine();
			~Engine();

			Engine(const Engine &) = delete;
			Engine &operator=(const Engine &) = delete;

			void run();

		private:
			const uint32_t WIDTH = 800;
			const uint32_t HEIGHT = 600;
			const std::string TITLE = "Vulkan";
			const std::string APP_NAME = "Fluffy Graphics Engine";
			const uint32_t APP_VERSION = VK_MAKE_VERSION(0, 0, 0);
			const std::vector<const char*> validationLayers = {
				"VK_LAYER_KHRONOS_validation"
			};
			const std::vector<const char*> m_deviceExtensions = {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
			}; // required extensions for the host gpu

			std::unique_ptr<Window> m_window;
			std::unique_ptr<Instance> m_instance;
			PhysicalDevice m_physicalDevice; // only represents the hardware
			std::unique_ptr<Device> m_device;
			std::unique_ptr<Surface> m_surface;
			std::unique_ptr<Swapchain> m_swapchain;

			void initWindow();
			void initVulkan();
			void createInstance();
			void mainLoop();
			bool checkValidationLayerSupport();
			void setupDebugMessenger();
			void pickPhysicalDevice();
			bool isDeviceSuitable(const PhysicalDevice &device);
			bool checkDeviceExtensionSupport(const PhysicalDevice &device);
			void createLogicalDevice();
			void createSurface();
			void createSwapchain();
	};
}
