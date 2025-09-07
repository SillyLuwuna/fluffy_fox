#include "ffge_engine.hpp"

#include <cstring>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <stdexcept>
#include <memory>

namespace FluffyFox::Graphics
{
	Engine::Engine() :
		m_window(nullptr),
		m_instance(nullptr),
		m_debugMessenger(nullptr),
		m_physicalDevice(nullptr),
		m_device(nullptr),
		m_graphicsQueue(nullptr),
		m_presentQueue(nullptr),
		m_surface(nullptr)
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
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
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
		m_debugMessenger = m_instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
	}

	void Engine::createSurface()
	{
		VkSurfaceKHR surface;
		if (glfwCreateWindowSurface(*m_instance, m_window->getHandle(), nullptr, &surface) != 0)
		{
			throw std::runtime_error("failed to create window surface!");
		}
		m_surface = vk::raii::SurfaceKHR(m_instance, surface);
	}

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
				for (auto const &extension : REQUIRED_DEVICE_EXTENSIONS)
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

	void Engine::createLogicalDevice()
	{
		// find the index of the first queue family that supports graphics and presenting
		std::vector<vk::QueueFamilyProperties> queueFamilyProperties = m_physicalDevice.getQueueFamilyProperties();
		uint32_t graphicsIndex = ~0;
		uint32_t presentIndex = ~0;
		for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
		{
			bool supportsGraphics = static_cast<bool>(queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics);
			bool supportsPresent = m_physicalDevice.getSurfaceSupportKHR(i, *m_surface);

			if (supportsGraphics && supportsPresent)
			{
				// most efficient
				graphicsIndex = i;
				presentIndex = i;
				break;
			}
			else if (supportsGraphics)
			{
				graphicsIndex = i;
			}
			else if (supportsPresent)
			{
				presentIndex = i;
			}
		}

		if (graphicsIndex == ~0u)
		{
			throw std::runtime_error("Could not find a queue for graphics!");
		}
		else if (presentIndex == ~0u)
		{
			throw std::runtime_error("Could not find a queue for presenting!");
		}
		else if (graphicsIndex == presentIndex)
		{
			std::cout << "Found a queue family for simultaneous graphics and presenting!\n";
		}
		else
		{
			std::cout << "Using different queue families for graphics and presenting...\n";
		}

		// query for Vulkan 1.3 features
		vk::StructureChain<vk::PhysicalDeviceFeatures2,
			vk::PhysicalDeviceVulkan13Features,
			vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> featureChain = {
				{},
				{ .dynamicRendering = true },
				{ .extendedDynamicState = true }
		};

		// create a Device
		std::set<uint32_t> uniqueQueueFamilies = { graphicsIndex, presentIndex };
		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
		float queuePriority = 0.0f;

		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			queueCreateInfos.push_back(vk::DeviceQueueCreateInfo {
				.queueFamilyIndex = queueFamily,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority
			});
		}

		vk::DeviceCreateInfo deviceCreateInfo {
			.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
			.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
			.pQueueCreateInfos = queueCreateInfos.data(),
			.enabledExtensionCount = static_cast<uint32_t>(REQUIRED_DEVICE_EXTENSIONS.size()),
			.ppEnabledExtensionNames = REQUIRED_DEVICE_EXTENSIONS.data()
		};

		m_device = vk::raii::Device(m_physicalDevice, deviceCreateInfo);
		m_graphicsQueue = vk::raii::Queue(m_device, graphicsIndex, 0);
		m_presentQueue = vk::raii::Queue(m_device, presentIndex, 0);
	}
}
