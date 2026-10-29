#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#define TINYGLTF3_ENABLE_FS
#define TINYGLTF3_IMPLEMENTATION
#include <tiny_gltf_v3.h>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <fluff/types.hpp>

using namespace fluff::types;

#include "engine/resource.hpp"
#include "rendering/buffer.hpp"
#include "rendering/gltf_reader.hpp"

using namespace fluffy_fox::rendering;

namespace fluffy_fox
{
	class Primitive
	{
	private:
		const tg3_primitive& primitive_;

		Buffer vertex_buffer_;
		u32 vertex_count_;

		Buffer index_buffer_;
		u32 index_count_;
	
		static bool load_primitive(
			GltfReader& gltf_reader,
			const tg3_primitive& primitive,
			std::vector<glm::vec3>& vertices,
			std::vector<u32>& indices
		);

		static bool add_vertices(
			GltfReader& gltf_reader,
			const tg3_primitive& primitive,
			const std::vector<f32>& positions,
			std::vector<glm::vec3>& vertices,
			std::vector<u32>& indices
		);

		static bool load_vertices(
			GltfReader& gltf_reader,
			const tg3_primitive& primitive,
			const std::vector<glm::vec3>& vertices,
			std::vector<u32>& indices
		);

	public:
		explicit Primitive(const std::string& id, u32 mesh_idx, u32 primitive_idx);
		~Primitive();

		inline const vk::Buffer vertex_buffer() const { return vertex_buffer_.buffer(); }
		inline const vk::Buffer index_buffer() const { return index_buffer_.buffer(); }
		inline u32 vertex_count() const { return vertex_count_; }
		inline u32 index_count() const { return index_count_; }
	};
}

