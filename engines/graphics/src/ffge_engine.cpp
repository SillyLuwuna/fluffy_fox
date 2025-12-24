#include "ffge_engine.hpp"

#include <cstring>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <stdexcept>
#include <memory>
#include <cstdint>
#include <limits>
#include <algorithm>


namespace FluffyFox::Graphics
{
	Engine::Engine() :
		m_window(nullptr),
		m_instance(nullptr),
		m_debugMessenger(nullptr),
		m_physicalDevice(nullptr),
		m_device(nullptr),
		m_graphicsQueue(nullptr),
		m_graphicsFamily(~0u),
		m_presentQueue(nullptr),
		m_presentFamily(~0u),
		m_surface(nullptr),
		m_swapchainSurfaceFormat(vk::Format::eUndefined),
		m_swapchainExtent(),
		m_swapchain(nullptr),
		m_swapchainImages()
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
		createSwapchain();
		createImageViews();
		createGraphicsPipeline();
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
		m_graphicsFamily = graphicsIndex;
		m_presentQueue = vk::raii::Queue(m_device, presentIndex, 0);
		m_presentFamily = presentIndex;
	}

	void Engine::createSwapchain()
	{
		vk::SurfaceCapabilitiesKHR surfaceCapabilities = m_physicalDevice.getSurfaceCapabilitiesKHR(m_surface);
		m_swapchainSurfaceFormat = chooseSwapchainSurfaceFormat(m_physicalDevice.getSurfaceFormatsKHR(m_surface));
		m_swapchainExtent = chooseSwapchainSwapExtent(surfaceCapabilities);
		vk::PresentModeKHR presentMode = chooseSwapchainPresentMode(m_physicalDevice.getSurfacePresentModesKHR(m_surface));
		uint32_t minImageCount = std::max(3u, surfaceCapabilities.minImageCount); // getting a good amount of image counts
		if (surfaceCapabilities.maxImageCount > 0 && minImageCount > surfaceCapabilities.maxImageCount) // fallback if image count was too high
		{
			minImageCount = surfaceCapabilities.maxImageCount;
		}

		vk::SwapchainCreateInfoKHR swapchainCreateInfo {
			.flags = vk::SwapchainCreateFlagsKHR(),
			.surface = m_surface,
			.minImageCount = minImageCount,
			.imageFormat = m_swapchainSurfaceFormat.format,
			.imageColorSpace = m_swapchainSurfaceFormat.colorSpace,
			.imageExtent = m_swapchainExtent,
			.imageArrayLayers = 1,
			.imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
			.imageSharingMode = vk::SharingMode::eExclusive,
			.preTransform = surfaceCapabilities.currentTransform,
			.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
			.presentMode = presentMode,
			.clipped = vk::True,
			.oldSwapchain = nullptr
		};

		if (m_graphicsFamily == ~0u || m_presentFamily == ~0u)
		{
			throw std::runtime_error("graphicsFamily/presentFamily non initialized");
		}
		uint32_t queueFamilyIndices[] = {m_graphicsFamily, m_presentFamily};

		if (m_graphicsFamily != m_presentFamily) // uses concurrent sharing when the queue families are different (needed)
												 // however this requires special handling
		{
			swapchainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			swapchainCreateInfo.queueFamilyIndexCount = 2;
			swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			swapchainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive; // most performant
			swapchainCreateInfo.queueFamilyIndexCount = 0; // Optional
			swapchainCreateInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		m_swapchain = vk::raii::SwapchainKHR(m_device, swapchainCreateInfo);
		m_swapchainImages = m_swapchain.getImages();
	}

	// surface format (color depth)
	vk::SurfaceFormatKHR Engine::chooseSwapchainSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats)
	{
		for (const vk::SurfaceFormatKHR &availableFormat : availableFormats)
		{
			if (availableFormat.format == vk::Format::eB8G8R8A8Srgb
				&& availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			{
				std::cout << "Using srgb.\n";
				return availableFormat;
			}
		}
		return availableFormats[0];
	}

	// present mode (conditions for "swapping" images to the screen)
	vk::PresentModeKHR Engine::chooseSwapchainPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes)
	{
		for (const vk::PresentModeKHR &availablePresentMode : availablePresentModes)
		{
			// if (availablePresentMode == vk::PresentModeKHR::eMailbox)
			if (availablePresentMode == vk::PresentModeKHR::eImmediate) // tearing
			{
				return availablePresentMode;
			}
		}
		return vk::PresentModeKHR::eFifo; // always available by default
	}

	// swap extent (resolution of images)
	vk::Extent2D Engine::chooseSwapchainSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities)
	{
		// if window manager allows us to set whatever we want
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}
		int width, height;
		glfwGetFramebufferSize(m_window->getHandle(), &width, &height);

		return {
			std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
			std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.minImageExtent.height)
		};
	}

	void Engine::createImageViews()
	{
		m_swapchainImageViews.clear();

		vk::ImageViewCreateInfo imageViewCreateInfo {
			.viewType = vk::ImageViewType::e2D,
			.format = m_swapchainSurfaceFormat.format,
			.subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
		};

		// WARNING might be wrong? the tutorial used wrong constants
		imageViewCreateInfo.components.r = vk::ComponentSwizzle::eIdentity;
		imageViewCreateInfo.components.g = vk::ComponentSwizzle::eIdentity;
		imageViewCreateInfo.components.b = vk::ComponentSwizzle::eIdentity;
		imageViewCreateInfo.components.a = vk::ComponentSwizzle::eIdentity;

		// WARNING might also be wrong?
		imageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;

		for (vk::Image& image : m_swapchainImages)
		{
			imageViewCreateInfo.image = image;
			m_swapchainImageViews.emplace_back(m_device, imageViewCreateInfo);
		}
	}

	void Engine::createGraphicsPipeline()
	{

	}
}
