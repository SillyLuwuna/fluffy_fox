#pragma once

#include "engine/resources/texture.hpp"
#include "rendering/uniform_buffer.hpp"
#include <deque>
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include <fluff/types.hpp>
using namespace fluff::types;

namespace fluffy_fox::rendering
{
	class DescriptorWriter
	{
	private:
		std::deque<vk::DescriptorBufferInfo> buffer_keep_alive_; // deque for stable addresses
		std::deque<vk::DescriptorImageInfo> image_keep_alive_;
		std::vector<vk::WriteDescriptorSet> writes_;

	public:

		template <typename T>
		DescriptorWriter& write(u32 binding, const UniformBuffer<T>& uniform)
		{
			vk::DescriptorBufferInfo& info = buffer_keep_alive_.emplace_back(uniform.descriptor_info());

			writes_.emplace_back(vk::WriteDescriptorSet {
				.dstBinding = binding,
				.descriptorCount = 1,
				.descriptorType = uniform.descriptor_type(),
				.pBufferInfo = &info
			});

			return *this;
		}

		DescriptorWriter& write(u32 binding, const Texture& texture)
		{
			vk::DescriptorImageInfo& info = image_keep_alive_.emplace_back(texture.descriptor_info());

			writes_.emplace_back(vk::WriteDescriptorSet {
				.dstBinding = binding,
				.descriptorCount = 1,
				.descriptorType = texture.descriptor_type(),
				.pImageInfo = &info
			});

			return *this;
		}

		void update(vk::raii::DescriptorSet& set)
		{
			for (vk::WriteDescriptorSet& write : writes_)
			{
				write.dstSet = set;
				write.dstArrayElement = 0;
			}

			Device::instance().logical().updateDescriptorSets(writes_, {});
		}
	};
}
