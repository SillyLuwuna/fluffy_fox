#pragma once

#include "engine/resources/primitive.hpp"
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#define TINYGLTF3_ENABLE_FS
#define TINYGLTF3_IMPLEMENTATION
#include <tiny_gltf_v3.h>

#include <fluff/types.hpp>

using namespace fluff::types;

#include "engine/resource.hpp"

namespace fluffy_fox
{
	class Mesh : public Resource
	{
	private:
		std::vector<Primitive> primitives_;
	
	protected:
		bool do_load() override;
		void do_unload() override;

		void print_error(tinygltf3::ErrorStack& errors);

	public:
		explicit Mesh(const std::string& id);
		~Mesh() override;
	};
}

