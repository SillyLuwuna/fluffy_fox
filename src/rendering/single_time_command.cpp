#pragma once

#include "rendering/single_time_command.hpp"
using namespace fluffy_fox::rendering;

SingleTimeCommand::SingleTimeCommand() :
	command_buffer_(create_command_buffer())
{ }

vk::raii::CommandBuffer SingleTimeCommand::create_command_buffer()
{
	vk::CommandBufferAllocateInfo alloc_info {
		.commandPool = Device::instance().single_time_commands_command_pool(),
		.level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = 1
	};

	vk::raii::CommandBuffer command_buffer = std::move(
		vk::raii::CommandBuffers(Device::instance().logical(), alloc_info).front()
	);

	vk::CommandBufferBeginInfo begin_info {
		.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
	};

	command_buffer.begin(begin_info);

	return command_buffer;
}

SingleTimeCommand::~SingleTimeCommand()
{
	command_buffer_.end();

	vk::SubmitInfo submit_info {
		.commandBufferCount = 1,
		.pCommandBuffers = &*command_buffer_
	};

	Device::instance().single_time_commands().submit(submit_info, nullptr);
	Device::instance().single_time_commands().waitIdle();
}
