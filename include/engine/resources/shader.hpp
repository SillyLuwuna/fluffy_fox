#pragma once

#include <vulkan/vulkan.hpp>
#include "engine/resource.hpp"

namespace fluffy_fox
{
	class Shader : public Resource
	{
	private:
		// TODO change everything to RAII, and initialize with nullptr
		vk::ShaderModule shader_module_;
		vk::ShaderStageFlagBits stage_;

	protected:
		bool do_load() override;
		void do_unload() override;

	public:
		explicit Shader(const std::string& id, vk::ShaderStageFlagBits shader_stage);
		~Shader() override;

		vk::ShaderModule get_shader_module() const;
		vk::ShaderStageFlagBits get_stage() const;
	};
}
