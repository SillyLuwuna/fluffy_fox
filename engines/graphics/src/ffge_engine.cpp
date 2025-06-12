#include "ffge_engine.hpp"
#include "ffge_config.hpp"
#include "ffge_device.hpp"

#include <cstring>
#include <iostream>
#include <vector>
#include <stdexcept>

namespace FluffyFox::Graphics
{
	Engine::Engine() : m_physicalDevice()
	{

	}

	Engine::~Engine()
	{
		m_device.reset();
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
		pickPhysicalDevice();
		createLogicalDevice();
	}

	void Engine::createInstance()
	{
		if (FFGE_DEBUG && !checkValidationLayerSupport())
		{
			throw std::runtime_error("validation layers requested, but not available!");
		}
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
				std::cout << device.id() << ":\t" << device.name() << "\n";
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
		QueueFamilyIndices indices = device.findQueueFamilies();
		return indices.isComplete();
		// return true;
		// return device.isDedicatedGPU() && device.supportsGeometryShaders(); // INFO must be changed to accomodate for laptops.
	}

	void Engine::createLogicalDevice()
	{
		m_device = std::make_unique<Device>(m_physicalDevice);
	}
}
