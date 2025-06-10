#pragma once

#include "ffge_debugMessenger.hpp"
#include <memory>
#include <string>
#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace FluffyFox::Graphics
{
	class Instance
	{
		public:
			Instance(const std::string appName, const uint32_t version, const std::vector<const char*> &validationLayers);
			~Instance();

			Instance(const Instance &) = delete;
			Instance &operator=(const Instance &) = delete;

			void setupDebugMessenger();

		private:
			const uint32_t API_VERSION = VK_API_VERSION_1_4; // INFO might want to use version 1_0

			VkInstance m_instance;
			std::string m_appName;
			uint32_t m_version;
			std::unique_ptr<DebugMessenger> m_debugMessenger;

			std::vector<const char *> getRequiredExtensions();
	};
}
