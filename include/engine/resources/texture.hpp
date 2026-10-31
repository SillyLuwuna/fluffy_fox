#pragma once

#include <glm/glm.hpp>
#include "engine/resource_handle.hpp"
#include "engine/resources/gltf_file.hpp"
#include "rendering/gpu.hpp"
#include "rendering/image.hpp"
#include <memory>
#include <vulkan/vulkan.hpp>
#include <fluff/types.hpp>
using namespace fluff::types;

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include <tiny_gltf_v3.h>
#include <stb_image.h>

#include "engine/resource.hpp"

namespace fluffy_fox
{
	using namespace fluffy_fox::rendering;

	class Texture : public Resource
	{
	private:
		std::string base_dir_;
		ResourceHandle<GltfFile> gltf_file_;
		u32 mesh_idx_;
		u32 primitive_idx_;

		Image image_; // TODO technically not needed as a member variable

		u32 mip_levels_;

		std::vector<glm::vec2> uvs_;
		GpuImage gpu_image_;
		vk::raii::Sampler sampler_;

		const tg3_image* get_texture_image(const tg3_model& model, const tg3_primitive& primitive);
		bool decode_image(const tg3_model& model, const tg3_image& image, Image& out_image);
		u32 get_mip_levels(i32 width, i32 height) const;
		vk::raii::Sampler create_sampler() const;

		bool load_uvs(
			GltfFile& gltf_file,
			const tg3_primitive& primitive,
			std::vector<glm::vec2>& out_uvs
		);

		bool add_uvs(
			const std::vector<f32>& uv_data,
			std::vector<glm::vec2>& out_uvs
		);

	protected:
		bool do_load() override;
		void do_unload() override;

	public:
		explicit Texture(const std::string& id, const std::string& base_dir, ResourceHandle<GltfFile> gltf_file, u32 mesh_idx, u32 primitive_idx);
		~Texture() override;

		inline const GpuImage& image() const { return gpu_image_; }
		inline vk::Sampler sampler() const { return sampler_; }
		inline const std::vector<glm::vec2>& uvs() const { return uvs_; }
	};
}
