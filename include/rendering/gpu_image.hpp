#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include <fluff/types.hpp>
using namespace fluff::types;


namespace fluffy_fox::rendering
{
	struct GpuImage
	{
		vk::raii::Image image;
		vk::raii::DeviceMemory memory;
		vk::raii::ImageView view;
		vk::Format format;
		u32 width;
		u32 height;
		u32 mip_levels;

		static GpuImage empty()
		{
			return {
				.image = nullptr,
				.memory = nullptr,
				.view = nullptr,
				.format = vk::Format::eUndefined,
				.width = 0,
				.height = 0,
				.mip_levels = 0
			};
		}

		inline operator bool() const
		{
			return image != nullptr;
		}

		inline GpuImage& operator=(std::nullptr_t)
		{
			image = nullptr;
			memory = nullptr;
			view = nullptr;
			return *this;
		}
	};
}
