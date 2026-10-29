#pragma once

#include "rendering/device.hpp"
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include <fluff/types.hpp>
using namespace fluff::types;


namespace fluffy_fox::rendering::vulkan_image
{
	// FIXME put in the actual Image class
	vk::raii::ImageView create_image_view(
		const vk::Image& image,
		vk::Format format,
		vk::ImageAspectFlags aspect_flags,
		u32 mip_levels
	);
}
