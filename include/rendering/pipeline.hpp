#pragma once

#include "engine/resource_manager.hpp"
#include "engine/resources/shader.hpp"
#include "rendering/device.hpp"
#include "rendering/swapchain.hpp"
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include <fluff/types.hpp>
using namespace fluff::types;


namespace fluffy_fox::rendering
{
	class Pipeline
	{
	private:
		vk::raii::DescriptorSetLayout descriptor_set_layout_;
		vk::raii::PipelineLayout pipeline_layout_;
		vk::raii::Pipeline pipeline_;

		vk::raii::DescriptorSetLayout create_descriptor_set_layout(Device& device) const;
		std::vector<vk::DescriptorSetLayoutBinding> get_descriptor_set_layout_bindings() const;

		vk::raii::PipelineLayout create_pipeline_layout(Device& device) const;

		vk::raii::Pipeline create_graphics_pipeline(
			ResourceManager& resource_manager,
			Device& device,
			Swapchain& swapchain
		) const;

		ResourceHandle<Shader> get_shader(
			ResourceManager& resource_manager,
			Device& device
		) const;

		std::vector<vk::PipelineShaderStageCreateInfo> get_shader_stages(
			ResourceHandle<Shader>& shader
		) const;

		std::vector<vk::DynamicState> get_dynamic_states() const;

		vk::PipelineDynamicStateCreateInfo get_dynamic_state(
			std::vector<vk::DynamicState>& dynamic_states
		) const;

		vk::PipelineVertexInputStateCreateInfo get_vertex_input_info(
			vk::VertexInputBindingDescription& binding_description,
			std::vector<vk::VertexInputAttributeDescription>& attribute_descriptions
		) const;

		vk::PipelineInputAssemblyStateCreateInfo get_input_assembly() const;
		vk::PipelineViewportStateCreateInfo get_viewport_state() const;
		vk::PipelineRasterizationStateCreateInfo get_rasterizer() const;
		vk::PipelineMultisampleStateCreateInfo get_multisampler(Device& device) const;
		vk::PipelineDepthStencilStateCreateInfo get_depth_stencil() const;
		vk::PipelineColorBlendAttachmentState get_color_blend_attachment() const;

		vk::PipelineColorBlendStateCreateInfo get_color_blend(
			vk::PipelineColorBlendAttachmentState& color_blend_attachment
		) const;

		vk::PipelineRenderingCreateInfo get_pipeline_rendering_create_info(
			Device& device,
			Swapchain& swapchain
		) const;

	public:
		Pipeline(
			ResourceManager& resource_manager,
			Device& device,
			Swapchain& swapchain
		);

		bool is_valid() const;

		inline const vk::raii::DescriptorSetLayout& descriptor_set_layout() const { return descriptor_set_layout_; }
		inline const vk::raii::PipelineLayout& layout() const { return pipeline_layout_; }
		inline const vk::raii::Pipeline& pipeline() const { return pipeline_; }
	};
}
