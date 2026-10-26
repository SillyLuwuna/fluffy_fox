#pragma once

#include "rendering/device.hpp"
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include <fluff/types.hpp>
using namespace fluff::types;


namespace fluffy_fox::rendering::vulkan_image
{
	vk::raii::ImageView create_image_view(
		Device& device,
		const vk::Image& image,
		vk::Format format,
		vk::ImageAspectFlags aspect_flags,
		u32 mip_levels
	);
}
