#pragma once

#include "rendering/command.hpp"
#include "rendering/device.hpp"

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include <fluff/types.hpp>
using namespace fluff::types;

namespace fluffy_fox::rendering
{
	class SingleTimeCommand : public Command
	{
	// PERF use a separate command pool for these short-lived buffers, for optimizations.
	// vk::CommandPoolCreateFlagBits::eTransient would be needed in command pool generation, in that case.
	// PERF we can use vk::raii::waitForFences instead if we want to schedule multiple transfers simultaneously
	public:
		SingleTimeCommand() :
			Command(
				Device::instance().single_time_commands_command_pool(),
				true
			)
		{ }
	};
}
