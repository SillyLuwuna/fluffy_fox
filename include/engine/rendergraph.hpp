// #pragma once
//
// #include "engine/resource.hpp"
// #include <functional>
// #include <string>
// #define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
// #include <vulkan/vulkan_raii.hpp>
// #include <fluff/types.hpp>
//
// using namespace fluff::types;
//
// namespace fluffy_fox
// {
// 	class Rendergraph
// 	{
// 	private:
// 		struct RenderResource
// 		{
// 			std::string name;
// 			vk::Format format;
// 			vk::Extent2D extent;
// 			vk::ImageUsageFlags usage;
// 			vk::ImageLayout initial_layout;
// 			vk::ImageLayout final_layout;
//
// 			vk::raii::Image image;
// 			vk::raii::DeviceMemory memory;
// 			vk::raii::ImageView view;
//
// 			RenderResource() :
// 				image(nullptr),
// 				memory(nullptr),
// 				view(nullptr)
// 			{ }
// 		};
//
// 		struct Pass
// 		{
// 			std::string name;
// 			std::vector<std::string> inputs; // resources this pass reads from
// 			std::vector<std::string> outputs; // resources this pass writes to
// 			std::function<void(vk::raii::CommandBuffer&)> execute_func; // the actual rendering code
// 		};
//
// 		std::unordered_map<std::string, RenderResource> resources_;
// 		std::vector<Pass> passes_;
// 		std::vector<u64> execution_order_;
//
// 		std::vector<vk::raii::Semaphore> semaphores_;
// 		std::vector<std::pair<u64, u64>> semaphore_signal_wait_pairs_;
//
// 		vk::raii::Device& device_;
//
// 		u32 find_memory_type(u32 type_filter, vk::MemoryPropertyFlags properties);
//
// 	public:
// 		explicit Rendergraph(vk::raii::Device& device);
//
// 		void add_resource(
// 			const std::string& name,
// 			vk::Format format,
// 			vk::Extent2D extent,
// 			vk::ImageUsageFlags usage,
// 			vk::ImageLayout initial_layout,
// 			vk::ImageLayout final_layout
// 		);
//
// 		void add_pass(
// 			const std::string& name,
// 			const std::vector<std::string>& inputs,
// 			const std::vector<std::string>& outputs,
// 			std::function<void(vk::raii::CommandBuffer&)> execute_function
// 		);
//
// 		void compile();
// 		void execute(vk::raii::CommandBuffer& command_buffer, vk::Queue queue);
// 		void render_frame(vk::raii::Device& device, vk::Queue graphics_queue, vk::Queue present_queue);
//
// 		Resource* get_resource(const std::string& name);
// 	};
// }
