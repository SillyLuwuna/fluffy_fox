#pragma once

#include "engine/resources/mesh.hpp"
#include "rendering/swapchain.hpp"
#include <iostream>

// #define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

#include "rendering/window.hpp"
#include "rendering/device.hpp"
#include "rendering/pipeline.hpp"
#include "rendering/uniform_buffer.hpp"
#include "rendering/descriptor_writer.hpp"
#include "rendering/descriptor_allocator.hpp"

// TODO push constants
// TODO instancing
// TODO batch draw calls
// TODO indirect drawing

// TODO imports for vulkan and other libraries

namespace fluffy_fox::rendering
{
	constexpr uint32_t WIDTH = 800;
	constexpr uint32_t HEIGHT = 600;
	constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

	// const std::string MODEL_PATH = "models/viking_room.gltf";
	const std::string MODEL_PATH = "models/source/kiu_1015162433_texture.glb";
	// const std::string MODEL_PATH = "models/lowpoly_fox.glb";
	// const std::string MODEL_PATH = "models/shiba.glb";
	const bool USE_MODEL_TEXTURE = true;

	constexpr int MAX_OBJECTS = 3;


	const std::vector<char const*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

#ifdef DEBUG
		constexpr bool enableValidationLayers = true;
#else
		constexpr bool enableValidationLayers = false;
#endif

	struct UniformBufferObject
	{
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};

	struct GameObject
	{
		// transform properties
		glm::vec3 position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

		// uniform buffer for this object (one per frame in flight)
		std::vector<UniformBuffer<UniformBufferObject>> uniformBuffers;

		// descriptor sets for this object (one per frame in flight)
		std::vector<vk::raii::DescriptorSet> descriptorSets;

		// calculate model matrix based on position, rotation and scale
		glm::mat4 getModelMatrix() const
		{
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, position);
			model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::rotate(model, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::rotate(model, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
			model = glm::scale(model, scale);
			return model;
		}
	};
}

namespace fluffy_fox::rendering
{
	class VulkanTutorial
	{
	private:
		Window& window;
		vk::raii::Context context;
		vk::raii::Instance instance = nullptr;
		vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;

		std::unique_ptr<ResourceManager> resource_manager;
		vk::raii::SurfaceKHR surface = nullptr; // FIXME
		std::unique_ptr<Swapchain> swapchain;
		std::unique_ptr<Pipeline> pipeline;
		ResourceHandle<Mesh> mesh;
		std::unique_ptr<DescriptorAllocator> descriptor_allocator;

		std::vector<vk::raii::CommandBuffer> commandBuffers;
		std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
		std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
		std::vector<vk::raii::Fence> inFlightFences;
		uint32_t frameIndex = 0;

		bool framebufferResized = false;

		std::vector<GameObject> gameObjects;

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

		void setupGameObjects()
		{
			gameObjects.emplace_back(GameObject());
			gameObjects.emplace_back(GameObject());
			gameObjects.emplace_back(GameObject());

			gameObjects[0].position = {0.0f, 0.0f, 0.0f};
			gameObjects[0].rotation = {0.0f, 0.0f, 0.0f};
			gameObjects[0].scale = {1.0f, 1.0f, 1.0f};

			gameObjects[1].position = {-2.0f, 0.0f, -1.0f};
			gameObjects[1].rotation = {0.0f, glm::radians(45.0f), 0.0f};
			gameObjects[1].scale = {0.75f, 0.75f, 0.75f};

			gameObjects[2].position = {0.5f, -1.8f, 0.5f};
			gameObjects[2].rotation = {0.0f, glm::radians(-45.0f), 0.0f};
			gameObjects[2].scale = {0.75f, 0.75f, 0.75f};

			for (auto& gameObject : gameObjects)
			{
				gameObject.descriptorSets = descriptor_allocator->allocate(
					*pipeline->descriptor_set_layout(),
					MAX_FRAMES_IN_FLIGHT
				);

				for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
				{
					gameObject.uniformBuffers.emplace_back(UniformBuffer<UniformBufferObject>());

					DescriptorWriter()
						.write(0, gameObject.uniformBuffers[i])
						.write(1, *mesh->textures()[0]) // FIXME only for primitive 0
						.update(gameObject.descriptorSets[i]);
				}
			}
		}

		void createCommandBuffers()
		{
			vk::CommandBufferAllocateInfo allocInfo {
				.commandPool = Device::instance().graphics_command_pool(),
				.level = vk::CommandBufferLevel::ePrimary,
				.commandBufferCount = MAX_FRAMES_IN_FLIGHT
			};
			commandBuffers = vk::raii::CommandBuffers(Device::instance().logical(), allocInfo);
			// commandBuffer = std::move(vk::raii::CommandBuffers(device, allocInfo).front());
		}

		std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
		{
			vk::BufferCreateInfo bufferInfo {
				.size = size,
				.usage = usage,
				.sharingMode = vk::SharingMode::eExclusive
			};

			vk::raii::Buffer buffer = vk::raii::Buffer(Device::instance().logical(), bufferInfo);
			vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();

			vk::MemoryAllocateInfo allocInfo = {
				.allocationSize = memRequirements.size,
				.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties)
			};

			vk::raii::DeviceMemory bufferMemory = vk::raii::DeviceMemory(Device::instance().logical(), allocInfo);
			buffer.bindMemory(*bufferMemory, 0); // second parameter is the offset within the region of memory
			return {
				std::move(buffer),
				std::move(bufferMemory)
			};
		}

		uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
		{
			// selecting a specific heap "memoryHeaps" may increase performance
			vk::PhysicalDeviceMemoryProperties memProperties = Device::instance().physical().getMemoryProperties();

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
				swapchain->images()[imageIndex],
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eColorAttachmentOptimal,
				{},
				vk::AccessFlagBits2::eColorAttachmentWrite,
				vk::PipelineStageFlagBits2::eColorAttachmentOutput,
				vk::PipelineStageFlagBits2::eColorAttachmentOutput,
				vk::ImageAspectFlagBits::eColor
			);

			transition_image_layout(
				commandBuffer,
				*swapchain->color().image,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eColorAttachmentOptimal,
				vk::AccessFlagBits2::eColorAttachmentWrite,
				vk::AccessFlagBits2::eColorAttachmentWrite,
				vk::PipelineStageFlagBits2::eColorAttachmentOutput,
				vk::PipelineStageFlagBits2::eColorAttachmentOutput,
				vk::ImageAspectFlagBits::eColor
			);

			transition_image_layout(
				commandBuffer,
				*swapchain->depth().image,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eDepthAttachmentOptimal,
				vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
				vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
				vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
				vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
				vk::ImageAspectFlagBits::eDepth
			);

			vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
			vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);

			vk::RenderingAttachmentInfo colorAttachmentInfo = {
				.imageView = swapchain->color().view,
				.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
				.resolveMode = vk::ResolveModeFlagBits::eAverage,
				.resolveImageView = swapchain->image_views()[imageIndex],
				.resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal,
				.loadOp = vk::AttachmentLoadOp::eClear,
				.storeOp = vk::AttachmentStoreOp::eStore,
				.clearValue = clearColor
			};

			vk::RenderingAttachmentInfo depthAttachmentInfo = {
				.imageView = swapchain->depth().view,
				.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
				.loadOp = vk::AttachmentLoadOp::eClear,
				.storeOp = vk::AttachmentStoreOp::eDontCare,
				.clearValue = clearDepth
			};

			vk::RenderingInfo renderingInfo = {
				.renderArea = {
					.offset = {0, 0},
					.extent = swapchain->extent()
				},
				.layerCount = 1,
				.colorAttachmentCount = 1,
				.pColorAttachments = &colorAttachmentInfo,
				.pDepthAttachment = &depthAttachmentInfo
			};

			commandBuffer.beginRendering(renderingInfo);
			commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline->pipeline());

			commandBuffer.bindVertexBuffers(0, *mesh->vertex_buffers()[0].buffer, {0}); // FIXME only for primitive 0, literally accepts an array
			commandBuffer.bindIndexBuffer(mesh->geometry()[0].index_buffer(), 0, vk::IndexTypeValue<u32>::value); // FIXME only for primitive 0

			vk::Viewport viewport {
				.x = 0.0f,
				// .y = 0.0f,
				.y = static_cast<float>(swapchain->extent().height), // because of glm, to flip Y coordinate
				.width = static_cast<float>(swapchain->extent().width),
				.height = -static_cast<float>(swapchain->extent().height), // minus sign because of glm, which assumes Y coordinate is flipped
				// .height = static_cast<float>(swapchain->extent().height),
				.minDepth = 0.0f,
				.maxDepth = 1.0f
			};

			vk::Rect2D scissor {
				.offset = vk::Offset2D { 0, 0 },
				.extent = swapchain->extent()
			};

			commandBuffer.setViewport(0, viewport);
			commandBuffer.setScissor(0, scissor);

			for (const auto& gameObject : gameObjects)
			{
				commandBuffer.bindDescriptorSets(
					vk::PipelineBindPoint::eGraphics,
					pipeline->layout(),
					0,
					*gameObject.descriptorSets[frameIndex],
					nullptr
				);

				commandBuffer.drawIndexed(mesh->geometry()[0].index_count(), 1, 0, 0, 0); // FIXME only for primitive 0
			}

			commandBuffer.endRendering();


			transition_image_layout(
				commandBuffer,
				swapchain->images()[imageIndex],
				vk::ImageLayout::eColorAttachmentOptimal,
				vk::ImageLayout::ePresentSrcKHR,
				vk::AccessFlagBits2::eColorAttachmentWrite,
				{},
				vk::PipelineStageFlagBits2::eColorAttachmentOutput,
				vk::PipelineStageFlagBits2::eBottomOfPipe,
				vk::ImageAspectFlagBits::eColor
			);

			commandBuffer.end();
		}

		void transition_image_layout(
			vk::raii::CommandBuffer& commandBuffer,
			vk::Image image,
			vk::ImageLayout old_layout,
			vk::ImageLayout new_layout,
			vk::AccessFlags2 src_access_mask,
			vk::AccessFlags2 dst_access_mask,
			vk::PipelineStageFlags2 src_stage_mask,
			vk::PipelineStageFlags2 dst_stage_mask,
			vk::ImageAspectFlags image_aspect_flags
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
				.image = image,
				.subresourceRange = {
					.aspectMask = image_aspect_flags,
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

			for (size_t i = 0; i < swapchain->images().size(); i++)
			{
				renderFinishedSemaphores.emplace_back(Device::instance().logical(), vk::SemaphoreCreateInfo());
			}

			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			{
				presentCompleteSemaphores.emplace_back(Device::instance().logical(), vk::SemaphoreCreateInfo());
				inFlightFences.emplace_back(Device::instance().logical(), vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
			}
		}

		void mainLoop()
		{
			while (!glfwWindowShouldClose(window.get_handle()))
			{
				glfwPollEvents();
				drawFrame();
			}

			Device::instance().logical().waitIdle();
		}

		void drawFrame()
		{
			auto fenceResult = Device::instance().logical().waitForFences(*inFlightFences[frameIndex], true, std::numeric_limits<uint64_t>::max());
			if (fenceResult != vk::Result::eSuccess)
			{
				throw std::runtime_error("failted to wait for fence!");
			}

			auto [result, imageIndex] = swapchain->swapchain().acquireNextImage(std::numeric_limits<uint64_t>::max(), *presentCompleteSemaphores[frameIndex], nullptr);

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

			updateUniformBuffers(frameIndex);

			Device::instance().logical().resetFences(*inFlightFences[frameIndex]);

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

			Device::instance().graphics().submit(submitInfo, *inFlightFences[frameIndex]); // TODO error handling

			const vk::PresentInfoKHR presentInfoKHR {
				.waitSemaphoreCount = 1,
				.pWaitSemaphores = &*renderFinishedSemaphores[imageIndex],
				.swapchainCount = 1,
				.pSwapchains = &*swapchain->swapchain(),
				.pImageIndices = &imageIndex,
				.pResults = nullptr
			};

			result = Device::instance().graphics().presentKHR(presentInfoKHR);
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

		void updateUniformBuffers(int32_t frameIndex)
		{
			// using a UBO this way is not the most efficient way to pass frequently changing values to the shader.
			// a more efficient way to pass a small buffer of data to shaders is "push constants".
			static auto startTime = std::chrono::high_resolution_clock::now();
			auto currentTime = std::chrono::high_resolution_clock::now();
			float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

			glm::mat4 view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			glm::mat4 proj = glm::perspective(glm::radians(45.0f), static_cast<float>(swapchain->extent().width) / static_cast<float>(swapchain->extent().height), 0.1f, 10.0f);

			for (auto& gameObject : gameObjects)
			{
				// gameObject.rotation.y += 0.001f;

				glm::mat4 initialRotation = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
				glm::mat4 model = gameObject.getModelMatrix() * initialRotation;

				UniformBufferObject ubo {
					.model = model,
					.view = view,
					.proj = proj
				};

				gameObject.uniformBuffers[frameIndex].set(ubo);
			}
		}

		void recreateSwapChain()
		{
			window.block_while_minimized();

			// slow, instead of waiting, just pass the old swapchain as oldSwapchain in the new swapchain
			// and destroy the old swapchain as soon as we're finished using it
			Device::instance().logical().waitIdle();

			swapchain->recreate(window);
			// check if valid
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
			// FIXME load time
			setWindowSettings();
			createInstance();
			setupDebugMessenger();

			// FIXME the vulkan instance should be the singleton
			// Device shouldn't be a singleton, it should be
			// accessed through something like Instance::instance().device()
			// either that or make Device hold the instance

			resource_manager = std::make_unique<ResourceManager>();

			// TODO make an overarching base class for render components, which has is_valid() method
			if (!window.create_surface(instance)) { throw std::runtime_error("failed to create window surface!"); }

			Device::create(instance, *window.surface());
			if (!Device::instance().is_valid()) { throw std::runtime_error("failed to create device!"); }

			swapchain = std::make_unique<Swapchain>(window);
			if (!swapchain->is_valid()) { throw std::runtime_error("failed to create swapchain!"); }

			pipeline = std::make_unique<Pipeline>(*resource_manager, *swapchain);
			if (!pipeline->is_valid()) { throw std::runtime_error("failed to create pipeline!"); }

			ResourceHandle<GltfFile> gltf_file = resource_manager->load<GltfFile>(MODEL_PATH);
			mesh = resource_manager->load<Mesh>(MODEL_PATH, *resource_manager, std::move(gltf_file), 0); // FIXME only mesh 0
			if (!mesh) { throw std::runtime_error("failed to load mesh!"); } // FIXME if this errors where it was erroring, the program freaks out

			descriptor_allocator = std::make_unique<DescriptorAllocator>(
				MAX_OBJECTS * MAX_FRAMES_IN_FLIGHT,
				*pipeline
			);

			setupGameObjects();

			createCommandBuffers();
			createSyncObjects();

			// FIXME make exports unified in a single file
		}

		~VulkanTutorial()
		{
			gameObjects.clear();

			surface = window.take_surface(); // FIXME

			// FIXME
			descriptor_allocator = nullptr;
			commandBuffers.clear();
			presentCompleteSemaphores.clear();
			renderFinishedSemaphores.clear();
			inFlightFences.clear();
			mesh = nullptr;
			pipeline = nullptr;
			swapchain = nullptr;
			resource_manager = nullptr;
			Device::release(); // FIXME not very clean

			// window.destroy_surface();
		}

		void run()
		{
			mainLoop();
		}
	};
}
