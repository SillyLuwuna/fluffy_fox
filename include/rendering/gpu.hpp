#pragma once

#include "rendering/gpu_buffer.hpp"
#include "rendering/gpu_image.hpp"
#include "rendering/device.hpp"
#include "rendering/image.hpp"
#include "rendering/single_time_command.hpp"
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include <fluff/types.hpp>
using namespace fluff::types;

namespace fluffy_fox::rendering
{
	class Gpu
	{
		// TODO
		// - make upload function be asynchronous
		// - use VMA instead of directly allocating
	private:

	public:

		static vk::raii::DeviceMemory alloc(
			vk::MemoryRequirements requirements,
			vk::MemoryPropertyFlags properties
		)
		{
			u32 mem_type = Device::instance().find_memory_type(
				requirements.memoryTypeBits,
				properties
			);
			if (mem_type == ~0u) return nullptr;

			vk::MemoryAllocateInfo alloc_info {
				.allocationSize = requirements.size,
				.memoryTypeIndex = mem_type
			};

			return vk::raii::DeviceMemory(
				Device::instance().logical(),
				alloc_info
			);
		}

		static GpuBuffer create_buffer(
			vk::DeviceSize size,
			vk::BufferUsageFlags usage,
			vk::MemoryPropertyFlags properties
		)
		{
			vk::BufferCreateInfo buffer_info {
				.size = size,
				.usage = usage,
				.sharingMode = vk::SharingMode::eExclusive
			};
			vk::raii::Buffer buffer(Device::instance().logical(), buffer_info);

			vk::MemoryRequirements mem_requirements = buffer.getMemoryRequirements();
			vk::raii::DeviceMemory device_memory = alloc(mem_requirements, properties);

			return {
				.buffer = std::move(buffer),
				.memory = std::move(device_memory)
			};
		}

		static GpuImage create_image(
			u32 width,
			u32 height,
			u32 mip_levels,
			vk::SampleCountFlagBits num_samples,
			vk::Format format,
			vk::ImageTiling tiling,
			vk::ImageUsageFlags usage,
			vk::ImageAspectFlags aspect,
			vk::MemoryPropertyFlags mem_properties
		)
		{
			vk::ImageCreateInfo image_info {
				.imageType = vk::ImageType::e2D,
				.format = format,
				.extent = {width, height, 1},
				.mipLevels = mip_levels,
				.arrayLayers = 1,
				.samples = num_samples,
				.tiling = tiling,
				.usage = usage,
				.sharingMode = vk::SharingMode::eExclusive
			};

			vk::raii::Image image(Device::instance().logical(), image_info);

			vk::MemoryRequirements mem_requirements = image.getMemoryRequirements();
			vk::raii::DeviceMemory memory = alloc(mem_requirements, mem_properties);
			image.bindMemory(memory, 0);

			vk::ImageViewCreateInfo view_info {
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
					.aspectMask = aspect,
					.baseMipLevel = 0,
					.levelCount = mip_levels,
					.baseArrayLayer = 0,
					.layerCount = 1,
				}
			};

			vk::raii::ImageView image_view(Device::instance().logical(), view_info);

			return {
				.image = std::move(image),
				.memory = std::move(memory),
				.view = std::move(image_view),
				.format = format,
				.width = width,
				.height = height,
				.mip_levels = mip_levels
			};
		}

		template <typename T>
		static GpuBuffer upload(const std::vector<T> arr, vk::BufferUsageFlags usage)
		{
			vk::DeviceSize buffer_size = sizeof(T) * arr.size();

			GpuBuffer dst_buffer = create_buffer(
				buffer_size,
				usage | vk::BufferUsageFlagBits::eTransferDst,
				vk::MemoryPropertyFlagBits::eDeviceLocal
			);

			GpuBuffer staging_buffer = create_buffer(
				buffer_size,
				vk::BufferUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
			);

			void* data_staging = staging_buffer.memory.mapMemory(0, buffer_size);
			memcpy(data_staging, arr.data(), buffer_size);
			staging_buffer.memory.unmapMemory();
			// eHostCoherent guarantees after this point that the data is actually there.
			// Could also call a function to flush the memory
			// PERF however eHostCoherent is less performant than explicit flushing
			// this will be uploaded to the gpu asynchronously, and will be complete as of
			// the next call to vk::raii::Queue::submit

			SingleTimeCommand single_time_command;
			single_time_command.copy_buffer(dst_buffer, staging_buffer, buffer_size);

			return dst_buffer;
		}

		static GpuImage upload_image(const Image& image, u32 mip_levels = 1)
		{
			GpuBuffer staging = create_buffer(
				image.bytes,
				vk::BufferUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
			);

			GpuImage gpu_image = create_image(
				image.width,
				image.height,
				mip_levels,
				vk::SampleCountFlagBits::e1,
				vk::Format::eR8G8B8A8Srgb,
				vk::ImageTiling::eOptimal,
				vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
				vk::ImageAspectFlagBits::eColor,
				vk::MemoryPropertyFlagBits::eDeviceLocal
			);

			SingleTimeCommand command;
			command.transition_image_layout(
				gpu_image,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eTransferDstOptimal
			);
			command.copy_buffer_to_image(staging, gpu_image);
			command.generate_mipmaps(gpu_image);

			return gpu_image;
		}
	};
}
