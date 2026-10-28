#include "rendering/pipeline.hpp"

#include "rendering/vertex.hpp"
#include <iostream>

using namespace fluffy_fox;
using namespace fluffy_fox::rendering;

Pipeline::Pipeline(
	ResourceManager& resource_manager,
	Device& device,
	Swapchain& swapchain
) :
	descriptor_set_layout_(create_descriptor_set_layout(device)),
	pipeline_layout_(create_pipeline_layout(device)),
	pipeline_(create_graphics_pipeline(resource_manager, device, swapchain))
{ }

vk::raii::DescriptorSetLayout Pipeline::create_descriptor_set_layout(Device& device) const
{
	std::vector<vk::DescriptorSetLayoutBinding> bindings = get_descriptor_set_layout_bindings();

	vk::DescriptorSetLayoutCreateInfo layout_info {
		.bindingCount = static_cast<u32>(bindings.size()),
		.pBindings = bindings.data()
	};

	return vk::raii::DescriptorSetLayout(device.logical(), layout_info);
}

std::vector<vk::DescriptorSetLayoutBinding> Pipeline::get_descriptor_set_layout_bindings() const
{
	return {
		{
			.binding = 0,
			.descriptorType = vk::DescriptorType::eUniformBuffer,
			.descriptorCount = 1,
			.stageFlags = vk::ShaderStageFlagBits::eVertex
		},
		{
			.binding = 1,
			.descriptorType = vk::DescriptorType::eCombinedImageSampler,
			.descriptorCount = 1,
			.stageFlags = vk::ShaderStageFlagBits::eFragment
		}
	};
}

vk::raii::PipelineLayout Pipeline::create_pipeline_layout(Device& device) const
{
	vk::PipelineLayoutCreateInfo pipeline_layout_info {
		.setLayoutCount = 1,
		.pSetLayouts = &*descriptor_set_layout_,
		.pushConstantRangeCount = 0
	};

	return vk::raii::PipelineLayout(device.logical(), pipeline_layout_info);
}

vk::raii::Pipeline Pipeline::create_graphics_pipeline(
	ResourceManager& resource_manager,
	Device& device,
	Swapchain& swapchain
) const
{
	ResourceHandle<Shader> shader = get_shader(resource_manager, device);
	if (!shader) return nullptr;
	std::vector<vk::PipelineShaderStageCreateInfo> shader_stages = get_shader_stages(shader);

	std::vector<vk::DynamicState> dynamic_states = get_dynamic_states();
	vk::PipelineDynamicStateCreateInfo dynamic_state = get_dynamic_state(dynamic_states);

	vk::VertexInputBindingDescription binding_description = Vertex::get_binding_description();
	std::vector<vk::VertexInputAttributeDescription> attribute_descriptions = Vertex::get_attribute_descriptions();
	vk::PipelineVertexInputStateCreateInfo vertex_input_info = get_vertex_input_info(binding_description, attribute_descriptions);

	vk::PipelineInputAssemblyStateCreateInfo input_assembly = get_input_assembly();
	vk::PipelineViewportStateCreateInfo viewport_state = get_viewport_state();
	vk::PipelineRasterizationStateCreateInfo rasterizer = get_rasterizer();
	vk::PipelineMultisampleStateCreateInfo multisampler = get_multisampler(device);
	vk::PipelineDepthStencilStateCreateInfo depth_stencil = get_depth_stencil();

	vk::PipelineColorBlendAttachmentState color_blend_attachment = get_color_blend_attachment();
	vk::PipelineColorBlendStateCreateInfo color_blend = get_color_blend(color_blend_attachment);

	vk::PipelineRenderingCreateInfo pipeline_rendering_create_info = get_pipeline_rendering_create_info(device, swapchain);

	vk::GraphicsPipelineCreateInfo graphics_pipeline_create_info {
		.stageCount = static_cast<u32>(shader_stages.size()),
		.pStages = shader_stages.data(),
		.pVertexInputState = &vertex_input_info,
		.pInputAssemblyState = &input_assembly,
		.pViewportState = &viewport_state,
		.pRasterizationState = &rasterizer,
		.pMultisampleState = &multisampler,
		.pDepthStencilState = &depth_stencil,
		.pColorBlendState = &color_blend,
		.pDynamicState = &dynamic_state,
		.layout = pipeline_layout_,
		.renderPass = nullptr // dynamic rendering
	};

	vk::StructureChain<
		vk::GraphicsPipelineCreateInfo,
		vk::PipelineRenderingCreateInfo
	> pipeline_create_info_chain = {
		graphics_pipeline_create_info,
		pipeline_rendering_create_info
	};

	return vk::raii::Pipeline(
		device.logical(),
		nullptr,
		pipeline_create_info_chain.get<vk::GraphicsPipelineCreateInfo>()
	);
}

ResourceHandle<Shader> Pipeline::get_shader(
		ResourceManager& resource_manager,
		Device& device
) const
{
	return resource_manager.load<Shader>(
		"shaders/slang.spv",
		device,
		vk::ShaderStageFlagBits::eAllGraphics
	);
}

std::vector<vk::PipelineShaderStageCreateInfo> Pipeline::get_shader_stages(
	ResourceHandle<Shader>& shader
) const
{
	vk::PipelineShaderStageCreateInfo vertex_shader_stage_info {
		.stage = vk::ShaderStageFlagBits::eVertex,
		.module = shader->get_shader_module(),
		.pName = "vertMain",
		.pSpecializationInfo = nullptr
	};

	vk::PipelineShaderStageCreateInfo fragment_shader_stage_info {
		.stage = vk::ShaderStageFlagBits::eFragment,
		.module = shader->get_shader_module(),
		.pName = "fragMain",
		.pSpecializationInfo = nullptr
	};

	return {
		vertex_shader_stage_info,
		fragment_shader_stage_info
	};
}

std::vector<vk::DynamicState> Pipeline::get_dynamic_states() const
{
	return {
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor
	};
}

vk::PipelineDynamicStateCreateInfo Pipeline::get_dynamic_state(
	std::vector<vk::DynamicState>& dynamic_states
) const
{
	return {
		.dynamicStateCount = static_cast<u32>(dynamic_states.size()),
		.pDynamicStates = dynamic_states.data()
	};
}

vk::PipelineVertexInputStateCreateInfo Pipeline::get_vertex_input_info(
	vk::VertexInputBindingDescription& binding_description,
	std::vector<vk::VertexInputAttributeDescription>& attribute_descriptions
) const
{
	return {
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &binding_description,
		.vertexAttributeDescriptionCount = static_cast<u32>(attribute_descriptions.size()),
		.pVertexAttributeDescriptions = attribute_descriptions.data()
	};
}

vk::PipelineInputAssemblyStateCreateInfo Pipeline::get_input_assembly() const
{
	return {
		.topology = vk::PrimitiveTopology::eTriangleList
	};
}

vk::PipelineViewportStateCreateInfo Pipeline::get_viewport_state() const
{
	return {
		.viewportCount = 1,
		.scissorCount = 1
	};
}

vk::PipelineRasterizationStateCreateInfo Pipeline::get_rasterizer() const
{
	return {
		.depthClampEnable = false,
		.rasterizerDiscardEnable = false,
		.polygonMode = vk::PolygonMode::eFill,
		.cullMode = vk::CullModeFlagBits::eBack,
		.frontFace = vk::FrontFace::eCounterClockwise,
		.depthBiasEnable = false,
		.lineWidth = 1.0f
	};
}

vk::PipelineMultisampleStateCreateInfo Pipeline::get_multisampler(Device& device) const
{
	return {
		.rasterizationSamples = device.msaa_sample_count(),
		.sampleShadingEnable = true, // PERF slow but better quality
		.minSampleShading = 0.2f
	};
}

vk::PipelineDepthStencilStateCreateInfo Pipeline::get_depth_stencil() const
{
	return {
		.depthTestEnable = true,
		.depthWriteEnable = true,
		.depthCompareOp = vk::CompareOp::eLess,
		.depthBoundsTestEnable = false,
		.stencilTestEnable = false
	};
}

vk::PipelineColorBlendAttachmentState Pipeline::get_color_blend_attachment() const
{
	return {
		.blendEnable = true,
		.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
		.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
		.colorBlendOp = vk::BlendOp::eAdd,
		.srcAlphaBlendFactor = vk::BlendFactor::eOne,
		.dstAlphaBlendFactor = vk::BlendFactor::eZero,
		.alphaBlendOp = vk::BlendOp::eAdd,
		.colorWriteMask =
			vk::ColorComponentFlagBits::eR |
			vk::ColorComponentFlagBits::eG |
			vk::ColorComponentFlagBits::eB |
			vk::ColorComponentFlagBits::eA
	};
}

vk::PipelineColorBlendStateCreateInfo Pipeline::get_color_blend(
	vk::PipelineColorBlendAttachmentState& color_blend_attachment
) const
{
	return {
		.logicOpEnable = false,
		.logicOp = vk::LogicOp::eCopy,
		.attachmentCount = 1,
		.pAttachments = &color_blend_attachment
	};
}

vk::PipelineRenderingCreateInfo Pipeline::get_pipeline_rendering_create_info(
	Device& device,
	Swapchain& swapchain
) const
{
	return {
		.colorAttachmentCount = 1,
		.pColorAttachmentFormats = &swapchain.surface_format().format,
		.depthAttachmentFormat = device.depth_format()
	};
}

bool Pipeline::is_valid() const
{
	return pipeline_ != nullptr;
}
