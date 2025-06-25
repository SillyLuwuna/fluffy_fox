#include "ffge_surface.hpp"
#include "ffge_engine.hpp"
#include "ffge_device.hpp"
#include "ffge_instance.hpp"
#include "ffge_swapchain.hpp"
#include "ffge_swapchainSupportDetails.hpp"
#include "ffge_window.hpp"
#include "ffge_queueFamilyIndices.hpp"

#include <cstring>
#include <iostream>
#include <set>
#include <vector>
#include <stdexcept>

namespace FluffyFox::Graphics
{
	Engine::Engine() : m_physicalDevice()
	{

	}

	Engine::~Engine()
	{
		m_swapchain.reset();
		m_device.reset();
		m_surface.reset();
		m_instance.reset();
		m_window.reset();
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
	}

	void Engine::createInstance()
	{
		#ifndef NDEBUG
		if (!checkValidationLayerSupport())
		{
			throw std::runtime_error("validation layers requested, but not available!");
		}
		#endif
		m_instance = std::make_unique<Instance>(APP_NAME, APP_VERSION, validationLayers);
	}

	bool Engine::checkValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char *layerName : validationLayers)
		{
			bool layerFound = false;
			for (const auto &layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				return false;
			}
		}

		return true;
	}

	void Engine::mainLoop()
	{
		while (!m_window->shouldClose())
		{
			glfwPollEvents();
		}
	}

	void Engine::setupDebugMessenger()
	{
		m_instance->setupDebugMessenger();
	}

	void Engine::pickPhysicalDevice()
	{
		std::vector<PhysicalDevice> devices = m_instance->getPhysicalDevices();

		#ifndef NDEBUG
			std::cout << "Physical devices:\n";
			for (const auto &device : devices)
			{
				std::cout << "\t" << device.id() << ":\t" << device.name() << "\n";
			}
		#endif

		for (const PhysicalDevice& device : devices)
		{
			if (isDeviceSuitable(device))
			{
				m_physicalDevice = device;
				break;
			}
		}

		if (m_physicalDevice.isNull())
		{
			throw std::runtime_error("failed to find a suitable GPU!");
		}
	}

	bool Engine::isDeviceSuitable(const PhysicalDevice &device)
	{
		// queue family support
		QueueFamilyIndices indices = device.findQueueFamilies(*m_surface);
		bool hasRequiredQueueFamilies = indices.isComplete();

		// extensions support
		bool extensionsSupported = checkDeviceExtensionSupport(device);

		// swapchain support
		bool swapchainAdequate = false;
		if (extensionsSupported)
		{
			SwapchainSupportDetails swapchainSupport = device.querySwapchainSupport(*m_surface);
			swapchainAdequate = !swapchainSupport.formats.empty() && !swapchainSupport.presentModes.empty();
		}

		return hasRequiredQueueFamilies && extensionsSupported && swapchainAdequate;
		// return device.isDedicatedGPU() && device.supportsGeometryShaders(); // INFO must be changed to accomodate for laptops.
	}

	bool Engine::checkDeviceExtensionSupport(const PhysicalDevice &device)
	{
		std::vector<VkExtensionProperties> availableExtensions = device.getAvailableExtensions();
		#ifndef NDEBUG
			// std::cout << "device available extensions:\n";
			// for (const auto &availableExtension : availableExtensions)
			// {
			// 	std::cout << "\t" << availableExtension.extensionName << "\n";
			// }
		#endif

		std::set<std::string> requiredExtensions(m_deviceExtensions.begin(), m_deviceExtensions.end());
		#ifndef NDEBUG
			// std::cout << "required extensions:\n";
			// for (const auto &requiredExtension : requiredExtensions)
			// {
			// 	std::cout << "\t" << requiredExtension << "\n";
			// }
		#endif
		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	void Engine::createLogicalDevice()
	{
		m_device = std::make_unique<Device>(m_physicalDevice, *m_surface, m_deviceExtensions);
	}

	void Engine::createSurface()
	{
		m_surface = m_window->createSurface(*m_instance);
	}

	void Engine::createSwapchain()
	{
		m_swapchain = std::make_unique<Swapchain>(m_physicalDevice, *m_surface, *m_window, *m_device);
	}

	void Engine::createImageViews()
	{
		m_swapchain->createImageViews();
	}
}
