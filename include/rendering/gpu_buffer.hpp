#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include <fluff/types.hpp>
using namespace fluff::types;

namespace fluffy_fox::rendering
{
	struct GpuBuffer
	{
		vk::raii::Buffer buffer;
		vk::raii::DeviceMemory memory;

		static GpuBuffer empty()
		{
			return {
				.buffer = nullptr,
				.memory = nullptr
			};
		}

		inline operator bool() const
		{
			return buffer != nullptr;
		}

		// inline Buffer& operator=(const Buffer& other) = delete;
		// inline Buffer(const Buffer& other) = delete;
		//
		// inline Buffer() :
		// 	buffer(nullptr),
		// 	memory(nullptr)
		// { }
		//
		// inline explicit Buffer(std::nullptr_t) :
		// 	buffer(nullptr),
		// 	memory(nullptr)
		// { }
		//
		// inline Buffer(vk::raii::Buffer&& buffer, vk::raii::DeviceMemory&& memory) :
		// 	buffer(std::move(buffer)),
		// 	memory(std::move(memory))
		// { }
		//
		// inline Buffer(Buffer&& other) :
		// 	buffer(std::move(other.buffer)),
		// 	memory(std::move(other.memory))
		// { }

		inline GpuBuffer& operator=(std::nullptr_t)
		{
			buffer = nullptr;
			memory = nullptr;
			return *this;
		}

		// inline Buffer& operator=(Buffer&& other)
		// {
		// 	buffer = std::move(other.buffer);
		// 	memory = std::move(other.memory);
		// 	return *this;
		// }
		//
	};
}
