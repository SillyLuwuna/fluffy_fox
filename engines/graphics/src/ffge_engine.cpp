#include "ffge_engine.hpp"

#include <cstring>
#include <iostream>
#include <map>
#include <vector>
#include <stdexcept>
#include <memory>

namespace FluffyFox::Graphics
{
	Engine::Engine() :
		m_window(nullptr),
		m_instance(nullptr),
		m_debugMessenger(nullptr),
		m_physicalDevice(nullptr)
	{

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
		pickPhysicalDevice();
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

	void Engine::pickPhysicalDevice()
	{
		std::vector<vk::raii::PhysicalDevice> devices = m_instance.enumeratePhysicalDevices();
		// if (devices.empty())
		// {
		// 	throw std::runtime_error("failed to find GPUs with Vulkan support!");
		// }

		// for (const auto& device : devices)
		// {
		// vk::PhysicalDeviceProperties deviceProperties = device.getProperties();
		// vk::PhysicalDeviceFeatures deviceFeatures = device.getFeatures();
		// FIXME refactor, complete code gore
		const auto devIter = std::ranges::find_if(devices,
			[&](auto const &device) {
				// checks for api version
				bool isSuitable = device.getProperties().apiVersion >= VK_API_VERSION_1_3;
				auto queueFamilies = device.getQueueFamilyProperties();
				const auto qfpIter = std::ranges::find_if(queueFamilies,
					[](vk::QueueFamilyProperties const &qfp) {
						// checks if GPU supports graphic operations
						return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) != static_cast<vk::QueueFlags>(0);
					}
				);
				isSuitable = isSuitable && (qfpIter != queueFamilies.end());
				auto extensions = device.enumerateDeviceExtensionProperties();
				bool found = true;
				for (auto const &extension : DEVICE_EXTENSIONS)
				{
					auto extensionIter = std::ranges::find_if(extensions,
						[extension](auto const &ext) {
							// must have extensions in DEVICE_EXTENSIONS
							return strcmp(ext.extensionName, extension) == 0;
						}
					);
					found = found && extensionIter != extensions.end();
				}
				isSuitable = isSuitable && found;
				if (isSuitable)
				{
					m_physicalDevice = device;
				}
				return isSuitable;
			}
		);
		if (devIter == devices.end())
		{
			throw std::runtime_error("failed to find a suitable GPU!");
		}
		std::cout << "GPU: " << m_physicalDevice.getProperties().deviceName << '\n';
		// uint32_t score = 0;
		//
		// // Discrete GPUs are better than integrated ones.
		// if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
		// {
		// 	score += 1000;
		// }
		//
		// // Maximum possible size of textures affects grpahics quality
		// score += deviceProperties.limits.maxImageDimension2D;
		//
		// // Application can't function without geometry shaders
		// if (!deviceFeatures.geometryShader)
		// {
		// 	continue;
		// }
		// candidates.insert(std::make_pair(score, device));
		//
		// if (!candidates.empty() && candidates.rbegin()->first > 0)
		// {
		// 	m_physicalDevice = std::move(candidates.rbegin()->second);
		// }
		// else
		// {
		// 	throw std::runtime_error("failed to find a suitable GPU!");
		// }
	}

	// FIXME not used at all?
	uint32_t Engine::findQueueFamilies(const VkPhysicalDevice &device) const
	{
		// find the index of the first queue family that supports graphics
		std::vector<vk::QueueFamilyProperties> queueFamilyProperties = m_physicalDevice.getQueueFamilyProperties();

		// get the first index into queueFamilyProperties which supports graphics
		auto graphicsQueueFamilyProperty = std::find_if(queueFamilyProperties.begin(), queueFamilyProperties.end(),
			[](vk::QueueFamilyProperties const &qfp) {
				return qfp.queueFlags & vk::QueueFlagBits::eGraphics;
			}
		);

		return static_cast<uint32_t>(std::distance(queueFamilyProperties.begin(), graphicsQueueFamilyProperty));
	}

	// FIXME not used at all?
	bool Engine::isDeviceSuitable(const vk::raii::PhysicalDevice &device) const
	{
		vk::PhysicalDeviceProperties deviceProperties = device.getProperties();
		vk::PhysicalDeviceFeatures deviceFeatures = device.getFeatures();

		if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu && deviceFeatures.geometryShader)
		{
			// m_physicalDevice = std::move(device);
			// break;
		}
		return true;
	}
}
