#pragma once

#include "rendering/command.hpp"
using namespace fluffy_fox::rendering;

Command::Command(vk::raii::CommandPool& command_pool, bool one_time) :
	command_buffer_(create_command_buffer(command_pool, one_time))
{ }

vk::raii::CommandBuffer Command::create_command_buffer(
	vk::raii::CommandPool& command_pool,
	bool one_time
)
{
	vk::CommandBufferAllocateInfo alloc_info {
		.commandPool = command_pool,
		.level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = 1
	};

	vk::raii::CommandBuffer command_buffer = std::move(
		vk::raii::CommandBuffers(Device::instance().logical(), alloc_info).front()
	);

	vk::CommandBufferBeginInfo begin_info {};

	if (one_time)
	{
		begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	}

	command_buffer.begin(begin_info);

	return command_buffer;
}

Command::~Command()
{
	command_buffer_.end();

	vk::SubmitInfo submit_info {
		.commandBufferCount = 1,
		.pCommandBuffers = &*command_buffer_
	};

	Device::instance().single_time_commands().submit(submit_info, nullptr);
	// FIXME might not be the best idea to wait idle on every command...
	Device::instance().single_time_commands().waitIdle();
}

void Command::copy_buffer(
	GpuBuffer& dst_buffer,
	const GpuBuffer& src_buffer,
	vk::DeviceSize size
)
{
	command_buffer_.copyBuffer(
		*src_buffer.buffer,
		*dst_buffer.buffer,
		vk::BufferCopy(0, 0, size)
	);
}

// requires image to be in TransferDstOptimal image layout
void Command::copy_buffer_to_image(const GpuBuffer& buffer, GpuImage& image)
{
	vk::BufferImageCopy region {
		.bufferOffset = 0,
		.bufferRowLength = 0,
		.bufferImageHeight = 0,
		.imageSubresource = {
			.aspectMask = vk::ImageAspectFlagBits::eColor,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1
		},
		.imageOffset = {0, 0, 0},
		.imageExtent = {image.width, image.height, 1}
	};

	command_buffer_.copyBufferToImage(
		buffer.buffer,
		image.image,
		vk::ImageLayout::eTransferDstOptimal,
		region
	);
}

// returns if the transition was supported
bool Command::transition_image_layout(
	const GpuImage& image,
	vk::ImageLayout old_layout,
	vk::ImageLayout new_layout
)
{
	vk::ImageMemoryBarrier barrier {
		.oldLayout = old_layout,
		.newLayout = new_layout,
		.srcQueueFamilyIndex = vk::QueueFamilyIgnored,
		.dstQueueFamilyIndex = vk::QueueFamilyIgnored,
		.image = image.image,
		.subresourceRange = {
			.aspectMask = vk::ImageAspectFlagBits::eColor,
			.levelCount = image.mip_levels,
			.layerCount = 1
		}
	};

	vk::PipelineStageFlags source_stage;
	vk::PipelineStageFlags destination_stage;

	if (old_layout == vk::ImageLayout::eUndefined &&
		new_layout == vk::ImageLayout::eTransferDstOptimal)
	{
		barrier.srcAccessMask = {};
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		source_stage = vk::PipelineStageFlagBits::eTopOfPipe;
		destination_stage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if (old_layout == vk::ImageLayout::eTransferDstOptimal &&
			new_layout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		source_stage = vk::PipelineStageFlagBits::eTransfer;
		destination_stage = vk::PipelineStageFlagBits::eFragmentShader;
	}
	else
	{
		return false;
	}

	command_buffer_.pipelineBarrier(
		source_stage,
		destination_stage,
		{},
		nullptr,
		nullptr,
		barrier
	);

	return true;
}

void Command::generate_mipmaps(GpuImage& image)
{
	// TODO cleanup
	// PERF usually done once and stored. Not generated each time.

	vk::FormatProperties format_properties = Device::instance().physical().getFormatProperties(image.format);
	if (!(format_properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear))
	{
		throw std::runtime_error("texture image format does not support linear blitting!");
	}

	vk::ImageMemoryBarrier barrier = {
		.srcAccessMask = vk::AccessFlagBits::eTransferWrite,
		.dstAccessMask = vk::AccessFlagBits::eTransferRead,
		.oldLayout = vk::ImageLayout::eTransferDstOptimal,
		.newLayout = vk::ImageLayout::eTransferSrcOptimal,
		.srcQueueFamilyIndex = vk::QueueFamilyIgnored,
		.dstQueueFamilyIndex = vk::QueueFamilyIgnored,
		.image = image.image,
		.subresourceRange = {
			.aspectMask = vk::ImageAspectFlagBits::eColor,
			.levelCount = 1,
			.layerCount = 1
		}
	};

	int32_t mip_width = image.width;
	int32_t mip_height = image.height;

	for (uint32_t i = 1; i < image.mip_levels; i++)
	{
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

		command_buffer_.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer,
			vk::PipelineStageFlagBits::eTransfer,
			{},
			nullptr,
			nullptr,
			barrier
		);

		vk::ImageBlit blit = {
			.srcSubresource = {
				.aspectMask = vk::ImageAspectFlagBits::eColor,
				.mipLevel = i - 1,
				.layerCount = 1
			},
			.srcOffsets = std::array<vk::Offset3D, 2>({ { }, {
					mip_width,
					mip_height,
					1
				}
			}),
			.dstSubresource = {
				.aspectMask = vk::ImageAspectFlagBits::eColor,
				.mipLevel = i,
				.layerCount = 1
			},
			.dstOffsets = std::array<vk::Offset3D, 2>({
				{ },
				{
					1 < mip_width ? mip_width / 2 : 1,
					1 < mip_height ? mip_height / 2 : 1,
					1
				}
			})
		};

		command_buffer_.blitImage(
			image.image,
			vk::ImageLayout::eTransferSrcOptimal,
			image.image,
			vk::ImageLayout::eTransferDstOptimal,
			blit,
			vk::Filter::eLinear
		);

		barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
		barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		command_buffer_.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer,
			vk::PipelineStageFlagBits::eFragmentShader,
			{},
			nullptr,
			nullptr,
			barrier
		);

		if (1 < mip_width)
		{
			mip_width /= 2;
		}
		if (1 < mip_height)
		{
			mip_height /= 2;
		}
	}

	barrier.subresourceRange.baseMipLevel = image.mip_levels - 1;
	barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
	barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
	barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

	command_buffer_.pipelineBarrier(
		vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eFragmentShader,
		{},
		nullptr,
		nullptr,
		barrier
	);
}
