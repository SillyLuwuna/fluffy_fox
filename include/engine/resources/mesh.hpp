#pragma once

#include "engine/resources/geometry.hpp"
#include "engine/resources/texture.hpp"
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#define TINYGLTF3_ENABLE_FS
#define TINYGLTF3_IMPLEMENTATION
#include <tiny_gltf_v3.h>

#include <fluff/types.hpp>

using namespace fluff::types;

#include "engine/resource.hpp"

namespace fluffy_fox
{
	// FIXME
	// target topology (each primitive should be a draw call):
	// Mesh
	// |-- VertexBuffer (shared)
	// |-- IndexBuffer (shared)
	// |-- Primitives[]
	//     |-- indexOffset
	//     |-- indexCount
	//     |-- Material* -> Material
	//                      |-- Shader/Pipeline (?)
	//                      |-- Textures
	//                      |-- Parameters (albedo, normal, ...)

	class Mesh : public Resource
	{
	private:
		ResourceManager& resource_manager_;
		ResourceHandle<GltfFile> gltf_file_;
		u32 mesh_idx_;
		std::vector<Geometry> geometry_;
		std::vector<ResourceHandle<Texture>> textures_;
	
	protected:
		bool do_load() override;
		void do_unload() override;

		void print_error(tinygltf3::ErrorStack& errors);

	public:
		explicit Mesh(
			const std::string& id,
			ResourceManager& resource_manager,
			ResourceHandle<GltfFile> gltf_file,
			u32 mesh_idx
		);

		~Mesh() override;
	};
}

