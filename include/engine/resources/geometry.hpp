#pragma once

#include "engine/resource_handle.hpp"
#include "engine/resources/gltf_file.hpp"
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include <tiny_gltf_v3.h>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <fluff/types.hpp>

using namespace fluff::types;

#include "engine/resource.hpp"
#include "rendering/gpu_buffer.hpp"

using namespace fluffy_fox::rendering;

namespace fluffy_fox
{
	class Geometry
	{
	private:
		// GpuBuffer vertex_buffer_;
		std::vector<glm::vec3> vertices_;
		u32 vertex_count_;

		GpuBuffer index_buffer_;
		u32 index_count_;

		bool is_valid_;

		bool load(GltfFile& gltf_file, const tg3_primitive& primitive);

		static bool load_primitive(
			GltfFile& gltf_file,
			const tg3_primitive& primitive,
			std::vector<glm::vec3>& vertices,
			std::vector<u32>& indices
		);

		static bool add_vertices(
			GltfFile& gltf_file,
			const tg3_primitive& primitive,
			const std::vector<f32>& positions,
			std::vector<glm::vec3>& vertices,
			std::vector<u32>& indices
		);

		static bool load_vertices(
			GltfFile& gltf_file,
			const tg3_primitive& primitive,
			const std::vector<glm::vec3>& vertices,
			std::vector<u32>& indices
		);

	public:
		Geometry(GltfFile& gltf_file, const tg3_primitive& primitive);
		Geometry(Geometry&& other);
		~Geometry();

		inline bool is_valid() const { return is_valid_; }
		// inline const vk::Buffer vertex_buffer() const { return vertex_buffer_.buffer; }
		inline const std::vector<glm::vec3>& vertices() const { return vertices_; }
		inline const vk::Buffer index_buffer() const { return index_buffer_.buffer; }
		inline u32 vertex_count() const { return vertex_count_; }
		inline u32 index_count() const { return index_count_; }
	};
}

