#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include <fluff/types.hpp>
using namespace fluff::types;

namespace fluffy_fox::rendering
{
	class SingleTimeCommand;

	class Device
	{
	private:
		Device();

		struct FeatureChain
		{
			vk::StructureChain<
				vk::PhysicalDeviceFeatures2,
				vk::PhysicalDeviceVulkan11Features,
				vk::PhysicalDeviceVulkan13Features,
				vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
			> feature_chain;

			inline void* get()
			{
				return &feature_chain.get<vk::PhysicalDeviceFeatures2>();
			}
		};

		struct QueueInfo
		{
			u32 family;
			u32 index;
			f32 priority;
			std::optional<vk::raii::Queue> queue;
		};

		QueueInfo graphics_;
		QueueInfo present_;
		QueueInfo transfer_;
		QueueInfo compute_;

		vk::raii::PhysicalDevice physical_device_;
		vk::raii::Device device_;

		vk::SampleCountFlagBits msaa_sample_count_;
		vk::raii::CommandPool graphics_command_pool_;
		vk::Format depth_format_;

		void init(vk::raii::Instance& instance, vk::SurfaceKHR surface);

		static vk::raii::PhysicalDevice choose_physical_device(vk::raii::Instance& instance);
		static bool is_physical_device_suitable(vk::raii::PhysicalDevice& device);
		static bool supports_vulkan_version(vk::raii::PhysicalDevice& device, u32 version);
		static bool supports_queue_type(vk::raii::PhysicalDevice& device, vk::QueueFlags queue_flags);
		static bool supports_required_extensions(vk::raii::PhysicalDevice& device);
		static bool supports_required_features(vk::raii::PhysicalDevice& device);
		static std::vector<std::string> get_physical_device_required_extensions();
		static i32 device_score(vk::raii::PhysicalDevice& device);

		vk::raii::Device create_logical_device(vk::raii::PhysicalDevice& physical_device, vk::SurfaceKHR surface);
		static i32 find_graphics_queue_index(vk::raii::PhysicalDevice& physical_device, vk::SurfaceKHR surface);
		static i32 find_present_queue_index(vk::raii::PhysicalDevice& physical_device, vk::SurfaceKHR surface);
		static i32 find_transfer_queue_index(vk::raii::PhysicalDevice& physical_device);
		static i32 find_compute_queue_index(vk::raii::PhysicalDevice& physical_device);
		static FeatureChain get_desired_feature_chain();
		static std::vector<const char*> get_logical_device_required_extensions();
		std::vector<vk::DeviceQueueCreateInfo> get_queue_create_infos(vk::SurfaceKHR surface);
		void create_graphics_queues(std::vector<vk::DeviceQueueCreateInfo>& out, vk::SurfaceKHR surface);
		void create_present_queues(std::vector<vk::DeviceQueueCreateInfo>& out, vk::SurfaceKHR surface);
		void create_transfer_queues(std::vector<vk::DeviceQueueCreateInfo>& out);
		void create_compute_queues(std::vector<vk::DeviceQueueCreateInfo>& out);
		static void init_queue(QueueInfo& queue, vk::raii::Device& device);

		vk::SampleCountFlagBits get_max_usable_msaa_sample_count() const;
		vk::raii::CommandPool create_command_pool(u32 family) const;
		vk::Format find_depth_format() const;
		vk::Format find_supported_format(
			const std::vector<vk::Format>& candidates,
			vk::ImageTiling tiling,
			vk::FormatFeatureFlags features
		) const;

	public:
		Device(const Device&) = delete;
		Device(Device&&) = delete;
		Device& operator=(const Device&) = delete;
		Device& operator=(Device&&) = delete;

		static void create(vk::raii::Instance& instance, vk::SurfaceKHR surface);
		inline static Device& instance()
		{
			static Device instance;

			return instance;
		}

		inline vk::raii::PhysicalDevice& physical() { return physical_device_; } // TODO remove
		inline vk::raii::Device& logical() { return device_; } // TODO remove
		bool is_valid();

		inline vk::raii::Queue& graphics() { return graphics_.queue.value(); }
		inline vk::raii::Queue& present() { return present_.queue.value(); }
		inline vk::raii::Queue& transfer() { return transfer_.queue.value(); }
		inline vk::raii::Queue& compute() { return compute_.queue.value(); }
		vk::raii::Queue& single_time_commands();

		inline u32 graphics_family() const { return graphics_.family; }
		inline u32 present_family() const { return present_.family; }
		inline u32 transfer_family() const { return transfer_.family; }
		inline u32 compute_family() const { return compute_.family; }

		inline vk::SampleCountFlagBits msaa_sample_count() const { return msaa_sample_count_; }
		inline vk::raii::CommandPool& graphics_command_pool() { return graphics_command_pool_; }
		inline vk::Format depth_format() const { return depth_format_; }
		vk::raii::CommandPool& single_time_commands_command_pool();
		// TODO others

		u32 find_memory_type(u32 type_filter, vk::MemoryPropertyFlags properties);
	};
}
