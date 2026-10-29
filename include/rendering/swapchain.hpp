#pragma once

#include "rendering/device.hpp"
#include "rendering/image.hpp"
#include "rendering/window.hpp"

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include <fluff/types.hpp>
using namespace fluff::types;


namespace fluffy_fox::rendering
{
	class Swapchain
	{
	private:
		vk::Extent2D extent_;
		vk::SurfaceFormatKHR surface_format_;
		vk::raii::SwapchainKHR swapchain_;
		std::vector<vk::Image> images_;
		std::vector<vk::raii::ImageView> image_views_;
		Image color_;
		Image depth_;

		static vk::Extent2D choose_extent(const Window& window);
		static vk::SurfaceFormatKHR choose_surface_format(const Window& window);
		static vk::PresentModeKHR choose_present_mode(const Window& window);
		static u32 choose_min_image_count(const Window& window);
		vk::raii::SwapchainKHR create_swapchain(const Window& window);
		std::vector<vk::raii::ImageView> create_image_views();
		Image create_color_image() const;
		Image create_depth_image() const;

	public:
		Swapchain(const Window& window);

		inline vk::Extent2D& extent() { return extent_; }
		inline vk::SurfaceFormatKHR& surface_format() { return surface_format_; }
		inline vk::raii::SwapchainKHR& swapchain() { return swapchain_; }
		inline std::vector<vk::Image>& images() { return images_; }
		inline std::vector<vk::raii::ImageView>& image_views() { return image_views_; }

		void recreate(const Window& window);
		bool is_valid() const;

		inline Image& color() { return color_; }
		inline Image& depth() { return depth_; }
	};
}
