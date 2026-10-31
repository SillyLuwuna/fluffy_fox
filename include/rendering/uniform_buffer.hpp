#pragma once

#include "rendering/gpu_buffer.hpp"
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include <glm/glm.hpp>
#include "rendering/gpu.hpp"

#include <fluff/types.hpp>
using namespace fluff::types;

namespace fluffy_fox::rendering
{
	template <typename T>
	class UniformBuffer
	{
	private:
		GpuBuffer buffer_;
		T* mapped_memory_;

	public:
		UniformBuffer(const UniformBuffer&) = delete;
		UniformBuffer& operator=(const UniformBuffer&) = delete;

		UniformBuffer() :
			buffer_(Gpu::create_buffer(
				sizeof(T),
				vk::BufferUsageFlagBits::eUniformBuffer,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
			)),
			mapped_memory_(static_cast<T*>(
				buffer_.memory.mapMemory(0, sizeof(T)))
			)
		{ }

		~UniformBuffer()
		{
			if (!buffer_) return;
			buffer_.memory.unmapMemory();
		}

		UniformBuffer(UniformBuffer&& other) :
			buffer_(std::move(other.buffer_)),
			mapped_memory_(other.mapped_memory_)
		{
			other.mapped_memory_ = nullptr;
		}

		UniformBuffer& operator=(UniformBuffer&& other)
		{
			buffer_ = std::move(other.buffer_);
			mapped_memory_ = other.mapped_memory_;
			other.mapped_memory_ = nullptr;
		}

		inline void set(const T& obj)
		{
			memcpy(mapped_memory_, &obj, sizeof(T));
		}

		vk::DescriptorBufferInfo descriptor_info() const
		{
			return {
				.buffer = buffer_.buffer,
				.offset = 0,
				.range = sizeof(T)
			};
		}

		constexpr vk::DescriptorType descriptor_type() const
		{
			return vk::DescriptorType::eUniformBuffer;
		}
	};
}
