#include "engine/rendergraph.hpp"

using namespace fluffy_fox;

Rendergraph::Rendergraph(vk::raii::Device& device) :
	device_(device)
{ }

void Rendergraph::add_resource(
	const std::string& name,
	vk::Format format,
	vk::Extent2D extent,
	vk::ImageUsageFlags usage,
	vk::ImageLayout initial_layout,
	vk::ImageLayout final_layout
)
{
	RenderResource resource;
	resource.name = name;
	resource.format = format;
	resource.extent = extent;
	resource.usage = usage;
	resource.initial_layout = initial_layout;
	resource.final_layout = final_layout;

	resources_[name] = std::move(resource);
}

void Rendergraph::add_pass(
	const std::string& name,
	const std::vector<std::string>& inputs,
	const std::vector<std::string>& outputs,
	std::function<void(vk::raii::CommandBuffer&)> execute_function
)
{
	Pass pass;
	pass.name = name;
	pass.inputs = inputs;
	pass.outputs = outputs;
	pass.execute_func = execute_function;

	passes_.emplace_back(pass);
}

void Rendergraph::compile()
{

}
