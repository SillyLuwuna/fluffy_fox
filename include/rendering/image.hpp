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
			u32 width,
			u32 height,
			u32 mip_levels,
			vk::SampleCountFlagBits num_samples,
			vk::Format format,
			vk::ImageTiling tiling,
			vk::ImageUsageFlags usage
		);

		vk::raii::DeviceMemory create_image_memory(
			vk::MemoryPropertyFlags properties
		);

		vk::raii::ImageView create_view(
			vk::Format format,
			vk::ImageAspectFlags aspect,
			u32 mip_levels
		);

	public:
		Image(
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
