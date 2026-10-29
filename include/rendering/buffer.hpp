#pragma once

#include "rendering/single_time_command.hpp"

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include <fluff/types.hpp>
using namespace fluff::types;

namespace fluffy_fox::rendering
{
	// TODO
	// - make an "upload" function
	// - make upload function be asynchronous
	// - use VMA instead of directly allocating
	class Buffer
	{
	private:
		vk::raii::Buffer buffer_;
		vk::raii::DeviceMemory memory_;

		static vk::raii::Buffer create_buffer(
			vk::DeviceSize buffer_size,
			vk::BufferUsageFlags usage
		)
		{
			vk::BufferCreateInfo buffer_info {
				.size = buffer_size,
				.usage = usage,
				.sharingMode = vk::SharingMode::eExclusive
			};

			return vk::raii::Buffer(Device::instance().logical(), buffer_info);
		}

		static vk::raii::DeviceMemory create_memory(
			vk::raii::Buffer& buffer,
			vk::MemoryPropertyFlags properties
		)
		{
			vk::MemoryRequirements mem_requirements = buffer.getMemoryRequirements();

			u32 mem_type = Device::instance().find_memory_type(mem_requirements.memoryTypeBits, properties);
			if (mem_type == ~0u) return nullptr;

			vk::MemoryAllocateInfo alloc_info = {
				.allocationSize = mem_requirements.size,
				.memoryTypeIndex = mem_type
			};

			vk::raii::DeviceMemory buffer_memory = vk::raii::DeviceMemory(
				Device::instance().logical(),
				alloc_info
			);

			buffer.bindMemory(*buffer_memory, 0); // second parameter is the offset within the region of memory
			return buffer_memory;
		}

		void copy_buffer_command(
			vk::raii::Buffer& src_buffer,
			vk::raii::Buffer& dst_buffer,
			vk::DeviceSize size
		)
		{
			// PERF use a separate command pool for these short-lived buffers, for optimizations.
			// vk::CommandPoolCreateFlagBits::eTransient would be needed in command pool generation, in that case.

			SingleTimeCommand single_time_command;
			single_time_command.command_buffer().copyBuffer(
				*src_buffer,
				*dst_buffer,
				vk::BufferCopy(0, 0, size)
			);

			// PERF we can use vk::raii::waitForFences instead if we want to schedule multiple transfers simultaneously
		}

	public:
		Buffer(const Buffer&) = delete;
		Buffer& operator=(const Buffer&) = delete;

		explicit Buffer(std::nullptr_t null) :
			buffer_(nullptr),
			memory_(nullptr)
		{ }

		template <typename T>
		Buffer(
			const std::vector<T> arr,
			vk::BufferUsageFlags usage,
			vk::MemoryPropertyFlags properties
		) :
			buffer_(create_buffer(arr, usage | vk::BufferUsageFlagBits::eTransferDst)),
			memory_(create_memory(buffer_, properties))
		{
			vk::DeviceSize buffer_size = sizeof(T) * arr.size();

			vk::raii::Buffer staging_buffer = create_buffer(
				buffer_size,
				vk::BufferUsageFlagBits::eTransferSrc
			);

			vk::raii::DeviceMemory staging_buffer_memory = create_memory(
				staging_buffer,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
			);

			void* data_staging = staging_buffer_memory.mapMemory(0, buffer_size);
			memcpy(data_staging, arr.data(), buffer_size);
			staging_buffer_memory.unmapMemory();
			// eHostCoherent guarantees after this point that the data is actually there.
			// Could also call a function to flush the memory
			// PERF however eHostCoherent is less performant than explicit flushing
			// this will be uploaded to the gpu asynchronously, and will be complete as of
			// the next call to vk::raii::Queue::submit

			copy_buffer_command(staging_buffer, buffer_, buffer_size);
		}

		Buffer(Buffer&& other) :
			buffer_(std::move(other.buffer_)),
			memory_(std::move(other.memory_))
		{ }

		Buffer& operator=(Buffer&& other)
		{
			buffer_ = std::move(other.buffer_);
			memory_ = std::move(other.memory_);
			return *this;
		}

		Buffer& operator=(std::nullptr_t null)
		{
			buffer_ = nullptr;
			memory_ = nullptr;
			return *this;
		}

		inline const vk::raii::Buffer& buffer() const { return buffer_; }
	};
}
