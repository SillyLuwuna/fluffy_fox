#include "rendering/vulkan_image.hpp"

using namespace fluffy_fox::rendering;

vk::raii::ImageView vulkan_image::create_image_view(
	const vk::Image& image,
	vk::Format format,
	vk::ImageAspectFlags aspect_flags,
	u32 mip_levels
)
{
	vk::ImageViewCreateInfo viewInfo {
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
			.aspectMask = aspect_flags,
			.baseMipLevel = 0,
			.levelCount = mip_levels,
			.baseArrayLayer = 0,
			.layerCount = 1,
		}
	};

	return vk::raii::ImageView(Device::instance().logical(), viewInfo);
}
