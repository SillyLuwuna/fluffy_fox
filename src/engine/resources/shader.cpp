#include "engine/resources/shader.hpp"
#include <fstream>

#include "fluff/types.hpp"
using namespace fluff::types;

using namespace fluffy_fox;

Shader::Shader(const std::string& id, Device& device, vk::ShaderStageFlagBits shader_stage) :
	Resource(id),
	device_(device),
	shader_module_(nullptr),
	stage_(shader_stage)
{

}

Shader::~Shader()
{
	unload();
}

bool Shader::do_load()
{
	std::vector<char> shader_code = read_file(get_id());
	if (shader_code.empty()) return false;

	vk::ShaderModuleCreateInfo create_info {
		.codeSize = shader_code.size() * sizeof(char),
		.pCode = reinterpret_cast<const u32*>(shader_code.data())
	};

	shader_module_ = vk::raii::ShaderModule(device_.logical(), create_info);

	return true;
}

void Shader::do_unload()
{
	if (!is_loaded()) return;

	shader_module_ = nullptr;
}

const vk::raii::ShaderModule& Shader::get_shader_module() const
{
	return shader_module_;
}

vk::ShaderStageFlagBits Shader::get_stage() const
{
	return stage_;
}
