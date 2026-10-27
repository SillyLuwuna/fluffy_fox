#pragma once

#include "engine/resource_manager.hpp"
#include "engine/resources/shader.hpp"
#include "rendering/device.hpp"
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <fluff/types.hpp>
using namespace fluff::types;

namespace fluffy_fox::rendering
{
	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 uv;

		bool operator==(const Vertex& other) const
		{
			return
				pos == other.pos &&
				color == other.color &&
				uv == other.uv;
		}

		static vk::VertexInputBindingDescription get_binding_description()
		{
			return {
				.binding = 0,
				.stride = sizeof(Vertex),
				.inputRate = vk::VertexInputRate::eVertex
			};
		}

		static std::vector<vk::VertexInputAttributeDescription> get_attribute_descriptions()
		{
			return {
				{
					.location = 0,
					.binding = 0,
					.format = vk::Format::eR32G32B32Sfloat,
					.offset = offsetof(Vertex, pos)
				},
				{
					.location = 1,
					.binding = 0,
					.format = vk::Format::eR32G32B32Sfloat,
					.offset = offsetof(Vertex, color)
				},
				{
					.location = 2,
					.binding = 0,
					.format = vk::Format::eR32G32Sfloat,
					.offset = offsetof(Vertex, uv)
				}
			};
		}

	};
}

namespace std
{
	template<> struct hash<fluffy_fox::rendering::Vertex>
	{
		size_t operator()(fluffy_fox::rendering::Vertex const& vertex) const
		{
			return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.uv) << 1);
		}
	};
}


