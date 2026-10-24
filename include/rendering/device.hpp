#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include <fluff/types.hpp>
using namespace fluff::types;


namespace fluffy_fox::rendering
{
	class Device
	{
	private:
		vk::raii::PhysicalDevice physical_device_;
		vk::raii::Device device_;

		static vk::raii::PhysicalDevice choose_physical_device(vk::raii::Instance& instance);
		static bool is_physical_device_suitable(vk::raii::PhysicalDevice& device);
		static bool supports_vulkan_version(vk::raii::PhysicalDevice& device, u32 version);
		static bool supports_queue_type(vk::raii::PhysicalDevice& device, vk::QueueFlags queue_flags);
		static bool supports_required_extensions(vk::raii::PhysicalDevice& device);
		static bool supports_required_features(vk::raii::PhysicalDevice& device);
		static std::vector<std::string> get_physical_device_required_extensions();
		static i32 device_score(vk::raii::PhysicalDevice& device);

		static vk::raii::Device create_logical_device(vk::raii::PhysicalDevice& physical_device);
		static i32 find_graphics_queue_index(vk::raii::PhysicalDevice& physical_device);

	public:
		Device(vk::raii::Instance& instance);

		vk::raii::PhysicalDevice& physical();
		vk::raii::Device& logical();
		bool is_valid();
	};
}
