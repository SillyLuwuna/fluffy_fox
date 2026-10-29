#pragma once

#include <vector>
#define TINYGLTF3_ENABLE_FS
#define TINYGLTF3_IMPLEMENTATION
#include <tiny_gltf_v3.h>

#include <fluff/types.hpp>
using namespace fluff::types;

#include <string>


namespace fluffy_fox::rendering
{

class GltfReader
{
private:
	tinygltf3::Model model_;
	bool loaded_;

	void print_error(tinygltf3::ErrorStack& errors);

	template <typename T>
	bool read_accessor_buffer(
		std::vector<T>& out,
		const tg3_accessor& accessor,
		u32 component_size,
		u64 num_components,
		bool is_accessor_normalized
	)
	{
		if ((u32)accessor.buffer_view >= model_->buffer_views_count) return false;
		const tg3_buffer_view& buffer_view = model_->buffer_views[accessor.buffer_view];
		i32 stride = tg3_accessor_byte_stride(&accessor, &buffer_view);
		if (stride <= 0) return false;

		const u8* src = get_data_view(
			accessor.buffer_view,
			accessor.byte_offset, // FIXME does it need this offset? already applied in function
			accessor.count,
			stride,
			(u64)num_components * component_size
		);
		if (!src && accessor.count) return false;

		for (u64 i = 0; i < accessor.count; i++)
		{
			for (int c = 0; c < num_components; c++)
			{
				out[i * num_components + c] = read_component<T>(
					src + i * stride + c * component_size,
					accessor.component_type,
					is_accessor_normalized
				);
			}
		}
	}

	const u8* get_data_view(
		u64 buffer_view_idx,
		u64 offset,
		u64 count,
		u64 stride,
		u64 element_size
	)
	{
		if (buffer_view_idx < 0 || (uint32_t)buffer_view_idx >= model_->buffer_views_count) return nullptr;

		const tg3_buffer_view& buffer_view = model_->buffer_views[buffer_view_idx];
		if (buffer_view.buffer < 0 || (uint32_t)buffer_view.buffer >= model_->buffers_count) return nullptr;

		const tg3_buffer& buffer = model_->buffers[buffer_view.buffer];
		if (!buffer.data.data) return nullptr;
		if (count == 0) return buffer.data.data;

		uint64_t start = buffer_view.byte_offset + offset;
		uint64_t end = start + (count - 1) * stride + element_size;
		if (end > buffer.data.count) return nullptr;

		return buffer.data.data + start;
	}

	template <typename T>
	void assert_accessor_type()
	{
		constexpr bool is_type =
			std::is_same_v<T, f32> ||
			std::is_same_v<T, f64> ||
			std::is_same_v<T, u8> ||
			std::is_same_v<T, i8> ||
			std::is_same_v<T, u16> ||
			std::is_same_v<T, i16> ||
			std::is_same_v<T, u32> ||
			std::is_same_v<T, i32>;

		static_assert(is_type);
	}

	template <typename T>
	T read_component(const u8* ptr, i32 component_type, bool is_normalized)
	{
		switch (component_type)
		{
			case TG3_COMPONENT_TYPE_FLOAT:
				{
					f32 v;
					memcpy(&v, ptr, 4);
					return (T)v;
				}
			case TG3_COMPONENT_TYPE_DOUBLE:
				{
					f64 v;
					memcpy(&v, ptr, 8);
					return (T)v;
				}
			case TG3_COMPONENT_TYPE_UNSIGNED_BYTE:
				{
					return (T)(is_normalized ? *ptr / 255.0f : *ptr);
				}
			case TG3_COMPONENT_TYPE_BYTE:
				{
					i8 v = (int8_t)*ptr;
					return (T)(is_normalized ? std::max(v / 127.0f, -1.0f) : v);
				}
			case TG3_COMPONENT_TYPE_UNSIGNED_SHORT:
				{
					u16 v;
					memcpy(&v, ptr, 2);
					return (T)(is_normalized ? v / 65535.0f : v);
				}
			case TG3_COMPONENT_TYPE_SHORT:
				{
					i16 v;
					memcpy(&v, ptr, 2);
					return (T)(is_normalized ? std::max(v / 32767.0f, -1.0f) : v);
				}
			case TG3_COMPONENT_TYPE_UNSIGNED_INT:
				{
					u32 v;
					memcpy(&v, ptr, 4);
					return (T)v;
				}
			case TG3_COMPONENT_TYPE_INT:
				{
					i32 v;
					memcpy(&v, ptr, 4);
					return (T)v;
				}
		}

		return (T)0.0f;
	}

public:
	GltfReader();
	~GltfReader();

	bool load(const std::string& file);
	inline const tinygltf3::Model& model() const { return model_; }

	static i32 find_attribute(const tg3_primitive& primitive, const char* name);

	template <typename T>
	bool read_accessor(
		i32 accessor_idx,
		std::vector<T>& out,
		int& out_num_components
	)
	{
		assert_accessor_type<T>();
		if (accessor_idx < 0 || (u32)accessor_idx >= model_->accessors_count) return false;
		const tg3_accessor& accessor = model_->accessors[accessor_idx];

		out_num_components = tg3_num_components(accessor.type);
		i32 component_size = tg3_component_size(accessor.component_type);
		if (out_num_components <= 0 || component_size <= 0) return false;

		out.assign((u64)accessor.count * component_size, 0.0f);
		const bool is_accessor_normalized = accessor.normalized != 0;

		if (accessor.buffer_view <= 0) return false;
		read_accessor_buffer<T>(out, accessor, component_size, out_num_components, is_accessor_normalized);

		return true;
	}

};

}
