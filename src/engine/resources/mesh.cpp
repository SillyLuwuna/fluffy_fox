#include "engine/resources/mesh.hpp"

#include "rendering/vertex.hpp"
#include <iostream>

using namespace fluffy_fox;


Mesh::Mesh(const std::string& id) :
	Resource(id)
{ }

Mesh::~Mesh()
{
	unload();
}

bool Mesh::do_load()
{
	GltfReader gltf_reader;
	if (!gltf_reader.load(get_id())) return false;

	const tg3_model& model = *gltf_reader.model().get();

	for (u32 mesh_idx = 0; mesh_idx < model.meshes_count; mesh_idx++)
	{
		const tg3_mesh& mesh = model.meshes[mesh_idx];
		for (u32 primitive_idx = 0; primitive_idx < mesh.primitives_count; primitive_idx++)
		{
			Primitive primitive = 
		}
	}
}


void Mesh::do_unload()
{
	if (!is_loaded()) return;

	// TODO
	// get device
	// destroy vk objects
	throw std::runtime_error("not implemented");
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
