#pragma once

#include "rendering/swapchain.hpp"
#include <iostream>
// #define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#define TINYGLTF3_ENABLE_STB_IMAGE
#define TINYGLTF3_ENABLE_STB_IMAGE_WRITE
#define TINYGLTF3_ENABLE_FS
#define TINYGLTF3_IMPLEMENTATION
#include <tiny_gltf_v3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "rendering/window.hpp"
#include "rendering/device.hpp"
#include "rendering/pipeline.hpp"
#include "rendering/vertex.hpp"

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
	const std::string TEXTURE_PATH = "textures/viking_room.png";
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

	struct DecodedImage
	{
		std::unique_ptr<uint8_t[]> pixels;
		int width;
		int height;
		int channels;
		uint64_t bytes;
	};

	struct GameObject
	{
		// transform properties
		glm::vec3 position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

		// uniform buffer for this object (one per frame in flight)
		std::vector<vk::raii::Buffer> uniformBuffers;
		std::vector<vk::raii::DeviceMemory> uniformBuffersMemory;
		std::vector<void*> uniformBuffersMapped;

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

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		// PERF vertexBuffer and indexBuffer should be the same buffer for performance
		vk::raii::Buffer vertexBuffer = nullptr;
		vk::raii::DeviceMemory vertexBufferMemory = nullptr;
		vk::raii::Buffer indexBuffer = nullptr;
		vk::raii::DeviceMemory indexBufferMemory = nullptr;

		vk::raii::DescriptorPool descriptorPool = nullptr;

		uint32_t mipLevels = 0;
		vk::raii::Image textureImage = nullptr;
		vk::raii::DeviceMemory textureImageMemory = nullptr;
		vk::raii::ImageView textureImageView = nullptr;
		vk::raii::Sampler textureSampler = nullptr;
		bool hasTextureDecoded = false;
		DecodedImage textureDecoded;

		std::vector<vk::raii::CommandBuffer> commandBuffers;
		std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
		std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
		std::vector<vk::raii::Fence> inFlightFences;
		uint32_t frameIndex = 0;

		bool framebufferResized = false;

		std::array<GameObject, MAX_OBJECTS> gameObjects;

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

		void createTextureImage()
		{
			if (!USE_MODEL_TEXTURE)
			{
				// TODO move into model load
				stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &textureDecoded.width, &textureDecoded.height, &textureDecoded.channels, STBI_rgb_alpha);

				if (!pixels)
				{
					throw std::runtime_error("failed to load texture image!");
				}

				textureDecoded.bytes = textureDecoded.width * textureDecoded.height * 4; // TODO put this in struct method
				textureDecoded.pixels = std::make_unique<uint8_t[]>(textureDecoded.bytes);
				memcpy(textureDecoded.pixels.get(), pixels, textureDecoded.bytes);

				stbi_image_free(pixels);
			}
			else
			{
				if (!hasTextureDecoded)
				{
					throw std::runtime_error("failed to load texture image!");
				}
			}

			mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(textureDecoded.width, textureDecoded.height)))) + 1;
			auto [stagingBuffer, stagingBufferMemory] = createBuffer(
				textureDecoded.bytes,
				vk::BufferUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
			);

			void* data = stagingBufferMemory.mapMemory(0, textureDecoded.bytes);
			memcpy(data, textureDecoded.pixels.get(), textureDecoded.bytes);
			stagingBufferMemory.unmapMemory();

			std::tie(textureImage, textureImageMemory) = createImage(
				textureDecoded.width,
				textureDecoded.height,
				mipLevels,
				vk::SampleCountFlagBits::e1,
				vk::Format::eR8G8B8A8Srgb,
				vk::ImageTiling::eOptimal,
				vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
				vk::MemoryPropertyFlagBits::eDeviceLocal
			);

			vk::raii::CommandBuffer commandBuffer = beginSingleTimeCommands();
			transitionImageLayout(commandBuffer, textureImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, mipLevels);
			copyBufferToImage(commandBuffer, stagingBuffer, textureImage, static_cast<uint32_t>(textureDecoded.width), static_cast<uint32_t>(textureDecoded.height));

			// transitioned to vk::ImageLayout::eShaderReadOnlyOptimal while generating mipmaps
			generateMipmaps(commandBuffer, textureImage, vk::Format::eR8G8B8A8Srgb, textureDecoded.width, textureDecoded.height, mipLevels);
			endSingleTimeCommands(std::move(commandBuffer));

			textureImageView = createImageView(*textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor, mipLevels);
		}

		void generateMipmaps(
			vk::raii::CommandBuffer& commandBuffer,
			vk::raii::Image& image,
			vk::Format imageFormat,
			int32_t texWidth,
			int32_t texHeight,
			uint32_t mipLevels
		)
		{
			// PERF usually done once and stored. Not generated each time.

			vk::FormatProperties formatProperties = Device::instance().physical().getFormatProperties(imageFormat);
			if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear))
			{
				throw std::runtime_error("texture image format does not support linear blitting!");
			}

			vk::ImageMemoryBarrier barrier = {
				.srcAccessMask = vk::AccessFlagBits::eTransferWrite,
				.dstAccessMask = vk::AccessFlagBits::eTransferRead,
				.oldLayout = vk::ImageLayout::eTransferDstOptimal,
				.newLayout = vk::ImageLayout::eTransferSrcOptimal,
				.srcQueueFamilyIndex = vk::QueueFamilyIgnored,
				.dstQueueFamilyIndex = vk::QueueFamilyIgnored,
				.image = image,
				.subresourceRange = {
					.aspectMask = vk::ImageAspectFlagBits::eColor,
					.levelCount = 1,
					.layerCount = 1
				}
			};

			int32_t mipWidth = texWidth;
			int32_t mipHeight = texHeight;

			for (uint32_t i = 1; i < mipLevels; i++)
			{
				barrier.subresourceRange.baseMipLevel = i - 1;
				barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
				barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
				barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
				barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

				commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, barrier);

				vk::ImageBlit blit = {
					.srcSubresource = {
						.aspectMask = vk::ImageAspectFlagBits::eColor,
						.mipLevel = i - 1,
						.layerCount = 1
					},
					.srcOffsets = std::array<vk::Offset3D, 2>({ { }, {
							mipWidth,
							mipHeight,
							1
						}
					}),
					.dstSubresource = {
						.aspectMask = vk::ImageAspectFlagBits::eColor,
						.mipLevel = i,
						.layerCount = 1
					},
					.dstOffsets = std::array<vk::Offset3D, 2>({
						{ },
						{
							1 < mipWidth ? mipWidth / 2 : 1,
							1 < mipHeight ? mipHeight / 2 : 1,
							1
						}
					})
				};

				commandBuffer.blitImage(
					image,
					vk::ImageLayout::eTransferSrcOptimal,
					image,
					vk::ImageLayout::eTransferDstOptimal,
					blit,
					vk::Filter::eLinear
				);

				barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
				barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
				barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
				barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

				commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, barrier);

				if (1 < mipWidth)
				{
					mipWidth /= 2;
				}
				if (1 < mipHeight)
				{
					mipHeight /= 2;
				}
			}

			barrier.subresourceRange.baseMipLevel = mipLevels - 1;
			barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
			barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, barrier);
		}

		std::pair<vk::raii::Image, vk::raii::DeviceMemory> createImage(
			uint32_t width,
			uint32_t height,
			uint32_t mipLevels,
			vk::SampleCountFlagBits numSamples,
			vk::Format format,
			vk::ImageTiling tiling,
			vk::ImageUsageFlags usage,
			vk::MemoryPropertyFlags properties
		)
		{
			vk::ImageCreateInfo imageInfo {
				.imageType = vk::ImageType::e2D,
				.format = format,
				.extent = {width, height, 1},
				.mipLevels = mipLevels,
				.arrayLayers = 1,
				.samples = numSamples,
				.tiling = tiling,
				.usage = usage,
				.sharingMode = vk::SharingMode::eExclusive
			};

			vk::raii::Image image = vk::raii::Image(device->logical(), imageInfo);

			vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
			vk::MemoryAllocateInfo allocInfo = {
				.allocationSize = memRequirements.size,
				.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties)
			};
			vk::raii::DeviceMemory imageMemory = vk::raii::DeviceMemory(device->logical(), allocInfo);
			image.bindMemory(imageMemory, 0);

			return { std::move(image), std::move(imageMemory) };
		}

		vk::raii::CommandBuffer beginSingleTimeCommands()
		{
			vk::CommandBufferAllocateInfo allocInfo {
				.commandPool = device->graphics_command_pool(),
				.level = vk::CommandBufferLevel::ePrimary,
				.commandBufferCount = 1
			};
			vk::raii::CommandBuffer commandBuffer = std::move(vk::raii::CommandBuffers(device->logical(), allocInfo).front());

			vk::CommandBufferBeginInfo beginInfo { .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit };
			commandBuffer.begin(beginInfo);

			return commandBuffer; // move ommited for copy elision
		}

		void endSingleTimeCommands(vk::raii::CommandBuffer&& commandBuffer)
		{
			commandBuffer.end();

			vk::SubmitInfo submitInfo {
				.commandBufferCount = 1,
				.pCommandBuffers = &*commandBuffer
			};

			device->graphics().submit(submitInfo, nullptr);
			device->graphics().waitIdle();
		}

		void transitionImageLayout(
			vk::raii::CommandBuffer& commandBuffer,
			const vk::raii::Image& image,
			vk::ImageLayout oldLayout,
			vk::ImageLayout newLayout,
			uint32_t mipLevels
		)
		{
			vk::ImageMemoryBarrier barrier {
				.oldLayout = oldLayout,
				.newLayout = newLayout,
				.srcQueueFamilyIndex = vk::QueueFamilyIgnored,
				.dstQueueFamilyIndex = vk::QueueFamilyIgnored,
				.image = image,
				.subresourceRange = {
					.aspectMask = vk::ImageAspectFlagBits::eColor,
					.levelCount = mipLevels,
					.layerCount = 1
				}
			};

			vk::PipelineStageFlags sourceStage;
			vk::PipelineStageFlags destinationStage;

			if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
			{
				barrier.srcAccessMask = {};
				barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

				sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
				destinationStage = vk::PipelineStageFlagBits::eTransfer;
			}
			else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
			{
				barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
				barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

				sourceStage = vk::PipelineStageFlagBits::eTransfer;
				destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
			}
			else
			{
				throw std::invalid_argument("unsupported layout transition!");
			}

			commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {}, nullptr, barrier);
		}

		void copyBufferToImage(
			vk::raii::CommandBuffer& commandBuffer,
			const vk::raii::Buffer& buffer,
			vk::raii::Image& image,
			uint32_t width,
			uint32_t height
		)
		{
			vk::BufferImageCopy region {
				.bufferOffset = 0,
				.bufferRowLength = 0,
				.bufferImageHeight = 0,
				.imageSubresource = {
					.aspectMask = vk::ImageAspectFlagBits::eColor,
					.mipLevel = 0,
					.baseArrayLayer = 0,
					.layerCount = 1
				},
				.imageOffset = {0, 0, 0},
				.imageExtent = {width, height, 1}
			};

			commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
		}

		vk::raii::ImageView createImageView(vk::Image const& image, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels) const
		{
			vk::ImageViewCreateInfo viewInfo {
				.image = image,
				.viewType = vk::ImageViewType::e2D,
				.format = format,
				.components = {
					vk::ComponentSwizzle::eIdentity,
					vk::ComponentSwizzle::eIdentity,
					vk::ComponentSwizzle::eIdentity,
					vk::ComponentSwizzle::eIdentity
				},
				.subresourceRange = {
					.aspectMask = aspectFlags,
					.baseMipLevel = 0,
					.levelCount = mipLevels,
					.baseArrayLayer = 0,
					.layerCount = 1,
				}
			};

			return vk::raii::ImageView(device->logical(), viewInfo);
		}

		void createTextureSampler()
		{
			vk::PhysicalDeviceProperties properties = device->physical().getProperties();

			vk::SamplerCreateInfo samplerInfo {
				.magFilter = vk::Filter::eLinear, // when close to camera, magnify
				.minFilter = vk::Filter::eLinear, // when far away from camera, minify
				.mipmapMode = vk::SamplerMipmapMode::eLinear, // eNearest to select nearest mipmap, eLinear to interpolate between the two with the Filters
				.addressModeU = vk::SamplerAddressMode::eRepeat,
				.addressModeV = vk::SamplerAddressMode::eRepeat,
				.addressModeW = vk::SamplerAddressMode::eRepeat,
				.mipLodBias = 0.0f, // bias for mipmap selection
				.anisotropyEnable = true,
				.maxAnisotropy = properties.limits.maxSamplerAnisotropy, // PERF slow, could select a lower value
				.compareEnable = false,
				.compareOp = vk::CompareOp::eAlways,
				.minLod = 0.0f,
				.maxLod = vk::LodClampNone, // 0.0f to disable
				.borderColor = vk::BorderColor::eIntOpaqueBlack,
				.unnormalizedCoordinates = false,
			};

			textureSampler = vk::raii::Sampler(device->logical(), samplerInfo);
		}

		void loadModel()
		{
			tinygltf3::Model model;
			tinygltf3::ErrorStack errors;

			tg3_error_code err = tinygltf3::parse_file(model, errors, MODEL_PATH.c_str());

			if (err != TG3_OK)
			{
				std::string error;
				for (uint32_t i = 0; i < errors.count(); i++)
				{
					error += "[" + std::to_string((int)errors.entry(i)->severity) + "] ";
					error += errors.entry(i)->message ? errors.entry(i)->message : "(null)";
				}
				throw std::runtime_error(error);
			}

			vertices.clear();
			indices.clear();
			
			for (uint32_t mesh_idx = 0; mesh_idx < model->meshes_count; mesh_idx++)
			{
				const tg3_mesh& mesh = model->meshes[mesh_idx];
				for (uint32_t primitive_idx = 0; primitive_idx < mesh.primitives_count; primitive_idx++)
				{
					const tg3_primitive& primitive = mesh.primitives[primitive_idx];
					if (primitive.mode == TG3_MODE_TRIANGLES || primitive.mode == -1)
					{
						my_tg3_load_primitive(model.get(), primitive, vertices, indices, &textureDecoded, hasTextureDecoded);
					}
				}
			}
		}

		static bool my_tg3_load_primitive(
			const tg3_model* model,
			const tg3_primitive& primitive,
			std::vector<Vertex>& vertices,
			std::vector<uint32_t>& indices,
			DecodedImage* texture,
			bool& texture_load_success
		)
		{
			int32_t pos_accessor = my_tg3_find_attribute(primitive, "POSITION");
			if (pos_accessor < 0) return false;

			std::vector<float> pos;
			std::vector<float> uv;

			int32_t pos_num_components = 0;
			int32_t uv_num_components = 0;
			if (!my_tg3_read_accessor_floats(model, pos_accessor, pos, pos_num_components) || pos_num_components != 3) return false;

			int uv_accessor = my_tg3_find_attribute(primitive, "TEXCOORD_0");
			if (uv_accessor >= 0 && !my_tg3_read_accessor_floats(model, uv_accessor, uv, uv_num_components)) return false;
			if (uv_accessor >= 0 && uv_num_components != 2) uv.clear();

			const size_t vertex_count = pos.size() / 3;
			const uint32_t base = (uint32_t)vertices.size();
			std::unordered_map<Vertex, uint32_t> unique_vertices;
			const bool has_indices = primitive.indices != TG3_INDEX_NONE;

			for (size_t i = 0; i < vertex_count; i++)
			{
				Vertex vertex{};
				memcpy(&vertex.pos, &pos[i * 3], 3 * sizeof(float));
				if (i * 2 + 1 < uv.size())
				{
					memcpy(&vertex.uv, &uv[i * 2], 2 * sizeof(float));
				}

				// inversion from Y-up to Z-up same would have to be applied to normals
				// FIXME maybe change vulkan itself?
				// maybe check somehow what axis the model uses
				float tmp = vertex.pos[1];
				vertex.pos[1] = -vertex.pos[2];
				vertex.pos[2] = tmp;

				// vertex.texCoord[1] = 1.0f - vertex.texCoord[1]; // inverted // FIXME some textures need this, some don't

				if (!has_indices)
				{
					auto [it, inserted] = unique_vertices.insert({ vertex, static_cast<uint32_t>(vertices.size()) });
					if (inserted)
					{
						vertices.push_back(vertex);
					}

					indices.push_back(it->second);
				}
				else
				{
					vertices.push_back(vertex);
				}
			}

			if (has_indices)
			{
				std::vector<uint32_t> idx;
				if (!my_tg3_read_indices(model, primitive.indices, idx)) return false;
				for (uint32_t i : idx)
				{
					indices.push_back(base + i);
				}
			}

			const tg3_image* image = my_tg3_get_texture_image(*model, primitive);
			if (image != nullptr)
			{
				*texture = my_tg3_decode_image(*model, *image, "models");
			}

			texture_load_success = (image != nullptr) && (texture->pixels.get() != nullptr);

			std::cout << "loaded model\n";
			std::cout << "\tvertices: " << std::to_string(indices.size()) << "\n";
			std::cout << "\tindices:  " << std::to_string(vertices.size()) << "\n";

			return true;
		}

		static const tg3_image* my_tg3_get_texture_image(
			const tg3_model& model,
			const tg3_primitive& primitive
		)
		{
			if (primitive.material < 0) return nullptr;

			const tg3_material* material = &model.materials[primitive.material];
			int32_t texture_index = material->pbr_metallic_roughness.base_color_texture.index;
			if (texture_index < 0) return nullptr;

			const tg3_texture* texture = &model.textures[texture_index];
			int32_t image_index = texture->source;
			if (image_index < 0) return nullptr;

			return &model.images[image_index];
		}

		static DecodedImage my_tg3_decode_image(const tg3_model& model, const tg3_image& image, const std::string& base_dir)
		{
			DecodedImage result {
				.pixels = nullptr,
				.width = -1,
				.height = -1,
				.channels = -1,
				.bytes = 0
			};

			const uint8_t* encoding = nullptr;
			size_t encoding_size = 0;
			std::vector<uint8_t> file_bytes;

			if (image.buffer_view >= 0)
			{
				const tg3_buffer_view& buffer_view = model.buffer_views[image.buffer_view];
				const tg3_buffer& buffer = model.buffers[buffer_view.buffer];
				encoding = buffer.data.data + buffer_view.byte_offset;
				encoding_size = (size_t)buffer_view.byte_length;
			}
			else if (image.image.count > 0)
			{
				encoding = image.image.data;
				encoding_size = (size_t)image.image.count;
			}
			else if (image.uri.len > 0 && !tg3_is_data_uri(image.uri.data, image.uri.len))
			{
				std::string path = base_dir + "/" + std::string(image.uri.data, image.uri.len);
				FILE* file = fopen(path.c_str(), "rb");
				if (!file) return result;
				fseek(file, 0, SEEK_END);
				long file_size = ftell(file);
				fseek(file, 0, SEEK_SET);
				file_bytes.resize((size_t)file_size);
				fread(file_bytes.data(), 1, (size_t)file_size, file);
				fclose(file);
				encoding = file_bytes.data();
				encoding_size = file_bytes.size();
			}

			if (!encoding) return result;

			// FIXME is num_components really the same as num_channels
			uint8_t* pixels = stbi_load_from_memory(encoding, (int)encoding_size, &result.width, &result.height, &result.channels, STBI_rgb_alpha);

			// result.bytes = result.width * result.height * result.channels; // channels should be 4 // TODO put this in struct method
			result.bytes = result.width * result.height * 4; // channels should be 4 // TODO put this in struct method
			result.pixels = std::make_unique<uint8_t[]>(result.bytes);
			memcpy(result.pixels.get(), pixels, result.bytes);

			stbi_image_free(pixels);

			return result;
		}

		static int32_t my_tg3_find_attribute(const tg3_primitive& primitive, const char* name)
		{
			for (uint32_t i = 0; i < primitive.attributes_count; i++)
			{
				if (tg3_str_equals_cstr(primitive.attributes[i].key, name))
				{
					return primitive.attributes[i].value;
				}
			}

			return -1;
		}

		static bool my_tg3_read_accessor_floats(
			const tg3_model* model,
			int32_t accessor_idx,
			std::vector<float>& out,
			int& num_components
		)
		{
			if (accessor_idx < 0 || (uint32_t)accessor_idx >= model->accessors_count) return false;
			const tg3_accessor& accessor = model->accessors[accessor_idx];

			num_components = tg3_num_components(accessor.type);
			int32_t component_size = tg3_component_size(accessor.component_type);
			if (num_components <= 0 || component_size <= 0) return false;

			out.assign((size_t)accessor.count * component_size, 0.0f);
			const bool is_accessor_normalized = accessor.normalized != 0;

			if (accessor.buffer_view >= 0)
			{
				if ((uint32_t)accessor.buffer_view >= model->buffer_views_count) return false;
				const tg3_buffer_view& buffer_view = model->buffer_views[accessor.buffer_view];
				int32_t stride = tg3_accessor_byte_stride(&accessor, &buffer_view);
				if (stride <= 0) return false;

				const uint8_t* src = my_tg3_view_data(
					model,
					accessor.buffer_view,
					accessor.byte_offset,
					accessor.count,
					stride,
					(uint64_t)num_components * component_size
				);
				if (!src && accessor.count) return false;

				for (uint64_t i = 0; i < accessor.count; i++)
				{
					for (int c = 0; c < num_components; c++)
					{
						out[i * num_components + c] = my_tg3_read_component(
							src + i * stride + c * component_size,
							accessor.component_type, is_accessor_normalized
						);
					}
				}
			}

			if (accessor.sparse.is_sparse && accessor.sparse.count > 0)
			{
				const tg3_accessor_sparse& accessor_sparse = accessor.sparse;
				int32_t sparse_component_size = tg3_component_size(accessor_sparse.indices.component_type);
				if (sparse_component_size > 0)
				{
					const uint8_t* index_ptr = my_tg3_view_data(
						model,
						accessor_sparse.indices.buffer_view,
						accessor_sparse.indices.byte_offset,
						accessor_sparse.count,
						sparse_component_size,
						sparse_component_size
					);

					const uint8_t* vertex_ptr = my_tg3_view_data(
						model,
						accessor_sparse.values.buffer_view,
						accessor_sparse.values.byte_offset,
						accessor_sparse.count,
						(uint64_t)num_components * component_size,
						(uint64_t)num_components * component_size
					);

					if (index_ptr && vertex_ptr)
					{
						for (int32_t k = 0; k < accessor_sparse.count; k++)
						{
							uint32_t idx = my_tg3_read_index_value(
								index_ptr + (size_t)k * sparse_component_size,
								accessor_sparse.indices.component_type
							);
							if (idx >= accessor.count) continue;
							for (int32_t c = 0; c < num_components; c++)
							{
								out[(size_t)idx * num_components + c] = my_tg3_read_component(
									vertex_ptr + ((size_t)k * num_components + c) * component_size,
									accessor.component_type,
									is_accessor_normalized
								);
							}
						}
					}
				}
			}

			return true;
		}

		static const uint8_t* my_tg3_view_data(
			const tg3_model* model,
			int32_t buffer_view_idx,
			uint64_t extra_offset,
			uint64_t count,
			uint64_t stride,
			uint64_t element_size
		)
		{
			if (buffer_view_idx < 0 || (uint32_t)buffer_view_idx >= model->buffer_views_count) return nullptr;

			const tg3_buffer_view& buffer_view = model->buffer_views[buffer_view_idx];
			if (buffer_view.buffer < 0 || (uint32_t)buffer_view.buffer >= model->buffers_count) return nullptr;

			const tg3_buffer& buffer = model->buffers[buffer_view.buffer];
			if (!buffer.data.data) return nullptr;
			if (count == 0) return buffer.data.data;

			uint64_t start = buffer_view.byte_offset + extra_offset;
			uint64_t end = start + (count - 1) * stride + element_size;
			if (end > buffer.data.count) return nullptr;

			return buffer.data.data + start;
		}

		static float my_tg3_read_component(const uint8_t* ptr, int32_t component_type, bool is_normalized)
		{
			switch (component_type)
			{
				case TG3_COMPONENT_TYPE_FLOAT:
					{
						float v;
						memcpy(&v, ptr, 4);
						return v;
					}
				case TG3_COMPONENT_TYPE_DOUBLE:
					{
						double v;
						memcpy(&v, ptr, 8);
						return (float)v;
					}
				case TG3_COMPONENT_TYPE_UNSIGNED_BYTE:
					{
						return is_normalized ? *ptr / 255.0f : (float)*ptr;
					}
				case TG3_COMPONENT_TYPE_BYTE:
					{
						int8_t v = (int8_t)*ptr;
						return is_normalized ? std::max(v / 127.0f, -1.0f) : (float)v;
					}
				case TG3_COMPONENT_TYPE_UNSIGNED_SHORT:
					{
						uint16_t v;
						memcpy(&v, ptr, 2);
						return is_normalized ? v / 65535.0f : (float)v;
					}
				case TG3_COMPONENT_TYPE_SHORT:
					{
						int16_t v;
						memcpy(&v, ptr, 2);
						return is_normalized ? std::max(v / 32767.0f, -1.0f) : (float)v;
					}
				case TG3_COMPONENT_TYPE_UNSIGNED_INT:
					{
						uint32_t v;
						memcpy(&v, ptr, 4);
						return (float)v;
					}
				case TG3_COMPONENT_TYPE_INT:
					{
						int32_t v;
						memcpy(&v, ptr, 4);
						return (float)v;
					}
			}

			return 0.0f;
		}

		static uint32_t my_tg3_read_index_value(const uint8_t* ptr, int32_t component_type)
		{
			switch (component_type)
			{
				case TG3_COMPONENT_TYPE_UNSIGNED_BYTE:
					{
						return *ptr;
					}
				case TG3_COMPONENT_TYPE_UNSIGNED_SHORT:
					{
						uint16_t v;
						memcpy(&v, ptr, 2);
						return v;
					}
				case TG3_COMPONENT_TYPE_UNSIGNED_INT:
					{
						uint32_t v;
						memcpy(&v, ptr, 4);
						return v;
					}
			}

			return 0;
		}

		static bool my_tg3_read_indices(
			const tg3_model* model,
			int32_t accessor_idx,
			std::vector<uint32_t>& out
		)
		{
			if (accessor_idx < 0 || (uint32_t)accessor_idx >= model->accessors_count) return false;

			const tg3_accessor& accessor = model->accessors[accessor_idx];
			if (accessor.type != TG3_TYPE_SCALAR) return false;

			int32_t component_size = tg3_component_size(accessor.component_type);
			if (component_size <= 0 || accessor.buffer_view < 0 ||
				(uint32_t)accessor.buffer_view >= model->buffer_views_count) return false;

			const tg3_buffer_view& buffer_view = model->buffer_views[accessor.buffer_view];
			const int32_t stride = tg3_accessor_byte_stride(&accessor, &buffer_view);
			if (stride <= 0) return false;

			const uint8_t* src = my_tg3_view_data(
				model,
				accessor.buffer_view,
				accessor.byte_offset,
				accessor.count,
				stride,
				component_size
			);
			if (!src && accessor.count) return false;

			out.resize((size_t)accessor.count);
			for (uint64_t i = 0; i < accessor.count; i++)
			{
				out[i] = my_tg3_read_index_value(src + i * stride, accessor.component_type);
			}

			return true;
		}

		void setupGameObjects()
		{
			gameObjects[0].position = {0.0f, 0.0f, 0.0f};
			gameObjects[0].rotation = {0.0f, 0.0f, 0.0f};
			gameObjects[0].scale = {1.0f, 1.0f, 1.0f};

			gameObjects[1].position = {-2.0f, 0.0f, -1.0f};
			gameObjects[1].rotation = {0.0f, glm::radians(45.0f), 0.0f};
			gameObjects[1].scale = {0.75f, 0.75f, 0.75f};

			gameObjects[2].position = {2.0f, 0.0f, -1.0f};
			gameObjects[2].rotation = {0.0f, glm::radians(-45.0f), 0.0f};
			gameObjects[2].scale = {0.75f, 0.75f, 0.75f};
		}

		void createCommandBuffers()
		{
			vk::CommandBufferAllocateInfo allocInfo {
				.commandPool = device->graphics_command_pool(),
				.level = vk::CommandBufferLevel::ePrimary,
				.commandBufferCount = MAX_FRAMES_IN_FLIGHT
			};
			commandBuffers = vk::raii::CommandBuffers(device->logical(), allocInfo);
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

			vk::raii::CommandBuffer commandCopyBuffer = beginSingleTimeCommands();
			commandCopyBuffer.copyBuffer(*srcBuffer, *dstBuffer, vk::BufferCopy(0, 0, size));
			endSingleTimeCommands(std::move(commandCopyBuffer));
			// we can use vk::raii::waitForFences instead if we want to schedule multiple transfers simultaneously
		}

		std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
		{
			vk::BufferCreateInfo bufferInfo {
				.size = size,
				.usage = usage,
				.sharingMode = vk::SharingMode::eExclusive
			};

			vk::raii::Buffer buffer = vk::raii::Buffer(device->logical(), bufferInfo);
			vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();

			vk::MemoryAllocateInfo allocInfo = {
				.allocationSize = memRequirements.size,
				.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties)
			};

			vk::raii::DeviceMemory bufferMemory = vk::raii::DeviceMemory(device->logical(), allocInfo);
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
			for (auto& gameObject : gameObjects)
			{
				gameObject.uniformBuffers.clear();
				gameObject.uniformBuffersMemory.clear();
				gameObject.uniformBuffersMapped.clear();

				for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
				{
					vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
					auto [buffer, bufferMem] = createBuffer(
						bufferSize,
						vk::BufferUsageFlagBits::eUniformBuffer,
						vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
					);
					gameObject.uniformBuffers.emplace_back(std::move(buffer));
					gameObject.uniformBuffersMemory.emplace_back(std::move(bufferMem));
					gameObject.uniformBuffersMapped.emplace_back(gameObject.uniformBuffersMemory.back().mapMemory(0, bufferSize));
				}
			}
		}

		void createDescriptorPool()
		{
			std::array<vk::DescriptorPoolSize, 2> poolSize {{
				{
					.type = vk::DescriptorType::eUniformBuffer,
					.descriptorCount = MAX_OBJECTS * MAX_FRAMES_IN_FLIGHT
				},
				{
					.type = vk::DescriptorType::eCombinedImageSampler,
					.descriptorCount = MAX_OBJECTS * MAX_FRAMES_IN_FLIGHT
				}
			}};

			vk::DescriptorPoolCreateInfo poolInfo {
				.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
				.maxSets = MAX_OBJECTS * MAX_FRAMES_IN_FLIGHT,
				.poolSizeCount = static_cast<uint32_t>(poolSize.size()),
				.pPoolSizes = poolSize.data()
			};

			descriptorPool = vk::raii::DescriptorPool(device->logical(), poolInfo);
		}

		void createDescriptorSets()
		{
			for (auto& gameObject : gameObjects)
			{
				std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *pipeline->descriptor_set_layout());

				vk::DescriptorSetAllocateInfo allocInfo {
					.descriptorPool = descriptorPool,
					.descriptorSetCount = static_cast<uint32_t>(layouts.size()),
					.pSetLayouts = layouts.data()
				};

				gameObject.descriptorSets.clear();
				gameObject.descriptorSets = device->logical().allocateDescriptorSets(allocInfo);

				for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
				{
					vk::DescriptorBufferInfo bufferInfo {
						.buffer = gameObject.uniformBuffers[i],
						.offset = 0,
						.range = sizeof(UniformBufferObject)
					};

					vk::DescriptorImageInfo imageInfo {
						.sampler = textureSampler,
						.imageView = textureImageView,
						.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
					};

					std::array<vk::WriteDescriptorSet, 2> descriptorWrites {{
						{
							.dstSet = gameObject.descriptorSets[i],
							.dstBinding = 0,
							.dstArrayElement = 0,
							.descriptorCount = 1,
							.descriptorType = vk::DescriptorType::eUniformBuffer,
							.pBufferInfo = &bufferInfo
						},
						{
							.dstSet = gameObject.descriptorSets[i],
							.dstBinding = 1,
							.dstArrayElement = 0,
							.descriptorCount = 1,
							.descriptorType = vk::DescriptorType::eCombinedImageSampler,
							.pImageInfo = &imageInfo
						}
					}};

					device->logical().updateDescriptorSets(descriptorWrites, {});
				}
			}
		}

		uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
		{
			// selecting a specific heap "memoryHeaps" may increase performance
			vk::PhysicalDeviceMemoryProperties memProperties = device->physical().getMemoryProperties();

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
				*swapchain->color().image(),
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
				*swapchain->depth().image(),
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
				.imageView = swapchain->color().image_view(),
				.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
				.resolveMode = vk::ResolveModeFlagBits::eAverage,
				.resolveImageView = swapchain->image_views()[imageIndex],
				.resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal,
				.loadOp = vk::AttachmentLoadOp::eClear,
				.storeOp = vk::AttachmentStoreOp::eStore,
				.clearValue = clearColor
			};

			vk::RenderingAttachmentInfo depthAttachmentInfo = {
				.imageView = swapchain->depth().image_view(),
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

			// TODO needs synchronization here?
			commandBuffer.beginRendering(renderingInfo);
			commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline->pipeline());

			commandBuffer.bindVertexBuffers(0, *vertexBuffer, {0});
			commandBuffer.bindIndexBuffer(*indexBuffer, 0, vk::IndexTypeValue<decltype(indices)::value_type>::value);

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

				commandBuffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
			}

			commandBuffer.endRendering();
			// TODO needs synchronization here?

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
				renderFinishedSemaphores.emplace_back(device->logical(), vk::SemaphoreCreateInfo());
			}

			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			{
				presentCompleteSemaphores.emplace_back(device->logical(), vk::SemaphoreCreateInfo());
				inFlightFences.emplace_back(device->logical(), vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
			}
		}

		void mainLoop()
		{
			while (!glfwWindowShouldClose(window.get_handle()))
			{
				glfwPollEvents();
				drawFrame();
			}

			device->logical().waitIdle();
		}

		void drawFrame()
		{
			auto fenceResult = device->logical().waitForFences(*inFlightFences[frameIndex], true, std::numeric_limits<uint64_t>::max());
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

			device->logical().resetFences(*inFlightFences[frameIndex]);

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

			device->graphics().submit(submitInfo, *inFlightFences[frameIndex]); // TODO error handling

			const vk::PresentInfoKHR presentInfoKHR {
				.waitSemaphoreCount = 1,
				.pWaitSemaphores = &*renderFinishedSemaphores[imageIndex],
				.swapchainCount = 1,
				.pSwapchains = &*swapchain->swapchain(),
				.pImageIndices = &imageIndex,
				.pResults = nullptr
			};

			result = device->graphics().presentKHR(presentInfoKHR);
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

				memcpy(gameObject.uniformBuffersMapped[frameIndex], &ubo, sizeof(ubo));
			}
		}

		void recreateSwapChain()
		{
			window.block_while_minimized();

			// slow, instead of waiting, just pass the old swapchain as oldSwapchain in the new swapchain
			// and destroy the old swapchain as soon as we're finished using it
			device->logical().waitIdle();

			swapchain->recreate(*device, window);
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
			if (!device->is_valid()) { throw std::runtime_error("failed to create device!"); }

			swapchain = std::make_unique<Swapchain>(*device, window);
			if (!swapchain->is_valid()) { throw std::runtime_error("failed to create swapchain!"); }

			pipeline = std::make_unique<Pipeline>(*resource_manager, *device, *swapchain);
			if (!pipeline->is_valid()) { throw std::runtime_error("failed to create pipeline!"); }

			loadModel();
			setupGameObjects();
			createTextureImage();
			createTextureSampler();
			createVertexBuffer();
			createIndexBuffer();
			createUniformBuffers();
			createDescriptorPool();
			createDescriptorSets();
			createCommandBuffers();
			createSyncObjects();

			// FIXME make exports unified in a single file
		}

		~VulkanTutorial()
		{
			for (auto& gameObject : gameObjects)
			{
				for (size_t i = 0; i < gameObject.uniformBuffersMemory.size(); i++)
				{
					if (gameObject.uniformBuffersMapped[i] != nullptr)
					{
						gameObject.uniformBuffersMemory[i].unmapMemory();
					}
				}

				gameObject.uniformBuffers.clear();
				gameObject.uniformBuffersMemory.clear();
				gameObject.uniformBuffersMapped.clear();
				gameObject.descriptorSets.clear();
			}

			surface = window.take_surface(); // FIXME

			// window.destroy_surface();
		}

		void run()
		{
			mainLoop();
		}
	};
}
