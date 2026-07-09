#include "hwinfo/gpu.h"
#include <cstdint>
#include <string>

namespace fluffy_fox::rendering
{
	struct GpuConstants
	{
		inline static const uint8_t count = hwinfo::getAllGPUs().size();
	};

	struct GpuInstance : GpuConstants
	{
	private:
		std::string vendor_name_;
		std::string name_;
		uint16_t vendor_id_;
		uint16_t device_id_;

	public:
		GpuInstance(uint8_t id) :
			vendor_name_(hwinfo::getAllGPUs()[id].vendor()),
			name_(hwinfo::getAllGPUs()[id].name()),
			vendor_id_(static_cast<uint16_t>(std::stoi(hwinfo::getAllGPUs()[id].vendor_id(), nullptr, 0))),
			device_id_(static_cast<uint16_t>(std::stoi(hwinfo::getAllGPUs()[id].device_id(), nullptr, 0)))
		{
		}

		std::string vendor_name() { return vendor_name_; }
		std::string name() { return name_; }
		uint16_t vendor_id() { return vendor_id_; }
		uint16_t device_id() { return device_id_; }
	};

	template<uint8_t Num>
	struct Gpu : GpuConstants
	{
	public:
		inline static const std::string vendor_name = hwinfo::getAllGPUs()[Num].vendor();
		inline static const std::string name = hwinfo::getAllGPUs()[Num].name();
		inline static const uint16_t vendor_id = static_cast<uint16_t>(std::stoi(hwinfo::getAllGPUs()[Num].vendor_id(), nullptr, 0));
		inline static const uint16_t device_id = static_cast<uint16_t>(std::stoi(hwinfo::getAllGPUs()[Num].device_id(), nullptr, 0));
	};
}
