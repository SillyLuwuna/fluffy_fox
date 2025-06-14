#include "ffge_config.hpp"
#include "ffge_debugMessenger.hpp"
#include "ffge_physicalDevice.hpp"
#include "ffge_instance.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>

namespace FluffyFox::Graphics
{
	Instance::Instance(
			const std::string appName,
			const uint32_t version,
			const std::vector<const char *> &validationLayers) :
		m_appName{appName},
		m_version{version}
	{
		// app info
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = m_appName.c_str();
		appInfo.applicationVersion = m_version;
		appInfo.pEngineName = m_appName.c_str();
		appInfo.engineVersion = m_version;
		appInfo.apiVersion = API_VERSION;

		// instance create info
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		std::vector<const char *> extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if constexpr (FFGE_DEBUG)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			m_debugMessenger->populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *) &debugCreateInfo;
		}
		else
		{
			createInfo.enabledLayerCount = 0;

			createInfo.pNext = nullptr;
		}

		// instance creation
		if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create instance!");
		}

		// debug
		if constexpr (FFGE_DEBUG)
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
		m_debugMessenger.reset(); // MUST BE DESTROYED FIRST!
		vkDestroyInstance(m_instance, nullptr);
	}

	std::vector<const char *> Instance::getRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount); // includes VK_KHR_win32_surface

		std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (FFGE_DEBUG)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	void Instance::setupDebugMessenger()
	{
		if (!FFGE_DEBUG) return;

		m_debugMessenger = std::make_unique<DebugMessenger>(m_instance);
	}

	std::vector<PhysicalDevice> Instance::getPhysicalDevices()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

		if (deviceCount == 0)
		{
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		std::cout << deviceCount << '\n';
		std::vector<VkPhysicalDevice> vkDevices(deviceCount);
		std::vector<PhysicalDevice> devices;
		devices.reserve(deviceCount);
		vkEnumeratePhysicalDevices(m_instance, &deviceCount, vkDevices.data());


		for (const VkPhysicalDevice &vkDevice : vkDevices)
		{
			devices.emplace_back(PhysicalDevice(vkDevice));
		}
		return devices;
	}
}
