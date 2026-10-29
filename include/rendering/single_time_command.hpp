#pragma once

#include "rendering/device.hpp"

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include <fluff/types.hpp>
using namespace fluff::types;

namespace fluffy_fox::rendering
{
	class SingleTimeCommand
	{
	private:
		vk::raii::CommandBuffer command_buffer_;

		vk::raii::CommandBuffer create_command_buffer();

	public:
		SingleTimeCommand();
		~SingleTimeCommand();

		inline vk::raii::CommandBuffer& command_buffer() { return command_buffer_; }
	};
}
