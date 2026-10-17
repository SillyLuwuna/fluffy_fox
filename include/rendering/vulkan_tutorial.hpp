#pragma once

#include <iostream>
#include <vulkan/vulkan_raii.hpp>
#include <fstream>
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include "rendering/window.hpp"

namespace fluffy_fox::rendering
{
	constexpr uint32_t WIDTH = 800;
	constexpr uint32_t HEIGHT = 600;

	constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

	const std::vector<char const*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

#ifdef DEBUG
		constexpr bool enableValidationLayers = true;
#else
		constexpr bool enableValidationLayers = false;
#endif

	struct Vertex
	{
		glm::vec2 pos;
		glm::vec3 color;

		static vk::VertexInputBindingDescription getBindingDescription()
		{
			return {
				.binding = 0,
				.stride = sizeof(Vertex),
				.inputRate = vk::VertexInputRate::eVertex
			};
		}

		static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions()
		{
			return {{
				{
					.location = 0,
					.binding = 0,
					.format = vk::Format::eR32G32Sfloat,
					.offset = offsetof(Vertex, pos)
				},
				{
					.location = 1,
					.binding = 0,
					.format = vk::Format::eR32G32B32Sfloat,
					.offset = offsetof(Vertex, color)
				}
			}};
		}
	};

	const std::vector<Vertex> vertices = {
		{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
		{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
		{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
		{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
	};

	const std::vector<uint16_t> indices = {
		0, 1, 2, 2, 3, 0
	};

	struct UniformBufferObject
	{
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};

	class VulkanTutorial
	{
	private:
		Window& window;
		vk::raii::Context context;
		vk::raii::Instance instance = nullptr;
		vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
		vk::raii::SurfaceKHR surface = nullptr;
		vk::raii::PhysicalDevice physicalDevice = nullptr;
		vk::raii::Device device = nullptr;
		uint32_t queueIndex;
		vk::raii::Queue graphicsQueue = nullptr;
		vk::raii::SwapchainKHR swapChain = nullptr;
		std::vector<vk::Image> swapChainImages;
		vk::SurfaceFormatKHR swapChainSurfaceFormat;
		vk::Extent2D swapChainExtent;
		std::vector<vk::raii::ImageView> swapChainImageViews;
		vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
		vk::raii::PipelineLayout pipelineLayout = nullptr;
		vk::raii::Pipeline graphicsPipeline = nullptr;
		vk::raii::CommandPool commandPool = nullptr;
		// PERF vertexBuffer and indexBuffer should be the same buffer for performance
		vk::raii::Buffer vertexBuffer = nullptr;
		vk::raii::DeviceMemory vertexBufferMemory = nullptr;
		vk::raii::Buffer indexBuffer = nullptr;
		vk::raii::DeviceMemory indexBufferMemory = nullptr;

		vk::raii::DescriptorPool descriptorPool = nullptr;
		std::vector<vk::raii::DescriptorSet> descriptorSets;

		std::vector<vk::raii::Buffer> uniformBuffers;
		std::vector<vk::raii::DeviceMemory> uniformBuffersMemory;
		std::vector<void*> uniformBuffersMapped;

		std::vector<vk::raii::CommandBuffer> commandBuffers;
		std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
		std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
		std::vector<vk::raii::Fence> inFlightFences;
		uint32_t frameIndex = 0;

		bool framebufferResized = false;

		static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
			vk::DebugUtilsMessageSeverityFlagBitsEXT, // severity
			vk::DebugUtilsMessageTypeFlagsEXT type,
			const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* // pUserData
		)
		{
			std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;
			return vk::False;
		}

		void setWindowSettings()
		{
			window.resize_event().subscribe(&VulkanTutorial::framebufferResizeCallback, this);
			window.focus_event().subscribe(&VulkanTutorial::onFocusEvent, this);
		}

		void setupDebugMessenger()
		{
			if constexpr (!enableValidationLayers) return;

			vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(
				vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
				vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
			);

			vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
				vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
				vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
				vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
			);

			vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT {
				.messageSeverity = severityFlags,
				.messageType = messageTypeFlags,
				.pfnUserCallback = &debugCallback
			};

			debugMessenger = instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
		}

		void createSurface()
		{
			VkSurfaceKHR _surface;
			if (glfwCreateWindowSurface(*instance, window.get_handle(), nullptr, &_surface) != 0)
			{
				throw std::runtime_error("failed to create window surface!");
			}

			surface = vk::raii::SurfaceKHR(instance, _surface);
		}

		std::vector<const char *> getRequiredInstanceExtensions()
		{
			uint32_t glfwExtensionCount = 0;
			const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

			std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

			if constexpr (enableValidationLayers)
			{
				extensions.push_back(vk::EXTDebugUtilsExtensionName);
			}

			return extensions;
		}

		void createInstance()
		{
			constexpr vk::ApplicationInfo appInfo
			{
				.pApplicationName = "Vulkan Tutorial",
				.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
				.pEngineName = "No Engine",
				.engineVersion = VK_MAKE_VERSION(1, 0, 0),
				.apiVersion = vk::ApiVersion14
			};

			std::vector<char const *> requiredLayers;
			if constexpr (enableValidationLayers)
			{
				requiredLayers.assign(validationLayers.begin(), validationLayers.end());
			}

			auto layerProperties = context.enumerateInstanceLayerProperties();
			auto unsupportedLayerIt = std::ranges::find_if(requiredLayers,
				[&layerProperties](auto const &requiredLayer) {
					return std::ranges::none_of(layerProperties,
						[requiredLayer](auto const &layerProperty) { return strcmp(layerProperty.layerName, requiredLayer) == 0; });
			});

			if (unsupportedLayerIt != requiredLayers.end())
			{
				throw std::runtime_error("Required layer not supported: " + std::string(*unsupportedLayerIt));
			}

			std::vector<const char *> requiredExtensions = getRequiredInstanceExtensions();
			auto extensionProperties = context.enumerateInstanceExtensionProperties(); // available extensions

			for (uint32_t i = 0; i < requiredExtensions.size(); i++)
			{
				if (std::ranges::none_of(extensionProperties,
					[glfwExtension = requiredExtensions[i]](auto const& extensionProperty)
					{
						return strcmp(extensionProperty.extensionName, glfwExtension) == 0;
					}
				))
				{
					throw std::runtime_error("Required extension not supported: " + std::string(requiredExtensions[i]));
				}
			}

			vk::InstanceCreateInfo createInfo
			{
				.pApplicationInfo = &appInfo,
				.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
				.ppEnabledLayerNames = requiredLayers.data(),
				.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
				.ppEnabledExtensionNames = requiredExtensions.data()
			};

			instance = vk::raii::Instance(context, createInfo); // TODO check for errors
		}

		void pickPhysicalDevice()
		{
			auto physicalDevices = instance.enumeratePhysicalDevices();

			if (physicalDevices.empty())
			{
				throw std::runtime_error("failed to find GPUs with Vulkan support!");
			}

			bool found = false;
			vk::raii::PhysicalDevice chosen = nullptr;

			std::cout << "available GPUs:\n";
			for (auto physicalDevice : physicalDevices)
			{
				// auto deviceFeatures = physicalDevice.getFeatures();

				auto deviceProperties = physicalDevice.getProperties();
				bool supportsVulkan1_3 = deviceProperties.apiVersion >= vk::ApiVersion13;
				std::cout << "\t" << deviceProperties.deviceName.data() << "\n";

				auto queueFamilies = physicalDevice.getQueueFamilyProperties();
				bool supportsGraphics = std::ranges::any_of(queueFamilies, [](auto const &qfp) { return (bool)(qfp.queueFlags & vk::QueueFlagBits::eGraphics); });

				auto availableDeviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();
				std::vector<const char*> requiredDeviceExtension = { vk::KHRSwapchainExtensionName };
				bool supportsAllRequiredExtensions =
					std::ranges::all_of(requiredDeviceExtension,
						[&availableDeviceExtensions](auto const& requiredDeviceExtension) {
							return std::ranges::any_of(availableDeviceExtensions,
								[requiredDeviceExtension](auto const& availableDeviceExtension) {
									return strcmp(availableDeviceExtension.extensionName, requiredDeviceExtension) == 0;
								}
							);
						}
					);

				auto features = physicalDevice.template getFeatures2<
					vk::PhysicalDeviceFeatures2,
					vk::PhysicalDeviceVulkan11Features,
					vk::PhysicalDeviceVulkan13Features,
					vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
				bool supportsRequiredFeatures =
					features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
					features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
					features.template get<vk::PhysicalDeviceVulkan13Features>().synchronization2 &&
					features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

				if (
					supportsVulkan1_3 &&
					supportsGraphics &&
					supportsAllRequiredExtensions &&
					supportsRequiredFeatures &&
					deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu
				)
				{
					if (found) continue;
					found = true;
					chosen = physicalDevice;
				}
			}

			physicalDevice = chosen;
		}

		void createLogicalDevice()
		{
			std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

			queueIndex = ~0;
			for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++)
			{
				if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) &&
					physicalDevice.getSurfaceSupportKHR(qfpIndex, *surface))
				{
					queueIndex = qfpIndex;
					break;
				}
			}

			if (queueIndex == ~0u)
			{
				throw std::runtime_error("Could not find a queue for graphics and present");
			}

			float queuePriority = 0.5f;

			vk::StructureChain<
				vk::PhysicalDeviceFeatures2,
				vk::PhysicalDeviceVulkan11Features,
				vk::PhysicalDeviceVulkan13Features,
				vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
			> featureChain = {
				{},
				{.shaderDrawParameters = true},
				{.synchronization2 = true, .dynamicRendering = true },
				{.extendedDynamicState = true}
			};

			std::vector<const char*> requiredDeviceExtension = {
				vk::KHRSwapchainExtensionName
			};

			vk::DeviceQueueCreateInfo deviceQueueCreateInfo {
				.queueFamilyIndex = queueIndex,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority
			};

			vk::DeviceCreateInfo deviceCreateInfo {
				.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
				.queueCreateInfoCount = 1,
				.pQueueCreateInfos = &deviceQueueCreateInfo,
				.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtension.size()),
				.ppEnabledExtensionNames = requiredDeviceExtension.data()
			};

			device = vk::raii::Device(physicalDevice, deviceCreateInfo); // TODO error checking
			graphicsQueue = vk::raii::Queue(device, queueIndex, 0);
		}

		void createSwapChain()
		{
			vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
			swapChainExtent = chooseSwapExtent(surfaceCapabilities);
			uint32_t minImageCount = chooseSwapMinImageCount(surfaceCapabilities);

			std::vector<vk::SurfaceFormatKHR> availableFormats = physicalDevice.getSurfaceFormatsKHR(*surface);
			swapChainSurfaceFormat = chooseSwapSurfaceFormat(availableFormats);

			std::vector<vk::PresentModeKHR> availablePresentModes = physicalDevice.getSurfacePresentModesKHR(*surface);
			vk::PresentModeKHR presentMode = chooseSwapPresentMode(availablePresentModes);

			vk::SwapchainCreateInfoKHR swapChainCreateInfo {
				.surface = *surface,
				.minImageCount = minImageCount,
				.imageFormat = swapChainSurfaceFormat.format,
				.imageColorSpace = swapChainSurfaceFormat.colorSpace,
				.imageExtent = swapChainExtent,
				.imageArrayLayers = 1,
				.imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
				.imageSharingMode = vk::SharingMode::eExclusive,
				.preTransform = surfaceCapabilities.currentTransform,
				.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
				.presentMode = presentMode,
				.clipped = true,
			};

			swapChain = vk::raii::SwapchainKHR(device, swapChainCreateInfo);
			swapChainImages = swapChain.getImages();
		}

		uint32_t chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const& surfaceCapabilities)
		{
			auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
			if ((0 < surfaceCapabilities.maxImageCount) && (surfaceCapabilities.maxImageCount < minImageCount))
			{
				minImageCount = surfaceCapabilities.maxImageCount;
			}
			return minImageCount;
		}

		vk::SurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& availableFormats)
		{
			const auto formatIt = std::ranges::find_if(availableFormats,
				[](const auto& format) {
					return format.format == vk::Format::eB8G8R8A8Srgb &&
						format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
				}
			);

			return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
		}

		vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const& availablePresentModes)
		{
			assert(std::ranges::any_of(availablePresentModes, [](auto presentMode) { return presentMode == vk::PresentModeKHR::eFifo; })); // should have eFifo at all times at the least

			vk::PresentModeKHR best_present_mode = vk::PresentModeKHR::eFifo;
			for (const vk::PresentModeKHR& present_mode : availablePresentModes)
			{
				if (present_mode == vk::PresentModeKHR::eImmediate)
				{
					best_present_mode = present_mode;
					break;
				}

				if (best_present_mode != vk::PresentModeKHR::eImmediate && present_mode == vk::PresentModeKHR::eMailbox)
				{
					best_present_mode = present_mode;
					continue;
				}
			}

			return best_present_mode;
		}

		vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR const& capabilities)
		{
			if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) // if not allowed to specify resolution
			{
				return capabilities.currentExtent;
			}
			int width;
			int height;
			glfwGetFramebufferSize(window.get_handle(), &width, &height);

			return {
				std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
				std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
			};
		}

		void createImageViews()
		{
			assert(swapChainImageViews.empty());

			vk::ImageViewCreateInfo imageViewCreateInfo {
				.viewType = vk::ImageViewType::e2D,
				.format = swapChainSurfaceFormat.format,
				.components = {
					vk::ComponentSwizzle::eIdentity,
					vk::ComponentSwizzle::eIdentity,
					vk::ComponentSwizzle::eIdentity,
					vk::ComponentSwizzle::eIdentity
				},
				.subresourceRange = {
					.aspectMask = vk::ImageAspectFlagBits::eColor,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1,
				}
			};

			for (auto& image : swapChainImages)
			{
				imageViewCreateInfo.image = image;
				swapChainImageViews.emplace_back(device, imageViewCreateInfo);
			}
		}

		void createDescriptorSetLayout()
		{
			vk::DescriptorSetLayoutBinding uboLayoutBinding {
				.binding = 0,
				.descriptorType = vk::DescriptorType::eUniformBuffer,
				.descriptorCount = 1,
				.stageFlags = vk::ShaderStageFlagBits::eVertex
			};

			vk::DescriptorSetLayoutCreateInfo layoutInfo {
				.bindingCount = 1,
				.pBindings = &uboLayoutBinding
			};

			descriptorSetLayout = vk::raii::DescriptorSetLayout(device, layoutInfo);
		}

		void createGraphicsPipeline()
		{
			std::vector<char> shaderCode = readFile("shaders/slang.spv");
			vk::raii::ShaderModule shaderModule = createShaderModule(readFile("shaders/slang.spv"));

			vk::PipelineShaderStageCreateInfo vertShaderStageInfo {
				.stage = vk::ShaderStageFlagBits::eVertex,
				.module = shaderModule,
				.pName = "vertMain",
				.pSpecializationInfo = nullptr
			};

			vk::PipelineShaderStageCreateInfo fragShaderStageInfo {
				.stage = vk::ShaderStageFlagBits::eFragment,
				.module = shaderModule,
				.pName = "fragMain",
				.pSpecializationInfo = nullptr
			};

			vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

			std::vector<vk::DynamicState> dynamicStates = {
				vk::DynamicState::eViewport,
				vk::DynamicState::eScissor
			};

			vk::PipelineDynamicStateCreateInfo dynamicState {
				.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
				.pDynamicStates = dynamicStates.data()
			};

			vk::VertexInputBindingDescription bindingDescription = Vertex::getBindingDescription();
			std::array<vk::VertexInputAttributeDescription, 2> attributeDesceriptions = Vertex::getAttributeDescriptions();
			vk::PipelineVertexInputStateCreateInfo vertexInputInfo {
				.vertexBindingDescriptionCount = 1,
				.pVertexBindingDescriptions = &bindingDescription,
				.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDesceriptions.size()),
				.pVertexAttributeDescriptions = attributeDesceriptions.data()
			};

			vk::PipelineInputAssemblyStateCreateInfo inputAssembly {
				.topology = vk::PrimitiveTopology::eTriangleList
			};

			// vk::Viewport viewport {
			// 	.x = 0.0f,
			// 	.y = 0.0f,
			// 	.width = static_cast<float>(swapChainExtent.width),
			// 	.height = static_cast<float>(swapChainExtent.height),
			// 	.minDepth = 0.0f,
			// 	.maxDepth = 1.0f
			// };
			//
			// vk::Rect2D scissor {
			// 	.offset = vk::Offset2D { 0, 0 },
			// 	.extent = swapChainExtent
			// };

			vk::PipelineViewportStateCreateInfo viewportState {
				.viewportCount = 1,
				// .pViewports = &viewport,
				.scissorCount = 1,
				// .pScissors = &scissor
			};

			vk::PipelineRasterizationStateCreateInfo rasterizer {
				.depthClampEnable = false,
				.rasterizerDiscardEnable = false,
				.polygonMode = vk::PolygonMode::eFill,
				.cullMode = vk::CullModeFlagBits::eBack,
				.frontFace = vk::FrontFace::eCounterClockwise,
				// .frontFace = vk::FrontFace::eClockwise,
				.depthBiasEnable = false,
				.lineWidth = 1.0f
			};

			vk::PipelineMultisampleStateCreateInfo multisampling {
				.rasterizationSamples = vk::SampleCountFlagBits::e1,
				.sampleShadingEnable = false
			};

			vk::PipelineColorBlendAttachmentState colorBlendAttachment {
				.blendEnable = true,
				.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
				.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
				.colorBlendOp = vk::BlendOp::eAdd,
				.srcAlphaBlendFactor = vk::BlendFactor::eOne,
				.dstAlphaBlendFactor = vk::BlendFactor::eZero,
				.alphaBlendOp = vk::BlendOp::eAdd,
				.colorWriteMask =
					vk::ColorComponentFlagBits::eR |
					vk::ColorComponentFlagBits::eG |
					vk::ColorComponentFlagBits::eB |
					vk::ColorComponentFlagBits::eA
			};

			vk::PipelineColorBlendStateCreateInfo colorBlending
			{
				.logicOpEnable = false,
				.logicOp = vk::LogicOp::eCopy,
				.attachmentCount = 1,
				.pAttachments = &colorBlendAttachment
			};

			vk::PipelineLayoutCreateInfo pipelineLayoutInfo {
				.setLayoutCount = 1,
				.pSetLayouts = &*descriptorSetLayout,
				.pushConstantRangeCount = 0
			};

			pipelineLayout = vk::raii::PipelineLayout(device, pipelineLayoutInfo);

			vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo {
				.colorAttachmentCount = 1,
				.pColorAttachmentFormats = &swapChainSurfaceFormat.format
			};

			vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo {
				.stageCount = 2,
				.pStages = shaderStages,
				.pVertexInputState = &vertexInputInfo,
				.pInputAssemblyState = &inputAssembly,
				.pViewportState = &viewportState,
				.pRasterizationState = &rasterizer,
				.pMultisampleState = &multisampling,
				.pColorBlendState = &colorBlending,
				.pDynamicState = &dynamicState,
				.layout = pipelineLayout,
				.renderPass = nullptr // dynamic rendering
			};

			vk::StructureChain<
				vk::GraphicsPipelineCreateInfo,
				vk::PipelineRenderingCreateInfo
			> pipelineCreateInfoChain = { graphicsPipelineCreateInfo , pipelineRenderingCreateInfo };

			graphicsPipeline = vk::raii::Pipeline(device, nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
		}

		[[nodiscard]]
		vk::raii::ShaderModule createShaderModule(const std::vector<char>& code) const
		{
			vk::ShaderModuleCreateInfo createInfo {
				.codeSize = code.size() * sizeof(char),
				.pCode = reinterpret_cast<const uint32_t*>(code.data())
			};
			vk::raii::ShaderModule shaderModule(device, createInfo);
			return shaderModule;
		}

		static std::vector<char> readFile(const std::string& filename)
		{
			std::ifstream file(filename, std::ios::ate | std::ios::binary);

			if (!file.is_open())
			{
				throw std::runtime_error("failed to open file");
			}

			std::vector<char> buffer(file.tellg());
			file.seekg(0, std::ios::beg);
			file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
			file.close();

			return buffer;
		}

		void createCommandPool()
		{
			vk::CommandPoolCreateInfo poolInfo {
				.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
				.queueFamilyIndex = queueIndex
			};
			commandPool = vk::raii::CommandPool(device, poolInfo);
		}

		void createCommandBuffers()
		{
			vk::CommandBufferAllocateInfo allocInfo {
				.commandPool = commandPool,
				.level = vk::CommandBufferLevel::ePrimary,
				.commandBufferCount = MAX_FRAMES_IN_FLIGHT
			};
			commandBuffers = vk::raii::CommandBuffers(device, allocInfo);
			// commandBuffer = std::move(vk::raii::CommandBuffers(device, allocInfo).front());
		}

		void createVertexBuffer()
		{
			vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

			auto [stagingBuffer, stagingBufferMemory] = createBuffer(
				bufferSize,
				vk::BufferUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
			);

			void* dataStaging = stagingBufferMemory.mapMemory(0, bufferSize);
			memcpy(dataStaging, vertices.data(), bufferSize);
			stagingBufferMemory.unmapMemory();
			// eHostCoherent guarantees after this point that the data is actually there. Could also call a function to flush the memory
			// however eHostCoherent is less performant than explicit flushing
			// this will be uploaded to the gpu asynchronously, and will be complete as of the next call to vk::raii::Queue::submit

			std::tie(vertexBuffer, vertexBufferMemory) = createBuffer(
				bufferSize,
				vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
				vk::MemoryPropertyFlagBits::eDeviceLocal
			);

			copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
		}

		void copyBuffer(vk::raii::Buffer& srcBuffer, vk::raii::Buffer& dstBuffer, vk::DeviceSize size)
		{
			// use a separate command pool for these short-lived buffers, for optimizations.
			// vk::CommandPoolCreateFlagBits::eTransient would be needed in command pool generation, in that case.

			vk::CommandBufferAllocateInfo allocInfo {
				.commandPool = commandPool,
				.level = vk::CommandBufferLevel::ePrimary,
				.commandBufferCount = 1
			};

			vk::raii::CommandBuffer commandCopyBuffer = std::move(device.allocateCommandBuffers(allocInfo).front());

			commandCopyBuffer.begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
			commandCopyBuffer.copyBuffer(*srcBuffer, *dstBuffer, vk::BufferCopy(0, 0, size));
			commandCopyBuffer.end();

			graphicsQueue.submit(vk::SubmitInfo {
				.commandBufferCount = 1,
				.pCommandBuffers = &*commandCopyBuffer
			}, nullptr);
			graphicsQueue.waitIdle();
			// we can use vk::raii::waitForFences instead if we want to schedule multiple transfers simultaneously
		}

		std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
		{
			vk::BufferCreateInfo bufferInfo {
				.size = size,
				.usage = usage,
				.sharingMode = vk::SharingMode::eExclusive
			};

			vk::raii::Buffer buffer = vk::raii::Buffer(device, bufferInfo);
			vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();

			vk::MemoryAllocateInfo allocInfo = {
				.allocationSize = memRequirements.size,
				.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties)
			};

			vk::raii::DeviceMemory bufferMemory = vk::raii::DeviceMemory(device, allocInfo);
			buffer.bindMemory(*bufferMemory, 0); // second parameter is the offset within the region of memory
			return {
				std::move(buffer),
				std::move(bufferMemory)
			};
		}

		void createIndexBuffer()
		{
			// this is essentially the same as createVertexBuffer...
			vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

			auto [stagingBuffer, stagingBufferMemory] = createBuffer(
				bufferSize,
				vk::BufferUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent // PERF eHostCoherent
			);

			void* dataStaging = stagingBufferMemory.mapMemory(0, bufferSize);
			memcpy(dataStaging, indices.data(), bufferSize);
			stagingBufferMemory.unmapMemory();

			std::tie(indexBuffer, indexBufferMemory) = createBuffer(
				bufferSize,
				vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
				vk::MemoryPropertyFlagBits::eDeviceLocal
			);

			copyBuffer(stagingBuffer, indexBuffer, bufferSize);
		}

		void createUniformBuffers()
		{
			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			{
				vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
				auto [buffer, bufferMem] = createBuffer(
					bufferSize,
					vk::BufferUsageFlagBits::eUniformBuffer,
					vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
				);
				uniformBuffers.emplace_back(std::move(buffer));
				uniformBuffersMemory.emplace_back(std::move(bufferMem));
				uniformBuffersMapped.emplace_back(uniformBuffersMemory.back().mapMemory(0, bufferSize));
			}
		}

		void createDescriptorPool()
		{
			vk::DescriptorPoolSize poolSize {
				.type = vk::DescriptorType::eUniformBuffer,
				.descriptorCount = MAX_FRAMES_IN_FLIGHT
			};

			vk::DescriptorPoolCreateInfo poolInfo {
				.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
				.maxSets = MAX_FRAMES_IN_FLIGHT,
				.poolSizeCount = 1,
				.pPoolSizes = &poolSize
			};

			descriptorPool = vk::raii::DescriptorPool(device, poolInfo);
		}

		void createDescriptorSets()
		{
			std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *descriptorSetLayout);

			vk::DescriptorSetAllocateInfo allocInfo {
				.descriptorPool = descriptorPool,
				.descriptorSetCount = static_cast<uint32_t>(layouts.size()),
				.pSetLayouts = layouts.data()
			};

			descriptorSets = device.allocateDescriptorSets(allocInfo);

			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			{
				vk::DescriptorBufferInfo bufferInfo {
					.buffer = uniformBuffers[i],
					.offset = 0,
					.range = sizeof(UniformBufferObject)
				};

				vk::WriteDescriptorSet descriptorWrite {
					.dstSet = descriptorSets[i],
					.dstBinding = 0,
					.dstArrayElement = 0,
					.descriptorCount = 1,
					.descriptorType = vk::DescriptorType::eUniformBuffer,
					.pBufferInfo = &bufferInfo
				};

				device.updateDescriptorSets(descriptorWrite, {});
			}
		}

		uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
		{
			// selecting a specific heap "memoryHeaps" may increase performance
			vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

			for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
			{
				if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
				{
					return i;
				}
			}

			throw std::runtime_error("failed to find suitable memory type!");
		}

		void recordCommandBuffer(uint32_t frameIndex, uint32_t imageIndex)
		{
			vk::raii::CommandBuffer& commandBuffer = commandBuffers[frameIndex];
			vk::CommandBufferBeginInfo beginInfo {};
			commandBuffer.begin(beginInfo);

			transition_image_layout(
				commandBuffer,
				imageIndex,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eColorAttachmentOptimal,
				{},
				vk::AccessFlagBits2::eColorAttachmentWrite,
				vk::PipelineStageFlagBits2::eColorAttachmentOutput,
				vk::PipelineStageFlagBits2::eColorAttachmentOutput
			);

			vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);

			vk::RenderingAttachmentInfo attachmentInfo = {
				.imageView = swapChainImageViews[imageIndex],
				.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
				.loadOp = vk::AttachmentLoadOp::eClear,
				.storeOp = vk::AttachmentStoreOp::eStore,
				.clearValue = clearColor
			};

			vk::RenderingInfo renderingInfo = {
				.renderArea = {
					.offset = {0, 0},
					.extent = swapChainExtent
				},
				.layerCount = 1,
				.colorAttachmentCount = 1,
				.pColorAttachments = &attachmentInfo
			};

			// TODO needs synchronization here?
			commandBuffer.beginRendering(renderingInfo);
			commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline);

			commandBuffer.bindVertexBuffers(0, *vertexBuffer, {0});
			commandBuffer.bindIndexBuffer(*indexBuffer, 0, vk::IndexType::eUint16);

			vk::Viewport viewport {
				.x = 0.0f,
				// .y = 0.0f,
				.y = static_cast<float>(swapChainExtent.height), // because of glm, to flip Y coordinate
				.width = static_cast<float>(swapChainExtent.width),
				.height = -static_cast<float>(swapChainExtent.height), // minus sign because of glm, which assumes Y coordinate is flipped
				.minDepth = 0.0f,
				.maxDepth = 1.0f
			};

			vk::Rect2D scissor {
				.offset = vk::Offset2D { 0, 0 },
				.extent = swapChainExtent
			};

			commandBuffer.setViewport(0, viewport);
			commandBuffer.setScissor(0, scissor);

			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, *descriptorSets[frameIndex], nullptr);
			// commandBuffer.draw(3, 1, 0, 0);
			// commandBuffer.draw(vertices.size(), 1, 0, 0);
			commandBuffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

			commandBuffer.endRendering();
			// TODO needs synchronization here?

			transition_image_layout(
				commandBuffer,
				imageIndex,
				vk::ImageLayout::eColorAttachmentOptimal,
				vk::ImageLayout::ePresentSrcKHR,
				vk::AccessFlagBits2::eColorAttachmentWrite,
				{},
				vk::PipelineStageFlagBits2::eColorAttachmentOutput,
				vk::PipelineStageFlagBits2::eBottomOfPipe
			);

			commandBuffer.end();
		}

		void transition_image_layout(
			vk::raii::CommandBuffer& commandBuffer,
			uint32_t imageIndex,
			vk::ImageLayout old_layout,
			vk::ImageLayout new_layout,
			vk::AccessFlags2 src_access_mask,
			vk::AccessFlags2 dst_access_mask,
			vk::PipelineStageFlags2 src_stage_mask,
			vk::PipelineStageFlags2 dst_stage_mask
		)
		{
			vk::ImageMemoryBarrier2 barrier = {
				.srcStageMask = src_stage_mask,
				.srcAccessMask = src_access_mask,
				.dstStageMask = dst_stage_mask,
				.dstAccessMask = dst_access_mask,
				.oldLayout = old_layout,
				.newLayout = new_layout,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.image = swapChainImages[imageIndex],
				.subresourceRange = {
					.aspectMask = vk::ImageAspectFlagBits::eColor,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1
				}
			};

			vk::DependencyInfo dependency_info = {
				.dependencyFlags = {},
				.imageMemoryBarrierCount = 1,
				.pImageMemoryBarriers = &barrier
			};

			commandBuffer.pipelineBarrier2(dependency_info);
		}

		void createSyncObjects()
		{
			assert(presentCompleteSemaphores.empty() && renderFinishedSemaphores.empty() && inFlightFences.empty());

			for (size_t i = 0; i < swapChainImages.size(); i++)
			{
				renderFinishedSemaphores.emplace_back(device, vk::SemaphoreCreateInfo());
			}

			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			{
				presentCompleteSemaphores.emplace_back(device, vk::SemaphoreCreateInfo());
				inFlightFences.emplace_back(device, vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
			}
		}

		void mainLoop()
		{
			while (!glfwWindowShouldClose(window.get_handle()))
			{
				glfwPollEvents();
				drawFrame();
			}

			device.waitIdle();
		}

		void drawFrame()
		{
			auto fenceResult = device.waitForFences(*inFlightFences[frameIndex], true, std::numeric_limits<uint64_t>::max());
			if (fenceResult != vk::Result::eSuccess)
			{
				throw std::runtime_error("failted to wait for fence!");
			}

			auto [result, imageIndex] = swapChain.acquireNextImage(std::numeric_limits<uint64_t>::max(), *presentCompleteSemaphores[frameIndex], nullptr);

			if (result == vk::Result::eErrorOutOfDateKHR)
			{
				framebufferResized = false;
				recreateSwapChain();
				return;
			}
			if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
			{
				assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
				throw std::runtime_error("failed to acquire swapchain image!");
			}

			updateUniformBuffer(frameIndex);

			device.resetFences(*inFlightFences[frameIndex]);

			commandBuffers[frameIndex].reset();
			recordCommandBuffer(frameIndex, imageIndex);

			vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
			const vk::SubmitInfo submitInfo {
				.waitSemaphoreCount = 1,
				.pWaitSemaphores = &*presentCompleteSemaphores[frameIndex],
				.pWaitDstStageMask = &waitDestinationStageMask,
				.commandBufferCount = 1,
				.pCommandBuffers = &*commandBuffers[frameIndex],
				.signalSemaphoreCount = 1,
				.pSignalSemaphores = &*renderFinishedSemaphores[imageIndex]
			};

			graphicsQueue.submit(submitInfo, *inFlightFences[frameIndex]); // TODO error handling

			const vk::PresentInfoKHR presentInfoKHR {
				.waitSemaphoreCount = 1,
				.pWaitSemaphores = &*renderFinishedSemaphores[imageIndex],
				.swapchainCount = 1,
				.pSwapchains = &*swapChain,
				.pImageIndices = &imageIndex,
				.pResults = nullptr
			};

			result = graphicsQueue.presentKHR(presentInfoKHR);
			if ((result == vk::Result::eSuboptimalKHR) || (result == vk::Result::eErrorOutOfDateKHR) || framebufferResized)
			{
				framebufferResized = false;
				recreateSwapChain();
			}
			else
			{
				assert(result == vk::Result::eSuccess);
			}

			frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
		}

		void updateUniformBuffer(uint32_t currentImage)
		{
			// using a UBO this way is not the most efficient way to pass frequently changing values to the shader.
			// a more efficient way to pass a small buffer of data to shaders is "push constants".
			static auto startTime = std::chrono::high_resolution_clock::now();

			auto currentTime = std::chrono::high_resolution_clock::now();
			float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

			UniformBufferObject ubo{};
			ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			ubo.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height), 0.1f, 10.0f);

			memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
		}

		void cleanupSwapChain()
		{
			swapChainImageViews.clear();
			swapChain = nullptr;
		}

		void recreateSwapChain()
		{
			std::cout << "recreating\n";
			window.block_while_minimized();

			// slow, instead of waiting, just pass the old swapchain as oldSwapchain in the new swapchain
			// and destroy the old swapchain as soon as we're finished using it
			device.waitIdle();

			cleanupSwapChain();

			createSwapChain();
			createImageViews();
		}

		void framebufferResizeCallback(int , int )
		{
			framebufferResized = true;
		}

		void onFocusEvent(bool focus)
		{
			if (!focus)
			{
				window.block_while_unfocused();
			}
		}

	public:
		VulkanTutorial(Window& window) : window(window)
		{
			setWindowSettings();
			createInstance();
			setupDebugMessenger();
			createSurface();
			pickPhysicalDevice();
			createLogicalDevice();
			createSwapChain();
			createImageViews();
			createDescriptorSetLayout();
			createGraphicsPipeline();
			createCommandPool();
			createVertexBuffer();
			createIndexBuffer();
			createUniformBuffers();
			createDescriptorPool();
			createDescriptorSets();
			createCommandBuffers();
			createSyncObjects();
		}

		void run()
		{
			mainLoop();
		}
	};
}
