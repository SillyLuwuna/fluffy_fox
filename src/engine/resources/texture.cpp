#include "engine/resources/texture.hpp"
#include <cmath>

using namespace fluffy_fox;

Texture::Texture(const std::string& id, const std::string& base_dir, ResourceHandle<GltfFile> gltf_file, u32 mesh_idx, u32 primitive_idx) :
	Resource(id),
	base_dir_(base_dir),
	gltf_file_(std::move(gltf_file)),
	mesh_idx_(mesh_idx),
	primitive_idx_(primitive_idx),
	gpu_image_(GpuImage::empty()),
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

	if (!decode_image(model, *image, image_)) return false;

	mip_levels_ = get_mip_levels(image_.width, image_.height);
	if (!load_uvs(*gltf_file_, primitive, uvs_)) return false;
	gpu_image_ = Gpu::upload_image(image_, mip_levels_);
	sampler_ = create_sampler();

	return true;
}

bool Texture::load_uvs(
	GltfFile& gltf_file,
	const tg3_primitive& primitive,
	std::vector<glm::vec2>& out_uvs
)
{
	i32 uv_accessor = gltf_file.find_attribute(primitive, "TEXCOORD_0");
	if (uv_accessor < 0) return false;

	std::vector<f32> uv_data;
	i32 num_components = 0;

	if (!gltf_file.read_accessor<f32>(uv_accessor, uv_data, num_components) ||
		num_components != 2
	) return false;

	if (!add_uvs(gltf_file, primitive, uv_data, out_uvs)) return false;

	return true;
}

bool Texture::add_uvs(
	GltfFile& gltf_file,
	const tg3_primitive& primitive,
	const std::vector<f32>& uv_data,
	std::vector<glm::vec2>& out_uvs
)
{
	const u64 uv_count = uv_data.size() / 2;
	out_uvs.reserve(uv_count);

	for (u64 i = 0; i < uv_count; i++)
	{
		glm::vec2 uv;
		memcpy(&uv, &uv_data[i * 2], 2 * sizeof(f32));

		out_uvs.push_back(uv);
	}

	return true;
}

u32 Texture::get_mip_levels(i32 width, i32 height) const
{
	return static_cast<u32>(std::floor(std::log2(std::max(width, height)))) + 1;
}

vk::raii::Sampler Texture::create_sampler() const
{
	vk::PhysicalDeviceProperties properties = Device::instance().physical().getProperties();

	vk::SamplerCreateInfo sampler_info {
		.magFilter = vk::Filter::eLinear, // when close to camera, magnify
		.minFilter = vk::Filter::eLinear, // when far away from camera, minify
		.mipmapMode = vk::SamplerMipmapMode::eLinear, // eNearest to select nearest mipmap, eLinear to interpolate between the two with the Filters
		.addressModeU = vk::SamplerAddressMode::eRepeat,
		.addressModeV = vk::SamplerAddressMode::eRepeat,
		.addressModeW = vk::SamplerAddressMode::eRepeat,
		.mipLodBias = 0.0f, // bias for mipmap selection
		.anisotropyEnable = true,
		.maxAnisotropy = properties.limits.maxSamplerAnisotropy, // PERF slow, could select a lower value
		.compareEnable = false,
		.compareOp = vk::CompareOp::eAlways,
		.minLod = 0.0f,
		.maxLod = vk::LodClampNone, // 0.0f to disable
		.borderColor = vk::BorderColor::eIntOpaqueBlack,
		.unnormalizedCoordinates = false,
	};

	return vk::raii::Sampler(Device::instance().logical(), sampler_info);
}

void Texture::do_unload()
{
	if (!is_loaded()) return;

	gltf_file_ = nullptr;
	image_ = nullptr;
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

bool Texture::decode_image(const tg3_model& model, const tg3_image& image, Image& out_image)
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
	u8* pixels = stbi_load_from_memory(
		encoding,
		(i32)encoding_size,
		&out_image.width,
		&out_image.height,
		&out_image.channels,
		STBI_rgb_alpha
	);

	out_image.bytes = out_image.width * out_image.height * 4; // channels should be 4
	out_image.pixels = std::make_unique<u8[]>(out_image.bytes);
	memcpy(out_image.pixels.get(), pixels, out_image.bytes);

	stbi_image_free(pixels);

	return true;
}
