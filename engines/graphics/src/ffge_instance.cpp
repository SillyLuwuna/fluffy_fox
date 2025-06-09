#include "ffge_instance.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>

namespace FluffyFox::Graphics
{
	Instance::Instance(std::string appName, uint32_t version) :
		m_appName{appName},
		m_version{version}
	{
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = m_appName.c_str();
		appInfo.applicationVersion = m_version;
		appInfo.pEngineName = m_appName.c_str();
		appInfo.engineVersion = m_version;
		appInfo.apiVersion = API_VERSION;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;

		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;
		createInfo.enabledLayerCount = 0;

		if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create instance!");
		}

		if (DEBUG)
		{
			uint32_t extensionCount = 0;
			vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

			std::vector<VkExtensionProperties> extensions(extensionCount);
			vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

			std::cout << "avaliable extensions:\n";
			for (const auto& extension : extensions)
			{
				std::cout << '\t' << extension.extensionName << '\n';
			}
		}
	}

	Instance::~Instance()
	{
		vkDestroyInstance(m_instance, nullptr);
	}
}
