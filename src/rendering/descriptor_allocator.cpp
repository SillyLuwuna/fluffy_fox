#include "rendering/descriptor_allocator.hpp"
#include <map>

using namespace fluffy_fox::rendering;

DescriptorAllocator::DescriptorAllocator(u32 max_sets, const Pipeline& pipeline) :
	pool_(create_pool(max_sets, pipeline))
{ }

vk::raii::DescriptorPool DescriptorAllocator::create_pool(u32 max_sets, const Pipeline& pipeline)
{
	std::vector<vk::DescriptorPoolSize> sizes = sizes_for(pipeline, max_sets);

	vk::DescriptorPoolCreateInfo pool_info {
		.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
		.maxSets = max_sets,
		.poolSizeCount = static_cast<u32>(sizes.size()),
		.pPoolSizes = sizes.data()
	};

	return vk::raii::DescriptorPool(Device::instance().logical(), pool_info);
}

std::vector<vk::raii::DescriptorSet> DescriptorAllocator::allocate(vk::DescriptorSetLayout layout, u32 count)
{
	std::vector<vk::DescriptorSetLayout> layouts(count, layout);

	vk::DescriptorSetAllocateInfo allocInfo {
		.descriptorPool = pool_,
		.descriptorSetCount = static_cast<u32>(layouts.size()),
		.pSetLayouts = layouts.data()
	};

	return Device::instance().logical().allocateDescriptorSets(allocInfo);
}

std::vector<vk::DescriptorPoolSize> DescriptorAllocator::sizes_for(
	const Pipeline& pipeline,
	u32 set_count
)
{
	std::span<const vk::DescriptorSetLayoutBinding> bindings = pipeline.descriptor_set_layout_bindings();
	std::map<vk::DescriptorType, u32> totals;
	for (const vk::DescriptorSetLayoutBinding& binding : bindings)
	{
		totals[binding.descriptorType] += binding.descriptorCount * set_count;
	}

	std::vector<vk::DescriptorPoolSize> sizes;
	for (auto& [type, count] : totals)
	{
		sizes.emplace_back(vk::DescriptorPoolSize {
			.type = type,
			.descriptorCount = count
		});
	}
	return sizes;
}
