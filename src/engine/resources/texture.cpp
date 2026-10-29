#include "engine/resources/texture.hpp"

using namespace fluffy_fox;

Texture::Texture(const std::string& id, const std::string& base_dir, ResourceHandle<GltfFile> gltf_file, u32 mesh_idx, u32 primitive_idx) :
	Resource(id),
	base_dir_(base_dir),
	gltf_file_(std::move(gltf_file)),
	mesh_idx_(mesh_idx),
	primitive_idx_(primitive_idx),
	width_(0),
	height_(0),
	channels_(0),
	bytes_(0),
	mip_levels_(0),
	image_(nullptr),
	memory_(nullptr),
	image_view_(nullptr),
	sampler_(nullptr)
{ }

Texture::~Texture()
{
	unload();
}

bool Texture::do_load()
{
	const tg3_model& model = *(*gltf_file_).model().get();
	if (mesh_idx_ >= model.meshes_count) return false;

	const tg3_mesh& mesh = model.meshes[mesh_idx_];
	if (primitive_idx_>= mesh.primitives_count) return false;

	const tg3_primitive& primitive = mesh.primitives[primitive_idx_];

	const tg3_image* image = get_texture_image(model, primitive);
	if (image == nullptr) return false;

	if (!decode_image(model, *image)) return false;

	// FIXME image_, memory_, image_view_ and sampler_ from VulkanTutorial::createTextureImage()

	return true;
}

void Texture::do_unload()
{
	if (!is_loaded()) return;

	gltf_file_ = nullptr;
	image_ = nullptr;
	memory_ = nullptr;
	image_view_ = nullptr;
	sampler_ = nullptr;
}

const tg3_image* Texture::get_texture_image(const tg3_model& model, const tg3_primitive& primitive)
{
	if (primitive.material < 0) return nullptr;

	const tg3_material* material = &model.materials[primitive.material];
	i32 texture_index = material->pbr_metallic_roughness.base_color_texture.index;
	if (texture_index < 0) return nullptr;

	const tg3_texture* texture = &model.textures[texture_index];
	i32 image_index = texture->source;
	if (image_index < 0) return nullptr;

	return &model.images[image_index];
}

bool Texture::decode_image(const tg3_model& model, const tg3_image& image)
{
	const uint8_t* encoding = nullptr;
	u64 encoding_size = 0;
	std::vector<u8> file_bytes;

	if (image.buffer_view >= 0)
	{
		const tg3_buffer_view& buffer_view = model.buffer_views[image.buffer_view];
		const tg3_buffer& buffer = model.buffers[buffer_view.buffer];
		encoding = buffer.data.data + buffer_view.byte_offset;
		encoding_size = (u64)buffer_view.byte_length;
	}
	else if (image.image.count > 0)
	{
		encoding = image.image.data;
		encoding_size = (u64)image.image.count;
	}
	else if (image.uri.len > 0 && !tg3_is_data_uri(image.uri.data, image.uri.len))
	{
		std::string path = base_dir_ + "/" + std::string(image.uri.data, image.uri.len);
		FILE* file = fopen(path.c_str(), "rb");
		if (!file) return false;
		fseek(file, 0, SEEK_END);
		long file_size = ftell(file);
		fseek(file, 0, SEEK_SET);
		file_bytes.resize((size_t)file_size);
		fread(file_bytes.data(), 1, (size_t)file_size, file);
		fclose(file);
		encoding = file_bytes.data();
		encoding_size = file_bytes.size();
	}

	if (!encoding) return false;

	// FIXME is num_components really the same as num_channels
	u8* pixels = stbi_load_from_memory(encoding, (i32)encoding_size, &width_, &height_, &channels_, STBI_rgb_alpha);

	// result.bytes = result.width * result.height * result.channels; // channels should be 4 // TODO put this in struct method
	bytes_ = width_ * height_ * 4; // channels should be 4
	pixels_ = std::make_unique<u8[]>(bytes_);
	memcpy(pixels_.get(), pixels, bytes_);

	stbi_image_free(pixels);

	return true;
}
