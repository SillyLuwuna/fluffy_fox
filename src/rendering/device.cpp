#include "rendering/device.hpp"
#include "rendering/window.hpp"

using namespace fluffy_fox::rendering;

Device::Device(vk::raii::Instance& instance) :
	physical_device_(choose_physical_device(instance)),
	device_(create_logical_device(physical_device_))
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
vk::raii::Device Device::create_logical_device(vk::raii::PhysicalDevice& physical_device)
{
	if (physical_device == nullptr) return nullptr;

	i32 queue_index = find_graphics_queue_index(physical_device);
	if (queue_index < 0) return nullptr;

	// needs physical_device
	// needs surface

	// generates queue_index
	// generates queue
}

// returns -1 if not found
i32 Device::find_graphics_queue_index(vk::raii::PhysicalDevice& physical_device)
{
	std::vector<vk::QueueFamilyProperties> queue_family_properties = physical_device.getQueueFamilyProperties();

	i32 queue_index = -1;
	for (i32 i = 0; i < queue_family_properties.size(); i++)
	{
		vk::QueueFamilyProperties& queue_properties = queue_family_properties[i];
		if ((queue_properties.queueFlags & vk::QueueFlagBits::eGraphics) &&
			physical_device.getSurfaceSupportKHR(i, *surface))
		{
			queue_index = i;
			break;
		}
	}

	return queue_index;
}

vk::raii::PhysicalDevice& Device::physical() // TODO remove
{
	return physical_device_;
}

vk::raii::Device& Device::logical() // TODO remove
{
	return device_;
}

bool Device::is_valid()
{
	return (physical_device_ != nullptr) && (device_ != nullptr);
}
