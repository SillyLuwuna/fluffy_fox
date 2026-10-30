#pragma once

#include "rendering/device.hpp"
#include "rendering/gpu_buffer.hpp"
#include "rendering/gpu_image.hpp"

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include <fluff/types.hpp>
using namespace fluff::types;

namespace fluffy_fox::rendering
{
	class Command
	{
	private:
		vk::raii::CommandBuffer command_buffer_;

		vk::raii::CommandBuffer create_command_buffer(
			vk::raii::CommandPool& command_pool,
			bool one_time
		);

	public:
		Command(vk::raii::CommandPool& command_pool, bool one_time = false);
		virtual ~Command();

		inline vk::raii::CommandBuffer& command_buffer() { return command_buffer_; }

		void copy_buffer(
			GpuBuffer& dst_buffer,
			const GpuBuffer& src_buffer,
			vk::DeviceSize size
		);

		void copy_buffer_to_image(const GpuBuffer& buffer, GpuImage& image);

		bool transition_image_layout(
			const GpuImage& image,
			vk::ImageLayout old_layout,
			vk::ImageLayout new_layout
		);

		void generate_mipmaps(GpuImage& image);
	};
}
