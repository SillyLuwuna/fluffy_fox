#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include <fstream>

#include "engine/resource.hpp"
#include "rendering/device.hpp"

using namespace fluffy_fox::rendering;

namespace fluffy_fox
{
	class Shader : public Resource
	{
	private:
		Device& device_;
		vk::raii::ShaderModule shader_module_;
		vk::ShaderStageFlagBits stage_;

		// returns empty vector when an error ocurred
		static std::vector<char> read_file(const std::string& filename)
		{
			std::ifstream file(filename, std::ios::ate | std::ios::binary);

			if (!file.is_open())
			{
				return std::vector<char>();
			}

			std::vector<char> buffer(file.tellg());
			file.seekg(0, std::ios::beg);
			file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
			file.close();

			return buffer;
		}

	protected:
		bool do_load() override;
		void do_unload() override;

	public:
		explicit Shader(const std::string& id, Device& device, vk::ShaderStageFlagBits shader_stage);
		~Shader() override;

		const vk::raii::ShaderModule& get_shader_module() const;
		vk::ShaderStageFlagBits get_stage() const;
	};
}
