#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include <fluff/types.hpp>
using namespace fluff::types;


namespace fluffy_fox::rendering
{
	struct Image
	{
		std::unique_ptr<u8[]> pixels;
		i32 width;
		i32 height;
		i32 channels;
		u64 bytes;

		inline Image& operator=(std::nullptr_t)
		{
			pixels = nullptr;
			return *this;
		}
	};
}
