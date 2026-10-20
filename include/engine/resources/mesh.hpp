#pragma once

#include <vulkan/vulkan.hpp>
#include <fluff/types.hpp>

using namespace fluff::types;

#include "engine/resource.hpp"

namespace fluffy_fox
{
	class Mesh : public Resource
	{
	private:
		// TODO change everything to RAII, and initialize with nullptr
		vk::Buffer vertex_buffer_;
		vk::DeviceMemory vertex_buffer_memory_;
		vk::DeviceSize vertex_buffer_offset_;
		u32 vertex_count_;

		vk::Buffer index_buffer_;
		vk::DeviceMemory index_buffer_memory_;
		vk::DeviceSize index_buffer_offset_;
		u32 index_count_;
	
	protected:
		bool do_load() override;
		void do_unload() override;

	public:
		explicit Mesh(const std::string& id);
		~Mesh() override;

		vk::Buffer get_vertex_buffer() const;
		vk::Buffer get_index_buffer() const;
		u32 get_vertex_count() const;
		u32 get_index_count() const;
	};
}

