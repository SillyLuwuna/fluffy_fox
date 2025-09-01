#include "ffge_engine.hpp"
#include "ffge_window.hpp"
// #include "ffge_surface.hpp"
// #include "ffge_device.hpp"
// #include "ffge_instance.hpp"
// #include "ffge_swapchain.hpp"
// #include "ffge_swapchainSupportDetails.hpp"
// #include "ffge_queueFamilyIndices.hpp"

#include <cstring>
#include <iostream>
#include <set>
#include <vector>
#include <stdexcept>
#include <fstream>

namespace FluffyFox::Graphics
{
	// Engine::Engine() : m_physicalDevice()
	Engine::Engine() :
		m_instance(nullptr),
		m_debugMessenger(nullptr)
	{

	}

	Engine::~Engine()
	{
	// 	m_swapchain.reset();
	// 	m_device.reset();
	// 	m_surface.reset();
	// 	m_instance.reset();
	// 	m_window.reset();
	}

	void Engine::run()
	{
		initWindow();
		initVulkan();
		mainLoop();
	}

	void Engine::initWindow()
	{
		m_window = std::make_unique<Window>(WIDTH, HEIGHT, TITLE);
	}

	void Engine::initVulkan()
	{
		createInstance();
		setupDebugMessenger();
	// 	createSurface();
	// 	pickPhysicalDevice();
	// 	createLogicalDevice();
	// 	createSwapchain();
	}

	void Engine::mainLoop()
	{
		while (!m_window->shouldClose())
		{
			glfwPollEvents();
		}
	}

	void Engine::createInstance()
	{
		vk::ApplicationInfo appInfo {
			.pApplicationName = APP_NAME.c_str(),
			.applicationVersion = APP_VERSION,
			.pEngineName = APP_NAME.c_str(),
			.engineVersion = APP_VERSION,
			.apiVersion = vk::ApiVersion14
		};

		// get required layers
		std::vector<char const*> requiredLayers;
		if (enableValidationLayers)
		{
			requiredLayers.assign(VALIDATION_LAYERS.begin(), VALIDATION_LAYERS.end());
		}

		// check if the required layers are supported by the Vulkan implementation
		std::vector<vk::LayerProperties> layerProperties = m_context.enumerateInstanceLayerProperties();
		if (std::ranges::any_of(
				requiredLayers,
				[&layerProperties](auto const &requiredLayer) {
					return std::ranges::none_of(
						layerProperties,
						[requiredLayer](auto const& layerProperty) {
							return strcmp(layerProperty.layerName, requiredLayer) == 0;
						}
					);
				}
			))
		{
			throw std::runtime_error("One or more required layers are not supported!");
		}

		std::vector<const char*> requiredExtensions = getRequiredExtensions();
		// Check if the required extensions are supported by the Vulkan implementation.
		std::vector<vk::ExtensionProperties> extensionProperties = m_context.enumerateInstanceExtensionProperties();
        for (auto const & requiredExtension : requiredExtensions)
        {
            if (std::ranges::none_of(
				extensionProperties,
				[requiredExtension](auto const& extensionProperty) {
					return strcmp(extensionProperty.extensionName, requiredExtension) == 0;
				}
			))
            {
                throw std::runtime_error("Required extension not supported: " + std::string(requiredExtension));
            }
        }

		vk::InstanceCreateInfo createInfo {
			.pApplicationInfo = &appInfo,
			.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
			.ppEnabledLayerNames = requiredLayers.data(),
			.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
			.ppEnabledExtensionNames = requiredExtensions.data()
		};

		m_instance = vk::raii::Instance(m_context, createInfo);
	}

	VKAPI_ATTR vk::Bool32 VKAPI_CALL Engine::debugCallback(
		vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
		vk::DebugUtilsMessageTypeFlagsEXT type,
		const vk::DebugUtilsMessengerCallbackDataEXT * pCallbackData,
		void*)
	{
		if (severity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
		{
			std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;
		}

		return vk::False;
	}

	std::vector<const char*> Engine::getRequiredExtensions()
	{
		// glfw extensions
		uint32_t glfwExtensionCount = 0;
		const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		// validation extensions
		if (enableValidationLayers)
		{
			extensions.push_back(vk::EXTDebugUtilsExtensionName);
		}

		return extensions;
	}

	void Engine::setupDebugMessenger()
	{
		if (!enableValidationLayers) return;
		vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
				vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
				vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
		vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
				vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
				vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);

		vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT {
			.messageSeverity = severityFlags,
			.messageType = messageTypeFlags,
			.pfnUserCallback = &debugCallback
		};
		m_debugMessenger = m_instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);}

	// bool Engine::checkValidationLayerSupport()
	// {
	// 	uint32_t layerCount;
	// 	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	//
	// 	std::vector<VkLayerProperties> availableLayers(layerCount);
	// 	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
	//
	// 	for (const char *layerName : validationLayers)
	// 	{
	// 		bool layerFound = false;
	// 		for (const auto &layerProperties : availableLayers)
	// 		{
	// 			if (strcmp(layerName, layerProperties.layerName) == 0)
	// 			{
	// 				layerFound = true;
	// 				break;
	// 			}
	// 		}
	//
	// 		if (!layerFound)
	// 		{
	// 			return false;
	// 		}
	// 	}
	//
	// 	return true;
	// }
	//
	// void Engine::mainLoop()
	// {
	// 	while (!m_window->shouldClose())
	// 	{
	// 		glfwPollEvents();
	// 	}
	// }
	//
	// void Engine::setupDebugMessenger()
	// {
	// 	m_instance->setupDebugMessenger();
	// }
	//
	// void Engine::pickPhysicalDevice()
	// {
	// 	std::vector<PhysicalDevice> devices = m_instance->getPhysicalDevices();
	//
	// 	#ifndef NDEBUG
	// 		std::cout << "Physical devices:\n";
	// 		for (const auto &device : devices)
	// 		{
	// 			std::cout << "\t" << device.id() << ":\t" << device.name() << "\n";
	// 		}
	// 	#endif
	//
	// 	for (const PhysicalDevice& device : devices)
	// 	{
	// 		if (isDeviceSuitable(device))
	// 		{
	// 			m_physicalDevice = device;
	// 			break;
	// 		}
	// 	}
	//
	// 	if (m_physicalDevice.isNull())
	// 	{
	// 		throw std::runtime_error("failed to find a suitable GPU!");
	// 	}
	// }
	//
	// bool Engine::isDeviceSuitable(const PhysicalDevice &device)
	// {
	// 	// queue family support
	// 	QueueFamilyIndices indices = device.findQueueFamilies(*m_surface);
	// 	bool hasRequiredQueueFamilies = indices.isComplete();
	//
	// 	// extensions support
	// 	bool extensionsSupported = checkDeviceExtensionSupport(device);
	//
	// 	// swapchain support
	// 	bool swapchainAdequate = false;
	// 	if (extensionsSupported)
	// 	{
	// 		SwapchainSupportDetails swapchainSupport = device.querySwapchainSupport(*m_surface);
	// 		swapchainAdequate = !swapchainSupport.formats.empty() && !swapchainSupport.presentModes.empty();
	// 	}
	//
	// 	return hasRequiredQueueFamilies && extensionsSupported && swapchainAdequate;
	// 	// return device.isDedicatedGPU() && device.supportsGeometryShaders(); // INFO must be changed to accomodate for laptops.
	// }
	//
	// bool Engine::checkDeviceExtensionSupport(const PhysicalDevice &device)
	// {
	// 	std::vector<VkExtensionProperties> availableExtensions = device.getAvailableExtensions();
	// 	#ifndef NDEBUG
	// 		// std::cout << "device available extensions:\n";
	// 		// for (const auto &availableExtension : availableExtensions)
	// 		// {
	// 		// 	std::cout << "\t" << availableExtension.extensionName << "\n";
	// 		// }
	// 	#endif
	//
	// 	std::set<std::string> requiredExtensions(m_deviceExtensions.begin(), m_deviceExtensions.end());
	// 	#ifndef NDEBUG
	// 		// std::cout << "required extensions:\n";
	// 		// for (const auto &requiredExtension : requiredExtensions)
	// 		// {
	// 		// 	std::cout << "\t" << requiredExtension << "\n";
	// 		// }
	// 	#endif
	// 	for (const auto& extension : availableExtensions)
	// 	{
	// 		requiredExtensions.erase(extension.extensionName);
	// 	}
	//
	// 	return requiredExtensions.empty();
	// }
	//
	// void Engine::createLogicalDevice()
	// {
	// 	m_device = std::make_unique<Device>(m_physicalDevice, *m_surface, m_deviceExtensions);
	// }
	//
	// void Engine::createSurface()
	// {
	// 	m_surface = m_window->createSurface(*m_instance);
	// }
	//
	// void Engine::createSwapchain()
	// {
	// 	m_swapchain = std::make_unique<Swapchain>(m_physicalDevice, *m_surface, *m_window, *m_device);
	// }
}
