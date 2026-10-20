#pragma once

#include <vulkan/vulkan.hpp>
#include <fluff/types.hpp>
using namespace fluff::types;

#include "engine/resource.hpp"

namespace fluffy_fox
{
	class Texture : public Resource
	{
	private:
		// TODO change everything to RAII, and initialize with nullptr
		vk::Image image_;
		vk::DeviceMemory memory_;
		vk::DeviceSize offset_;
		vk::ImageView image_view_;
		vk::Sampler sampler_;

		i32 width;
		i32 height;
		i32 channels;

	protected:
		bool do_load() override;
		void do_unload() override;

	public:
		explicit Texture(const std::string& id);
		~Texture() override;

		vk::Image get_image() const;
		vk::ImageView get_image_view() const;
		vk::Sampler get_sampler() const;
	};
}
