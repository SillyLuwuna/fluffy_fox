#pragma once

#include "ffge_debugMessenger.hpp"
#include "ffge_window.hpp"
#include "ffge_instance.hpp"
#include <memory>
#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace FluffyFox::Graphics
{
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

			std::unique_ptr<Window> m_window;
			std::unique_ptr<Instance> m_instance;
			// std::unique_ptr<DebugMessenger> m_debugMessenger;

			void initWindow();
			void initVulkan();
			void createInstance();
			void mainLoop();
			bool checkValidationLayerSupport();
			void setupDebugMessenger();
	};
}
