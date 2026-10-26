#include "rendering/device.hpp"

using namespace fluffy_fox::rendering;

Device::Device(vk::raii::Instance& instance, vk::SurfaceKHR surface) :
	physical_device_(choose_physical_device(instance)),
	device_(create_logical_device(physical_device_, surface))
	// TODO error handling, what if both of these end up being nullptr? how should other
	// classes know and deal with it?
{ }

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

	init_queue(graphics_, device);
	// init_queue(present_, device);
	// init_queue(transfer_, device);
	// init_queue(compute_, device);

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

bool Device::is_valid()
{
	return (physical_device_ != nullptr) && (device_ != nullptr);
}
