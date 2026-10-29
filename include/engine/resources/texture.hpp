#pragma once

#include "engine/resource_handle.hpp"
#include "engine/resources/gltf_file.hpp"
#include <memory>
#include <vulkan/vulkan.hpp>
#include <fluff/types.hpp>
using namespace fluff::types;

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#define TINYGLTF3_ENABLE_FS
#define TINYGLTF3_IMPLEMENTATION
#include <tiny_gltf_v3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "engine/resource.hpp"

namespace fluffy_fox
{
	class Texture : public Resource
	{
	private:
		std::string base_dir_;
		ResourceHandle<GltfFile> gltf_file_;
		u32 mesh_idx_;
		u32 primitive_idx_;

		std::unique_ptr<u8[]> pixels_;
		i32 width_;
		i32 height_;
		i32 channels_;
		u64 bytes_;

		u32 mip_levels_;

		vk::raii::Image image_;
		vk::raii::DeviceMemory memory_;
		vk::raii::ImageView image_view_;
		vk::raii::Sampler sampler_;

	protected:
		bool do_load() override;
		void do_unload() override;

		const tg3_image* get_texture_image(const tg3_model& model, const tg3_primitive& primitive);
		bool decode_image(const tg3_model& model, const tg3_image& image);

	public:
		explicit Texture(const std::string& id, const std::string& base_dir, ResourceHandle<GltfFile> gltf_file, u32 mesh_idx, u32 primitive_idx);
		~Texture() override;

		inline vk::Image image() const { return image_; }
		inline vk::ImageView image_view() const { return image_view_; }
		inline vk::Sampler sampler() const { return sampler_; }
	};
}
