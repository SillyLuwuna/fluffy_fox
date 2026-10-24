// #include "engine/rendergraph.hpp"
//
// using namespace fluffy_fox;
//
// Rendergraph::Rendergraph(vk::raii::Device& device) :
// 	device_(device)
// { }
//
// void Rendergraph::add_resource(
// 	const std::string& name,
// 	vk::Format format,
// 	vk::Extent2D extent,
// 	vk::ImageUsageFlags usage,
// 	vk::ImageLayout initial_layout,
// 	vk::ImageLayout final_layout
// )
// {
// 	RenderResource resource;
// 	resource.name = name;
// 	resource.format = format;
// 	resource.extent = extent;
// 	resource.usage = usage;
// 	resource.initial_layout = initial_layout;
// 	resource.final_layout = final_layout;
//
// 	resources_[name] = std::move(resource);
// }
//
// void Rendergraph::add_pass(
// 	const std::string& name,
// 	const std::vector<std::string>& inputs,
// 	const std::vector<std::string>& outputs,
// 	std::function<void(vk::raii::CommandBuffer&)> execute_function
// )
// {
// 	Pass pass;
// 	pass.name = name;
// 	pass.inputs = inputs;
// 	pass.outputs = outputs;
// 	pass.execute_func = execute_function;
//
// 	passes_.emplace_back(pass);
// }
//
// void Rendergraph::compile()
// {
// 	std::vector<std::vector<u64>> dependencies(passes_.size());
// 	std::vector<std::vector<u64>> dependents(passes_.size());
//
// 	std::unordered_map<std::string, u64> resource_writers;
//
// 	// resource production registration
// 	for (u64 i = 0; i < passes_.size(); i++)
// 	{
// 		for (const auto& output : passes_[i].outputs)
// 		{
// 			resource_writers[output] = i; // record pass as producer
// 		}
// 	}
//
// 	// dependency discovery through resource usage analysis
// 	for (u64 i = 0; i < passes_.size(); i++)
// 	{
// 		const auto& pass = passes_[i];
//
// 		for (const auto& input : pass.inputs)
// 		{
// 			auto it = resource_writers.find(input);
// 			if (it != resource_writers.end())
// 			{
// 				dependencies[i].push_back(it->second);
// 				dependents[it->second].push_back(i);
// 			}
// 		}
// 	}
//
// 	// topological sort for optimal execution order
// 	std::vector<bool> visited(passes_.size(), false);
// 	std::vector<bool> in_stack(passes_.size(), false);
//
// 	// TODO change this to an actual static function?
// 	std::function<void(u64)> visit = [&](u64 node) {
// 		if (in_stack[node])
// 		{
// 			throw std::runtime_error("cycle detected in rendergraph");
// 		}
//
// 		if (visited[node])
// 		{
// 			return; // already processed this node and it's dependencies
// 		}
//
// 		in_stack[node] = true;
//
// 		for (auto dependency : dependencies[node])
// 		{
// 			visit(dependency);
// 		}
//
// 		in_stack[node] = false;
// 		visited[node] = true;
// 		execution_order_.push_back(node);
// 	};
//
// 	// process all unvisited nodes to handle disconnected graph components
// 	for (u64 i = 0; i < passes_.size(); i++) 
// 	{
// 		if (!visited[i])
// 		{
// 			visit(i);
// 		}
// 	}
//
// 	// automatic synchronization object creation
// 	for (u64 i = 0; i < passes_.size(); i++)
// 	{
// 		for (auto dep : dependencies[i])
// 		{
// 			semaphores_.emplace_back(device_.createSemaphore({}));
// 			semaphore_signal_wait_pairs_.emplace_back(dep, i); // (producer, consumer) pair
// 		}
// 	}
//
// 	// physical resource allocation and creation
// 	for (auto& [name, resource] : resources_)
// 	{
// 		vk::ImageCreateInfo image_info {
// 			.imageType = vk::ImageType::e2D,
// 			.format = resource.format,
// 			.extent = {
// 				resource.extent.width,
// 				resource.extent.height,
// 				1
// 			},
// 			.mipLevels = 1, // FIXME
// 			.arrayLayers = 1,
// 			.samples = vk::SampleCountFlagBits::e1,
// 			.tiling = vk::ImageTiling::eOptimal,
// 			.usage = resource.usage,
// 			.sharingMode = vk::SharingMode::eExclusive,
// 			.initialLayout = vk::ImageLayout::eUndefined
// 		};
//
// 		resource.image = device_.createImage(image_info);
//
// 		// allocate memory for the image
// 		vk::MemoryRequirements mem_requirements = resource.image.getMemoryRequirements();
//
// 		vk::MemoryAllocateInfo alloc_info {
// 			.allocationSize = mem_requirements.size,
// 			.memoryTypeIndex = find_memory_type(
// 				mem_requirements.memoryTypeBits,
// 				vk::MemoryPropertyFlagBits::eDeviceLocal
// 			),
// 		};
//
// 		resource.memory = device_.allocateMemory(alloc_info);
// 		resource.image.bindMemory(resource.memory, 0);
//
// 		// create image view for shader access
// 		vk::ImageViewCreateInfo view_info {
// 			.image = resource.image,
// 			.viewType = vk::ImageViewType::e2D,
// 			.format = resource.format,
// 			.subresourceRange = {
// 				.aspectMask = vk::ImageAspectFlagBits::eColor,
// 				.baseMipLevel = 0, // FIXME
// 				.levelCount = 1,
// 				.baseArrayLayer = 0,
// 				.layerCount = 1
// 			},
// 		};
//
// 		resource.view = device_.createImageView(view_info);
// 	}
// }
// void Rendergraph::execute(vk::raii::CommandBuffer& command_buffer, vk::Queue queue)
// {
// 	// execution state management for dynamic synchronization
// 	std::vector<vk::CommandBuffer> command_buffers;
// 	std::vector<vk::Semaphore> wait_semaphores;
// 	std::vector<vk::PipelineStageFlags> wait_stages;
// 	std::vector<vk::Semaphore> signal_semaphores;
//
// 	// ordered pass execution with automatic dependency management
// 	// execute each pass in the computed dependency-safe order
// 	for (auto pass_idx : execution_order_)
// 	{
// 		const auto& pass = passes_[pass_idx];
//
// 		// synchronization setup - collect dependencies for current pass
// 		// determine what this pass must wait for before executing
// 		wait_semaphores.clear();
// 		wait_stages.clear();
//
// 		for (u64 i = 0; i < semaphore_signal_wait_pairs_.size(); i++)
// 		{
// 			if (semaphore_signal_wait_pairs_[i].second == pass_idx)
// 			{
// 				wait_semaphores.push_back(*semaphores_[i]);
// 				wait_stages.push_back(vk::PipelineStageFlagBits::eColorAttachmentOutput);
// 			}
// 		}
//
// 		// collect semaphores that this pass will signal for dependent passes
// 		signal_semaphores.clear();
// 		for (u64 i = 0; i < semaphore_signal_wait_pairs_.size(); i++)
// 		{
// 			if (semaphore_signal_wait_pairs_[i].first == pass_idx)
// 			{
// 				signal_semaphores.push_back(semaphores_[i]);
// 			}
// 		}
//
// 		// command buffer preparation and resource layout transitions
// 		// set up command recording and transition resources to appropriate layouts
// 		command_buffer.begin({});
//
// 		// transition input resources to shader-reader layouts
// 		for (const auto& input : pass.inputs)
// 		{
// 			auto& resource = resources_[input];
//
// 			vk::ImageMemoryBarrier barrier {
// 				.srcAccessMask = vk::AccessFlagBits::eMemoryWrite,
// 				.dstAccessMask = vk::AccessFlagBits::eShaderRead,
// 				.oldLayout = resource.initial_layout,
// 				.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
// 				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
// 				.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
// 				.image = resource.image,
// 				.subresourceRange = {
// 					.aspectMask = vk::ImageAspectFlagBits::eColor,
// 					.baseMipLevel = 0, // FIXME
// 					.levelCount = 1,
// 					.baseArrayLayer = 0,
// 					.layerCount = 1
// 				}
// 			};
//
// 			command_buffer.pipelineBarrier(
// 				vk::PipelineStageFlagBits::eAllCommands,
// 				vk::PipelineStageFlagBits::eFragmentShader,
// 				vk::DependencyFlagBits::eByRegion,
// 				{}, {}, barrier
// 			);
// 		}
//
// 		// transition output resources to render target layouts
// 		for (const auto& output : pass.outputs)
// 		{
// 			auto& resource = resources_[output];
// 			
// 			// TODO repetitive, make a function for this?
// 			vk::ImageMemoryBarrier barrier {
// 				.srcAccessMask = vk::AccessFlagBits::eMemoryRead,
// 				.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
// 				.oldLayout = resource.initial_layout,
// 				.newLayout = vk::ImageLayout::eColorAttachmentOptimal,
// 				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
// 				.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
// 				.image = resource.image,
// 				.subresourceRange = {
// 					.aspectMask = vk::ImageAspectFlagBits::eColor,
// 					.baseMipLevel = 0, // FIXME
// 					.levelCount = 1,
// 					.baseArrayLayer = 0,
// 					.layerCount = 1
// 				}
// 			};
//
// 			command_buffer.pipelineBarrier(
// 				vk::PipelineStageFlagBits::eAllCommands,
// 				vk::PipelineStageFlagBits::eColorAttachmentOutput,
// 				vk::DependencyFlagBits::eByRegion,
// 				{}, {}, barrier
// 			);
// 		}
//
// 		// pass execution - execute the actual rendering logic
// 		// call the user-provided rendering function
// 		pass.execute_func(command_buffer);
//
// 		// final layout transitions - prepare resources for subsequent use
// 		// transition output resources to their final required layouts
// 		for (const auto& output : pass.outputs)
// 		{
// 			auto& resource = resources_[output];
//
// 			vk::ImageMemoryBarrier barrier {
// 				.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
// 				.dstAccessMask = vk::AccessFlagBits::eMemoryRead,
// 				.oldLayout = vk::ImageLayout::eColorAttachmentOptimal,
// 				.newLayout = resource.final_layout,
// 				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
// 				.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
// 				.image = resource.image,
// 				.subresourceRange = {
// 					.aspectMask = vk::ImageAspectFlagBits::eColor,
// 					.baseMipLevel = 0, // FIXME
// 					.levelCount = 1,
// 					.baseArrayLayer = 0,
// 					.layerCount = 1
// 				}
// 			};
//
// 			command_buffer.pipelineBarrier(
// 				vk::PipelineStageFlagBits::eColorAttachmentOutput,
// 				vk::PipelineStageFlagBits::eAllCommands,
// 				vk::DependencyFlagBits::eByRegion,
// 				{}, {}, barrier
// 			);
// 		}
//
// 		// command submission and synchronization
// 		// submit command buffer with appropriate dependency and signaling semaphores
// 		command_buffer.end();
//
// 		vk::SubmitInfo submit_info {
// 			.waitSemaphoreCount = static_cast<u32>(wait_semaphores.size()),
// 			.pWaitSemaphores = wait_semaphores.data(),
// 			.pWaitDstStageMask = wait_stages.data(),
// 			.commandBufferCount = 1,
// 			.pCommandBuffers = &*command_buffer,
// 			.signalSemaphoreCount = static_cast<u32>(signal_semaphores.size()),
// 			.pSignalSemaphores = signal_semaphores.data()
// 		};
//
// 		// FIXME change to raii queue? would change this function call
// 		// FIXME ignoring return
// 		queue.submit(1, &submit_info, nullptr);
//
// 		// FIXME waitidle()?
// 	}
// }
//
// u32 Rendergraph::find_memory_type(u32 type_filter, vk::MemoryPropertyFlags properties)
// {
// 	// TODO
// }
//
// void Rendergraph::render_frame(vk::raii::Device& device, vk::Queue graphics_queue, vk::Queue present_queue)
// {
// 	// FIXME change function to different class
//
// 	// synchronize with previous frame completion
// 	// prevent cpu from submitting work faster than GPU can process it
// 	vk::Result result = device.waitForFences(1, &*in_flight_fence, VK_TRUE, UINT64_MAX); // TODO change to std::limits
//
// 	// reset fence for this frame's completion tracking
// 	// prepare the fence to signal when this frame's GPU work completes
// 	device_.resetFences(1, &*in_flight_fence);
//
// 	// acquire next available image from the swapchain
// 	// this operation coordinates with the presentation engine and display system
// 	u32 image_index;
// 	result = device.acquireNextImageKHR(
// 		*swapchain,
// 		UINT64_MAX,
// 		*image_available_semaphore,
// 		nullptr,
// 		&image_index
// 	); // TODO change to std::limits
//
// 	// record command buffer for this frame's rendering
// 	// command buffer recording happens here with acquired image as render target
// 	// TODO
// }
//
// Resource* Rendergraph::get_resource(const std::string& name)
// {
// 	auto it = resources_.find(name);
// 	return (it != resources_.end()) ? &it->second : nullptr; // FIXME reference to hashmap!
// 	// FIXME Resource and RenderResource should be the same thing
// }
