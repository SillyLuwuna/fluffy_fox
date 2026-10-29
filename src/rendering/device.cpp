#include "rendering/device.hpp"

using namespace fluffy_fox::rendering;
Device::Device() :
	physical_device_(nullptr),
	device_(nullptr),
	msaa_sample_count_(vk::SampleCountFlagBits::e1),
	graphics_command_pool_(nullptr)
{ }

void Device::create(vk::raii::Instance& instance, vk::SurfaceKHR surface)
{
	Device::instance().init(instance, surface);
}

void Device::init(vk::raii::Instance& instance, vk::SurfaceKHR surface)
{
	physical_device_ = choose_physical_device(instance);
	device_ = create_logical_device(physical_device_, surface);
	msaa_sample_count_ = get_max_usable_msaa_sample_count();
	graphics_command_pool_ = create_command_pool(graphics_family());
	depth_format_ = find_depth_format();

	// TODO mix command pools, make this abstract.
	// different families might have the same index,
	// meaning that they should have the same queue and
	// command pool! However, the API should display them
	// as if they were separate, regardless if they're actually
	// just the same queue/command_pool
}

// returns nullptr if there is no suitable device
vk::raii::PhysicalDevice Device::choose_physical_device(vk::raii::Instance& instance)
{
	auto physical_devices = instance.enumeratePhysicalDevices();
	if (physical_devices.empty()) return nullptr;

	vk::raii::PhysicalDevice chosen_device = nullptr;
	i32 score = -1;

	for (auto physical_device : physical_devices)
	{
		if (!is_physical_device_suitable(physical_device)) continue;

		i32 curr_score = device_score(physical_device);

		if (curr_score > score)
		{
			chosen_device = physical_device;
			score = curr_score;
		}
	}

	return chosen_device;
}

bool Device::is_physical_device_suitable(vk::raii::PhysicalDevice& device)
{
	return
		supports_vulkan_version(device, vk::ApiVersion13) &&
		supports_queue_type(device, vk::QueueFlagBits::eGraphics) &&
		supports_required_extensions(device) &&
		supports_required_features(device)
	;
}

bool Device::supports_vulkan_version(vk::raii::PhysicalDevice& device, u32 version)
{
	auto device_properties = device.getProperties();
	return device_properties.apiVersion >= version;
}

bool Device::supports_queue_type(vk::raii::PhysicalDevice& device, vk::QueueFlags queue_flags)
{
	auto queue_families = device.getQueueFamilyProperties();
	return std::ranges::any_of(queue_families, [queue_flags](auto const &queue_family) {
		return (bool)(queue_family.queueFlags & queue_flags);
	});
}

bool Device::supports_required_extensions(vk::raii::PhysicalDevice& device)
{
	auto available_extensions = device.enumerateDeviceExtensionProperties();
	std::vector<std::string> required_extensions = get_physical_device_required_extensions();

	for (std::string& required_extension : required_extensions)
	{
		bool found = false;
		for (auto extension : available_extensions)
		{
			std::string extension_name = extension.extensionName;
			if (extension_name == required_extension)
			{
				found = true;
				break;
			}
		}

		if (!found) return false;
	}

	return true;
}

bool Device::supports_required_features(vk::raii::PhysicalDevice& device)
{
	auto features = device.template getFeatures2<
		vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceVulkan11Features,
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
	>();

	return
		features.template get <vk::PhysicalDeviceFeatures2>().features.samplerAnisotropy &&
		features.template get <vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
		features.template get <vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
		features.template get <vk::PhysicalDeviceVulkan13Features>().synchronization2 &&
		features.template get <vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState
	;
}

std::vector<std::string> Device::get_physical_device_required_extensions()
{
// 	std::vector<std::string> required_extensions = Window::get_required_extensions();
//
// 	required_extensions.emplace_back(vk::KHRSwapchainExtensionName);
//
// #ifdef VALIDATION_LAYERS
// 		required_extensions.emplace_back(vk::EXTDebugUtilsExtensionName);
// #endif
// 	
// 	return required_extensions;
	std::vector<std::string> required_extensions {
		vk::KHRSwapchainExtensionName,
	};
	
	return required_extensions;
}

i32 Device::device_score(vk::raii::PhysicalDevice& device)
{
	auto device_properties = device.getProperties();
	i32 score = 0;

	if (device_properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
	{
		score += 1;
	}

	return score;
}

// returns nullptr if error
vk::raii::Device Device::create_logical_device(vk::raii::PhysicalDevice& physical_device, vk::SurfaceKHR surface)
{
	if (physical_device == nullptr) return nullptr;

	FeatureChain feature_chain = get_desired_feature_chain();
	std::vector<const char*> required_device_extensions = get_logical_device_required_extensions();
	std::vector<vk::DeviceQueueCreateInfo> queue_create_infos = get_queue_create_infos(surface);

	vk::DeviceCreateInfo device_create_info {
		.pNext = feature_chain.get(),
		.queueCreateInfoCount = static_cast<u32>(queue_create_infos.size()),
		.pQueueCreateInfos = queue_create_infos.data(),
		.enabledExtensionCount = static_cast<u32>(required_device_extensions.size()),
		.ppEnabledExtensionNames = required_device_extensions.data()
	};

	vk::raii::Device device = vk::raii::Device(physical_device, device_create_info);

	return std::move(device);
}

// assumes queue_info is populated
void Device::init_queue(Device::QueueInfo& queue_info, vk::raii::Device& device)
{
	queue_info.queue = vk::raii::Queue(device, queue_info.family, queue_info.index);
}

// returns -1 if not found
i32 Device::find_graphics_queue_index(vk::raii::PhysicalDevice& physical_device, vk::SurfaceKHR surface)
{
	std::vector<vk::QueueFamilyProperties> queue_family_properties = physical_device.getQueueFamilyProperties();

	i32 queue = -1;

	for (i32 i = 0; i < queue_family_properties.size(); i++)
	{
		vk::QueueFamilyProperties& queue_properties = queue_family_properties[i];
		bool is_graphics_queue = static_cast<bool>(queue_properties.queueFlags & vk::QueueFlagBits::eGraphics);
		bool is_present_queue = physical_device.getSurfaceSupportKHR(i, surface);

		if (is_graphics_queue && is_present_queue)
		{
			return i;
		}

		if (is_graphics_queue)
		{
			queue = i;
		}
	}

	return queue;
}

// returns -1 if not found
i32 Device::find_transfer_queue_index(vk::raii::PhysicalDevice& physical_device)
{
	std::vector<vk::QueueFamilyProperties> queue_family_properties = physical_device.getQueueFamilyProperties();

	for (i32 i = 0; i < queue_family_properties.size(); i++)
	{
		vk::QueueFamilyProperties& queue_properties = queue_family_properties[i];
		if (queue_properties.queueFlags & vk::QueueFlagBits::eTransfer)
		{
			return i;
		}
	}

	return -1;
}

i32 Device::find_present_queue_index(vk::raii::PhysicalDevice& physical_device, vk::SurfaceKHR surface)
{
	std::vector<vk::QueueFamilyProperties> queue_family_properties = physical_device.getQueueFamilyProperties();

	i32 queue = -1;

	for (i32 i = 0; i < queue_family_properties.size(); i++)
	{
		vk::QueueFamilyProperties& queue_properties = queue_family_properties[i];
		bool is_graphics_queue = static_cast<bool>(queue_properties.queueFlags & vk::QueueFlagBits::eGraphics);
		bool is_present_queue = physical_device.getSurfaceSupportKHR(i, surface);

		if (is_graphics_queue && is_present_queue)
		{
			return i;
		}

		if (is_present_queue)
		{
			queue = i;
		}
	}

	return queue;
}

i32 Device::find_compute_queue_index(vk::raii::PhysicalDevice& physical_device)
{
	std::vector<vk::QueueFamilyProperties> queue_family_properties = physical_device.getQueueFamilyProperties();

	for (i32 i = 0; i < queue_family_properties.size(); i++)
	{
		vk::QueueFamilyProperties& queue_properties = queue_family_properties[i];
		if (queue_properties.queueFlags & vk::QueueFlagBits::eCompute)
		{
			return i;
		}
	}

	return -1;
}

Device::FeatureChain Device::get_desired_feature_chain()
{
	FeatureChain feature_chain {
		.feature_chain {
			{ // Features2
				.features = {
					.sampleRateShading = true,
					.samplerAnisotropy = true
				}
			},
			{ // Vulkan11Features
				.shaderDrawParameters = true
			},
			{ // Vulkan13Features
				.synchronization2 = true,
				.dynamicRendering = true
			},
			{ // ExtendedDynamicStateFeaturesEXT
				.extendedDynamicState = true
			}
		}
	};

	return feature_chain;
}

std::vector<const char*> Device::get_logical_device_required_extensions()
{
	return {
		vk::KHRSwapchainExtensionName
	};
}

// populates queues_
std::vector<vk::DeviceQueueCreateInfo> Device::get_queue_create_infos(vk::SurfaceKHR surface)
{
	std::vector<vk::DeviceQueueCreateInfo> device_create_infos;

	// FIXME check if the queues are actually the same family.
	// if they are, don't create so many queues.
	create_graphics_queues(device_create_infos, surface);
	create_present_queues(device_create_infos, surface);
	create_transfer_queues(device_create_infos);
	create_compute_queues(device_create_infos);

	return device_create_infos;
}

void Device::create_graphics_queues(std::vector<vk::DeviceQueueCreateInfo>& out, vk::SurfaceKHR surface)
{
	i32 family = find_graphics_queue_index(physical_device_, surface);
	if (family < 0) return;

	graphics_ = {
		.family = static_cast<u32>(family),
		.index = 0,
		.priority = 0.5f,
		.queue = nullptr
	};

	vk::DeviceQueueCreateInfo queue_create_info {
		.queueFamilyIndex = graphics_.family,
		.queueCount = 1,
		.pQueuePriorities = &graphics_.priority
	};

	out.emplace_back(queue_create_info);
}

void Device::create_present_queues(std::vector<vk::DeviceQueueCreateInfo>& out, vk::SurfaceKHR surface)
{
	i32 family = find_present_queue_index(physical_device_, surface);
	if (family < 0) return;

	present_ = {
		.family = static_cast<u32>(family),
		.index = 0,
		.priority = 0.5f,
		.queue = nullptr
	};

	vk::DeviceQueueCreateInfo queue_create_info {
		.queueFamilyIndex = present_.family,
		.queueCount = 1,
		.pQueuePriorities = &present_.priority
	};

	// out.emplace_back(queue_create_info);
}

void Device::create_transfer_queues(std::vector<vk::DeviceQueueCreateInfo>& out)
{
	i32 family = find_transfer_queue_index(physical_device_);
	if (family < 0) return;

	transfer_ = {
		.family = static_cast<u32>(family),
		.index = 0,
		.priority = 0.5f,
		.queue = nullptr
	};

	vk::DeviceQueueCreateInfo queue_create_info {
		.queueFamilyIndex = transfer_.family,
		.queueCount = 1,
		.pQueuePriorities = &transfer_.priority
	};

	// out.emplace_back(queue_create_info);
}

void Device::create_compute_queues(std::vector<vk::DeviceQueueCreateInfo>& out)
{
	i32 family = find_compute_queue_index(physical_device_);
	if (family < 0) return;

	compute_ = {
		.family = static_cast<u32>(family),
		.index = 0,
		.priority = 0.5f,
		.queue = nullptr
	};

	vk::DeviceQueueCreateInfo queue_create_info {
		.queueFamilyIndex = compute_.family,
		.queueCount = 1,
		.pQueuePriorities = &compute_.priority
	};

	// out.emplace_back(queue_create_info);
}

vk::raii::CommandPool Device::create_command_pool(u32 family) const
{
	vk::CommandPoolCreateInfo pool_info {
		.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		.queueFamilyIndex = family
	};

	return vk::raii::CommandPool(device_, pool_info);
}

vk::SampleCountFlagBits Device::get_max_usable_msaa_sample_count() const
{
	// PERF might not be the best idea to choose the highest possible
	vk::PhysicalDeviceProperties physical_device_properties = physical_device_.getProperties();

	vk::SampleCountFlags counts =
		physical_device_properties.limits.framebufferColorSampleCounts &
		physical_device_properties.limits.framebufferDepthSampleCounts;

	if (counts & vk::SampleCountFlagBits::e64) { return vk::SampleCountFlagBits::e64; }
	if (counts & vk::SampleCountFlagBits::e32) { return vk::SampleCountFlagBits::e32; }
	if (counts & vk::SampleCountFlagBits::e16) { return vk::SampleCountFlagBits::e16; }
	if (counts & vk::SampleCountFlagBits::e8) { return vk::SampleCountFlagBits::e8; }
	if (counts & vk::SampleCountFlagBits::e4) { return vk::SampleCountFlagBits::e4; }
	if (counts & vk::SampleCountFlagBits::e2) { return vk::SampleCountFlagBits::e2; }

	return vk::SampleCountFlagBits::e1;
}

bool Device::is_valid()
{
	return (physical_device_ != nullptr) && (device_ != nullptr);
}

vk::Format Device::find_depth_format() const
{
	return find_supported_format(
		{
			vk::Format::eD32Sfloat,
			vk::Format::eD32SfloatS8Uint,
			vk::Format::eD24UnormS8Uint
		},
		vk::ImageTiling::eOptimal,
		vk::FormatFeatureFlagBits::eDepthStencilAttachment
	);
}

vk::Format Device::find_supported_format(
	const std::vector<vk::Format>& candidates,
	vk::ImageTiling tiling,
	vk::FormatFeatureFlags features
) const
{
	for (const auto format : candidates)
	{
		vk::FormatProperties props = physical_device_.getFormatProperties(format);

		if (((tiling == vk::ImageTiling::eLinear) && ((props.linearTilingFeatures & features) == features)) ||
			((tiling == vk::ImageTiling::eOptimal) && ((props.optimalTilingFeatures & features) == features)))
		{
			return format;
		}
	}

	// FIXME remove exception
	throw std::runtime_error("failed to find supported format!");
}

// returns ~0u when errored
u32 Device::find_memory_type(u32 type_filter, vk::MemoryPropertyFlags properties)
{
	// PERF selecting a specific heap "memoryHeaps" may increase performance
	vk::PhysicalDeviceMemoryProperties mem_properties = physical_device_.getMemoryProperties();

	for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
	{
		if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	return ~0u;
}

vk::raii::CommandPool& Device::single_time_commands_command_pool()
{
	// TODO develop
	return graphics_command_pool_;
}

vk::raii::Queue& Device::single_time_commands()
{
	// TODO develop
	return graphics();
}
