#pragma once

#include "engine/resources/texture.hpp"
#include "rendering/pipeline.hpp"
#include "rendering/uniform_buffer.hpp"
#include <deque>
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include <fluff/types.hpp>
using namespace fluff::types;

namespace fluffy_fox::rendering
{
	class DescriptorAllocator
	{
	private:
		vk::raii::DescriptorPool pool_;

		vk::raii::DescriptorPool create_pool(u32 max_sets, const Pipeline& pipeline);
		std::vector<vk::DescriptorPoolSize> sizes_for(
			const Pipeline& pipeline,
			u32 set_count
		);

	public:
		DescriptorAllocator(u32 max_sets, const Pipeline& pipeline);

		std::vector<vk::raii::DescriptorSet> allocate(vk::DescriptorSetLayout layout, u32 count);
	};
}
