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
#include <fstream>

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
		createGraphicsPipeline();
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

	void Engine::createGraphicsPipeline()
	{
		std::vector<char> vertShaderCode = readFile("shaders/vert.spv");
		std::vector<char> fragShaderCode = readFile("shaders/frag.spv");

		VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";
		vertShaderStageInfo.pSpecializationInfo = nullptr;
		// INFO pSpecializationInfo could be used for shader constants, which could be used to specify behaviour of the shaders
		// INFO this is *way* more efficient than writing the variables in the shaders, the compiler will
		// INFO optimize the if statements of said variables

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";
		fragShaderStageInfo.pSpecializationInfo = nullptr;

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		vkDestroyShaderModule(m_device->getHandle(), fragShaderModule, nullptr);
		vkDestroyShaderModule(m_device->getHandle(), vertShaderModule, nullptr);
	}

	VkShaderModule Engine::createShaderModule(const std::vector<char> &bytecode)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = bytecode.size();
		createInfo.pCode = reinterpret_cast<const uint32_t *>(bytecode.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(m_device->getHandle(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create shader module!");
		}

		return shaderModule;
	}

	std::vector<char> Engine::readFile(const std::string &filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary); // start reading at the end of file & read the file as a binary

		if (!file.is_open())
		{
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t) file.tellg(); // get read position to get length of file
		std::vector<char> buffer(fileSize);

		file.seekg(0); // return read position to start
		file.read(buffer.data(), fileSize);

		file.close();
		return buffer;
	}
}
