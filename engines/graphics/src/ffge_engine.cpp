#include "ffge_engine.hpp"
#include "ffge_config.hpp"

#include <cstring>
#include <iostream>
#include <vector>
#include <stdexcept>

namespace FluffyFox::Graphics
{
	Engine::Engine()
	{

	}

	Engine::~Engine()
	{
		// !!! EXPLICIT DESTROY ORDER !!!
		// m_debugMessenger.reset();
		m_instance.reset();
		m_window.reset();
		// !!! EXPLICIT DESTROY ORDER !!!
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
}
