#pragma once

#include <string>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace FluffyFox::Graphics
{
	class Instance
	{
		public:
			Instance(std::string appName, uint32_t version);
			~Instance();

			Instance(const Instance &) = delete;
			Instance &operator=(const Instance &) = delete;

		private:
			const uint32_t API_VERSION = VK_API_VERSION_1_4; // INFO might want to use version 1_0
			const bool DEBUG = true;

			VkInstance m_instance;
			std::string m_appName;
			uint32_t m_version;
	};
}
