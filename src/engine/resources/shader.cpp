#include "engine/resources/shader.hpp"

using namespace fluffy_fox;

Shader::Shader(const std::string& id, vk::ShaderStageFlagBits shader_stage) :
	Resource(id),
	stage_(shader_stage)
{

}

Shader::~Shader()
{
	unload();
}

bool Shader::do_load()
{
	// TODO
	// load file
	// load shader
	// transform data into vulkan resources
	// cleanup cpu resources
	throw std::runtime_error("not implemented");
}

void Shader::do_unload()
{
	if (!is_loaded()) return;

	// TODO
	// get device
	// destroy vk objects
	throw std::runtime_error("not implemented");
}

vk::ShaderModule Shader::get_shader_module() const
{
	return shader_module_;
}

vk::ShaderStageFlagBits Shader::get_stage() const
{
	return stage_;
}
