#pragma once

#include "rendering/buffer.hpp"
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include <fluff/types.hpp>
using namespace fluff::types;

namespace fluffy_fox::rendering
{
	class Gpu
	{
	private:

	public:
		Gpu();

		vk::raii::DeviceMemory alloc(
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

		template <typename T>
		Buffer upload(
			const std::vector<T> arr,
			vk::raii::DeviceMemory& memory
			// Buffer& dst_buffer // FIXME circular dependency
		)
		{
			vk::DeviceSize buffer_size = sizeof(T) * arr.size();

			Buffer staging_buffer(); // FIXME make a Buffer class
		}
	};
}
