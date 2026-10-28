#include "rendering/swapchain.hpp"

#include "rendering/vulkan_image.hpp"

using namespace fluffy_fox::rendering;

Swapchain::Swapchain(Device& device, const Window& window) :
	extent_(choose_extent(device, window)),
	surface_format_(choose_surface_format(device, window)),
	swapchain_(create_swapchain(device, window)),
	images_(swapchain_.getImages()),
	image_views_(create_image_views(device)),
	color_(create_color_image(device)),
	depth_(create_depth_image(device))
{ }

vk::Extent2D Swapchain::choose_extent(Device& device, const Window& window)
{
	vk::SurfaceCapabilitiesKHR capabilities = device.physical().getSurfaceCapabilitiesKHR(*window.surface());
	if (capabilities.currentExtent.width != std::numeric_limits<u32>::max())
	{
		// if not allowed to specify resolution
		return capabilities.currentExtent;
	}

	auto [width, height] = window.framebuffer_size();

	return {
		std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
		std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
	};
}

vk::SurfaceFormatKHR Swapchain::choose_surface_format(Device& device, const Window& window)
{
	std::vector<vk::SurfaceFormatKHR> available_formats = device.physical().getSurfaceFormatsKHR(*window.surface());

	for (u64 i = 0; i < available_formats.size(); i++)
	{
		vk::SurfaceFormatKHR& format = available_formats[i];
		bool has_format = format.format == vk::Format::eB8G8R8A8Srgb;
		bool has_color = format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;

		if (has_format && has_color)
		{
			return format;
		}
	}

	return available_formats[0]; // fallback
}

vk::PresentModeKHR Swapchain::choose_present_mode(Device& device, const Window& window)
{
	std::vector<vk::PresentModeKHR> available_present_modes = device.physical().getSurfacePresentModesKHR(*window.surface());

	vk::PresentModeKHR best_present_mode = vk::PresentModeKHR::eFifo;

	for (const vk::PresentModeKHR& present_mode : available_present_modes)
	{
		if (present_mode == vk::PresentModeKHR::eImmediate)
		{
			best_present_mode = present_mode;
			break;
		}

		if (best_present_mode != vk::PresentModeKHR::eImmediate && present_mode == vk::PresentModeKHR::eMailbox)
		{
			best_present_mode = present_mode;
			break;
		}
	}

	return best_present_mode;
}

u32 Swapchain::choose_min_image_count(Device& device, const Window& window)
{
	vk::SurfaceCapabilitiesKHR surface_capabilities = device.physical().getSurfaceCapabilitiesKHR(*window.surface());

	u32 min_image_count = std::max(3u, surface_capabilities.minImageCount);
	if ((0 < surface_capabilities.maxImageCount) && (surface_capabilities.maxImageCount < min_image_count))
	{
		min_image_count = surface_capabilities.maxImageCount;
	}

	return min_image_count;
}

vk::raii::SwapchainKHR Swapchain::create_swapchain(Device& device, const Window& window)
{
	vk::SurfaceCapabilitiesKHR surface_capabilities = device.physical().getSurfaceCapabilitiesKHR(*window.surface());
	u32 min_image_count = choose_min_image_count(device, window);
	vk::PresentModeKHR present_mode = choose_present_mode(device, window);

	vk::SwapchainCreateInfoKHR swapchain_create_info {
		.surface = *window.surface(),
		.minImageCount = min_image_count,
		.imageFormat = surface_format_.format,
		.imageColorSpace = surface_format_.colorSpace,
		.imageExtent = extent_,
		.imageArrayLayers = 1,
		.imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
		.imageSharingMode = vk::SharingMode::eExclusive,
		.preTransform = surface_capabilities.currentTransform,
		.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
		.presentMode = present_mode,
		.clipped = true,
	};

	return vk::raii::SwapchainKHR(device.logical(), swapchain_create_info);
}

std::vector<vk::raii::ImageView> Swapchain::create_image_views(Device& device)
{
	std::vector<vk::raii::ImageView> image_views;

	image_views.reserve(images_.size());

	for (vk::Image& image : images_)
	{
		image_views.emplace_back(vulkan_image::create_image_view(
			device,
			image,
			surface_format_.format,
			vk::ImageAspectFlagBits::eColor,
			1
		));
	}

	return image_views;
}

void Swapchain::recreate(Device& device, const Window& window)
{
	image_views_.clear();
	swapchain_ = nullptr;

	extent_ = choose_extent(device, window);
	surface_format_ = choose_surface_format(device, window);
	swapchain_ = create_swapchain(device, window);
	images_ = swapchain_.getImages();
	image_views_ = create_image_views(device);
	color_ = create_color_image(device);
	depth_ = create_depth_image(device);
}

Image Swapchain::create_color_image(Device& device) const
{
	return Image(
		device,
		extent_.width,
		extent_.height,
		1,
		device.msaa_sample_count(),
		surface_format_.format,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		vk::ImageAspectFlagBits::eColor
	);
}

Image Swapchain::create_depth_image(Device& device) const
{
	return Image(
		device,
		extent_.width,
		extent_.height,
		1,
		device.msaa_sample_count(),
		device.depth_format(),
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eDepthStencilAttachment,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		vk::ImageAspectFlagBits::eDepth
	);
}

bool Swapchain::is_valid() const
{
	return color_.is_valid() && depth_.is_valid();
}
