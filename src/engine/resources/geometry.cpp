#include "engine/resources/geometry.hpp"

#include <iostream>

using namespace fluffy_fox;

// Geometry::Geometry() :
// 	vertex_buffer_(nullptr),
// 	vertex_count_(0),
// 	index_buffer_(nullptr),
// 	index_count_(0),
// 	is_valid_(false)
// { }

Geometry::Geometry(GltfFile& gltf_file, const tg3_primitive& primitive) :
	vertex_buffer_(nullptr),
	vertex_count_(0),
	index_buffer_(nullptr),
	index_count_(0),
	is_valid_(load(gltf_file, primitive))
{ }

Geometry::Geometry(Geometry&& other) :
	vertex_buffer_(std::move(other.vertex_buffer_)),
	vertex_count_(other.vertex_count_),
	index_buffer_(std::move(other.index_buffer_)),
	index_count_(other.index_count_),
	is_valid_(other.is_valid_)
{ }

Geometry::~Geometry()
{
	vertex_buffer_ = nullptr;
	index_buffer_ = nullptr;
	vertex_count_ = 0;
	index_count_ = 0;
}

bool Geometry::load(GltfFile& gltf_file, const tg3_primitive& primitive)
{
	assert(primitive.mode == TG3_MODE_TRIANGLES || primitive.mode == -1);

	std::vector<glm::vec3> vertices;
	std::vector<u32> indices;
	if (!load_primitive(gltf_file, primitive, vertices, indices)) return false;

	vertex_buffer_ = Buffer(
		vertices,
		vk::BufferUsageFlagBits::eVertexBuffer,
		vk::MemoryPropertyFlagBits::eDeviceLocal
	);
	vertex_count_ = vertices.size();

	index_buffer_ = Buffer(
		indices,
		vk::BufferUsageFlagBits::eIndexBuffer,
		vk::MemoryPropertyFlagBits::eDeviceLocal
	);
	index_count_ = indices.size();

	return true;
}

bool Geometry::load_primitive(
	GltfFile& gltf_file,
	const tg3_primitive& primitive,
	std::vector<glm::vec3>& vertices,
	std::vector<u32>& indices
)
{
	i32 pos_accessor = gltf_file.find_attribute(primitive, "POSITION");
	if (pos_accessor < 0) return false;

	std::vector<f32> positions;
	i32 pos_num_components = 0;

	if (
		!gltf_file.read_accessor<f32>(pos_accessor, positions, pos_num_components) ||
		pos_num_components != 3
	) return false;

	if (!add_vertices(gltf_file, primitive, positions, vertices, indices)) return false;

	std::cout << "loaded primitive\n";
	std::cout << "\tvertices: " << std::to_string(indices.size()) << "\n";
	std::cout << "\tindices:  " << std::to_string(vertices.size()) << "\n";

	return true;
}

bool Geometry::add_vertices(
	GltfFile& gltf_file,
	const tg3_primitive& primitive,
	const std::vector<f32>& positions,
	std::vector<glm::vec3>& vertices,
	std::vector<u32>& indices
)
{
	const u64 vertex_count = positions.size() / 3;
	indices.reserve(vertex_count);

	std::unordered_map<glm::vec3, u32> unique_vertices;
	const bool has_indices = primitive.indices != TG3_INDEX_NONE;

	for (u64 i = 0; i < vertex_count; i++)
	{
		glm::vec3 pos;
		memcpy(&pos, &positions[i * 3], 3 * sizeof(f32));

		// inversion from Y-up to Z-up same would have to be applied to normals
		// FIXME maybe change vulkan itself?
		// maybe check somehow what axis the model uses
		float tmp = pos[1];
		pos[1] = -pos[2];
		pos[2] = tmp;

		if (has_indices)
		{
			vertices.push_back(pos);
			continue;
		}

		auto [it, inserted] = unique_vertices.insert({
			pos, static_cast<u32>(vertices.size())
		});

		if (inserted)
		{
			vertices.push_back(pos);
		}

		indices.push_back(it->second);
	}

	if (has_indices)
	{
		load_vertices(gltf_file, primitive, vertices, indices);
	}

	return true;
}

bool Geometry::load_vertices(
	GltfFile& gltf_file,
	const tg3_primitive& primitive,
	const std::vector<glm::vec3>& vertices,
	std::vector<u32>& indices
)
{
	const u32 base = (u32)vertices.size();

	i32 idx_num_components;
	std::vector<u32> idx;
	if (!gltf_file.read_accessor<u32>(primitive.indices, idx, idx_num_components)) return false;

	for (uint32_t i : idx)
	{
		indices.push_back(base + i);
	}

	return true;
}
