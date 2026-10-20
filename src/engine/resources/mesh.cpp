#include "engine/resources/mesh.hpp"

using namespace fluffy_fox;


Mesh::Mesh(const std::string& id) :
	Resource(id),
	vertex_count_(0),
	index_count_(0)
{ }

Mesh::~Mesh()
{
	unload();
}

bool Mesh::do_load()
{
	// TODO
	// load file
	// load mesh
	// transform data into vulkan resources
	// cleanup cpu resources
	throw std::runtime_error("not implemented");
}


void Mesh::do_unload()
{
	if (!is_loaded()) return;

	// TODO
	// get device
	// destroy vk objects
	throw std::runtime_error("not implemented");
}

vk::Buffer Mesh::get_vertex_buffer() const
{
	return vertex_buffer_;
}

vk::Buffer Mesh::get_index_buffer() const
{
	return index_buffer_;
}

u32 Mesh::get_vertex_count() const
{
	return vertex_count_;
}

u32 Mesh::get_index_count() const
{
	return index_count_;
}
