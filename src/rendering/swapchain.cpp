#include "rendering/swapchain.hpp"

#include "rendering/vulkan_image.hpp"
#include "rendering/gpu.hpp"

using namespace fluffy_fox::rendering;

Swapchain::Swapchain(const Window& window) :
	extent_(choose_extent(window)),
	surface_format_(choose_surface_format(window)),
	swapchain_(create_swapchain(window)),
	images_(swapchain_.getImages()),
	image_views_(create_image_views()),
	color_(create_color_image()),
	depth_(create_depth_image())
{ }

vk::Extent2D Swapchain::choose_extent(const Window& window)
{
	vk::SurfaceCapabilitiesKHR capabilities = Device::instance().physical().getSurfaceCapabilitiesKHR(*window.surface());
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

vk::SurfaceFormatKHR Swapchain::choose_surface_format(const Window& window)
{
	std::vector<vk::SurfaceFormatKHR> available_formats = Device::instance().physical().getSurfaceFormatsKHR(*window.surface());

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

vk::PresentModeKHR Swapchain::choose_present_mode(const Window& window)
{
	std::vector<vk::PresentModeKHR> available_present_modes = Device::instance().physical().getSurfacePresentModesKHR(*window.surface());

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

u32 Swapchain::choose_min_image_count(const Window& window)
{
	vk::SurfaceCapabilitiesKHR surface_capabilities = Device::instance().physical().getSurfaceCapabilitiesKHR(*window.surface());

	u32 min_image_count = std::max(3u, surface_capabilities.minImageCount);
	if ((0 < surface_capabilities.maxImageCount) && (surface_capabilities.maxImageCount < min_image_count))
	{
		min_image_count = surface_capabilities.maxImageCount;
	}

	return min_image_count;
}

vk::raii::SwapchainKHR Swapchain::create_swapchain(const Window& window)
{
	vk::SurfaceCapabilitiesKHR surface_capabilities = Device::instance().physical().getSurfaceCapabilitiesKHR(*window.surface());
	u32 min_image_count = choose_min_image_count(window);
	vk::PresentModeKHR present_mode = choose_present_mode(window);

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

	return vk::raii::SwapchainKHR(Device::instance().logical(), swapchain_create_info);
}

std::vector<vk::raii::ImageView> Swapchain::create_image_views()
{
	std::vector<vk::raii::ImageView> image_views;

	image_views.reserve(images_.size());

	for (vk::Image& image : images_)
	{
		image_views.emplace_back(vulkan_image::create_image_view(
			image,
			surface_format_.format,
			vk::ImageAspectFlagBits::eColor,
			1
		));
	}

	return image_views;
}

void Swapchain::recreate(const Window& window)
{
	image_views_.clear();
	swapchain_ = nullptr;

	extent_ = choose_extent(window);
	surface_format_ = choose_surface_format(window);
	swapchain_ = create_swapchain(window);
	images_ = swapchain_.getImages();
	image_views_ = create_image_views();
	color_ = create_color_image();
	depth_ = create_depth_image();
}

GpuImage Swapchain::create_color_image() const
{
	return Gpu::create_image(
		extent_.width,
		extent_.height,
		1,
		Device::instance().msaa_sample_count(),
		surface_format_.format,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment,
		vk::ImageAspectFlagBits::eColor,
		vk::MemoryPropertyFlagBits::eDeviceLocal
	);
}

GpuImage Swapchain::create_depth_image() const
{
	return Gpu::create_image(
		extent_.width,
		extent_.height,
		1,
		Device::instance().msaa_sample_count(),
		Device::instance().depth_format(),
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eDepthStencilAttachment,
		vk::ImageAspectFlagBits::eDepth,
		vk::MemoryPropertyFlagBits::eDeviceLocal
	);
}

bool Swapchain::is_valid() const
{
	// return color_.is_valid() && depth_.is_valid();
	return true; // FIXME
}
