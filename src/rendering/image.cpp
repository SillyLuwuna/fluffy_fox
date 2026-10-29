#include "rendering/image.hpp"

using namespace fluffy_fox::rendering;

Image::Image(
	u32 width,
	u32 height,
	u32 mip_levels,
	vk::SampleCountFlagBits num_samples,
	vk::Format format,
	vk::ImageTiling tiling,
	vk::ImageUsageFlags usage,
	vk::MemoryPropertyFlags properties,
	vk::ImageAspectFlags aspect
) :
	image_(create_image(width, height, mip_levels, num_samples, format, tiling, usage)),
	memory_(create_image_memory(properties)),
	view_(create_view(format, aspect, mip_levels))
{ }

vk::raii::Image Image::create_image(
	u32 width,
	u32 height,
	u32 mip_levels,
	vk::SampleCountFlagBits num_samples,
	vk::Format format,
	vk::ImageTiling tiling,
	vk::ImageUsageFlags usage
)
{
	vk::ImageCreateInfo image_info {
		.imageType = vk::ImageType::e2D,
		.format = format,
		.extent = {width, height, 1},
		.mipLevels = mip_levels,
		.arrayLayers = 1,
		.samples = num_samples,
		.tiling = tiling,
		.usage = usage,
		.sharingMode = vk::SharingMode::eExclusive
	};

	return vk::raii::Image(Device::instance().logical(), image_info);
}

vk::raii::DeviceMemory Image::create_image_memory(
	vk::MemoryPropertyFlags properties
)
{
	vk::MemoryRequirements mem_requirements = image_.getMemoryRequirements();
	u32 mem_type = Device::instance().find_memory_type(mem_requirements.memoryTypeBits, properties);
	if (mem_type == ~0u) return nullptr;

	vk::MemoryAllocateInfo alloc_info = {
		.allocationSize = mem_requirements.size,
		.memoryTypeIndex = mem_type
	};

	vk::raii::DeviceMemory image_memory = vk::raii::DeviceMemory(Device::instance().logical(), alloc_info);
	image_.bindMemory(image_memory, 0);

	return std::move(image_memory);
}

vk::raii::ImageView Image::create_view(
	vk::Format format,
	vk::ImageAspectFlags aspect,
	u32 mip_levels
)
{
	vk::ImageViewCreateInfo view_info {
		.image = image_,
		.viewType = vk::ImageViewType::e2D,
		.format = format,
		.components = {
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity
		},
		.subresourceRange = {
			.aspectMask = aspect,
			.baseMipLevel = 0,
			.levelCount = mip_levels,
			.baseArrayLayer = 0,
			.layerCount = 1,
		}
	};

	return vk::raii::ImageView(Device::instance().logical(), view_info);
}

bool Image::is_valid() const
{
	return (image_ != nullptr && memory_ != nullptr && view_ != nullptr);
}
