#pragma once

#include "rendering/device.hpp"
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include <fluff/types.hpp>
using namespace fluff::types;


namespace fluffy_fox::rendering
{
	class Image
	{
	private:
		vk::raii::Image image_;
		vk::raii::DeviceMemory memory_;
		vk::raii::ImageView view_;

		vk::raii::Image create_image(
			Device& device,
			u32 width,
			u32 height,
			u32 mip_levels,
			vk::SampleCountFlagBits num_samples,
			vk::Format format,
			vk::ImageTiling tiling,
			vk::ImageUsageFlags usage
		);

		vk::raii::DeviceMemory create_image_memory(
			Device& device,
			vk::MemoryPropertyFlags properties
		);

		vk::raii::ImageView create_view(
			Device& device,
			vk::Format format,
			vk::ImageAspectFlags aspect,
			u32 mip_levels
		);

		u32 find_memory_type(Device& device, u32 type_filter, vk::MemoryPropertyFlags properties);

	public:
		Image(
			Device& device,
			u32 width,
			u32 height,
			u32 mip_levels,
			vk::SampleCountFlagBits num_samples,
			vk::Format format,
			vk::ImageTiling tiling,
			vk::ImageUsageFlags usage,
			vk::MemoryPropertyFlags properties,
			vk::ImageAspectFlags aspect
		);

		bool is_valid() const;

		inline vk::raii::Image& image() { return image_; }
		inline vk::raii::ImageView& image_view() { return view_; }
	};
}
