#include "engine/resources/mesh.hpp"

#include <iostream>

using namespace fluffy_fox;

Mesh::Mesh(
	const std::string& id,
	ResourceManager& resource_manager,
	ResourceHandle<GltfFile> gltf_file,
	u32 mesh_idx
) :
	Resource(id),
	resource_manager_(resource_manager),
	gltf_file_(std::move(gltf_file)),
	mesh_idx_(mesh_idx)
{ }

Mesh::~Mesh()
{
	unload();
}

bool Mesh::do_load()
{
	const tg3_model& model = *(*gltf_file_).model().get();

	if (model.meshes_count <= 0) return false;

	const tg3_mesh& mesh = model.meshes[mesh_idx_];
	geometry_.reserve(mesh.primitives_count);
	textures_.reserve(mesh.primitives_count);

	for (u32 primitive_idx = 0; primitive_idx < mesh.primitives_count; primitive_idx++)
	{
		const tg3_primitive& mesh_primitive = mesh.primitives[primitive_idx];

		Geometry geometry(*gltf_file_, mesh_primitive);
		if (!geometry.is_valid()) return false;

		geometry_.emplace_back(std::move(geometry));

		const std::string idx = get_id() + "#texture/" + std::to_string(primitive_idx);
		const std::string base_dir = "models";
		ResourceHandle<Texture> texture = resource_manager_.load<Texture>(
			idx,
			base_dir,
			gltf_file_,
			mesh_idx_,
			primitive_idx
		);

		textures_.emplace_back(std::move(texture));
	}

	return true;
}


void Mesh::do_unload()
{
	if (!is_loaded()) return;

	geometry_.clear();
}

void Mesh::print_error(tinygltf3::ErrorStack& errors)
{
	std::string error;
	for (uint32_t i = 0; i < errors.count(); i++)
	{
		error += "[" + std::to_string((int)errors.entry(i)->severity) + "] ";
		error += errors.entry(i)->message ? errors.entry(i)->message : "(null)";
	}
	std::cout << error << "\n";
}
