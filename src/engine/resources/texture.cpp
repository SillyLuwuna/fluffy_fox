#include "engine/resources/texture.hpp"

using namespace fluffy_fox;

Texture::Texture(const std::string& id) :
	Resource(id),
	width(0),
	height(0),
	channels(0)
{ }

Texture::~Texture()
{
	unload();
}

bool Texture::do_load()
{
	// TODO
	// load file
	// load texture
	// transform data into vulkan resources
	// cleanup cpu resources
	throw std::runtime_error("not implemented");
}

void Texture::do_unload()
{
	if (!is_loaded()) return;

	// TODO
	// get device
	// destroy vk objects
	throw std::runtime_error("not implemented");
}

vk::Image Texture::get_image() const
{
	return image_;
}

vk::ImageView Texture::get_image_view() const
{
	return image_view_;
}

vk::Sampler Texture::get_sampler() const
{
	return sampler_;
}
