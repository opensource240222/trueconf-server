
#ifndef NVML_DEVICE_MONITORING_H
#define NVML_DEVICE_MONITORING_H

#include "DeviceMonitoring.h"

namespace hardware_monitoring
{

	class NvmlDeviceMonitoring : public DeviceMonitoring
	{

	public:

		NvmlDeviceMonitoring();
		~NvmlDeviceMonitoring();
		std::uintptr_t GetMonitor(const std::string & targetName, const int32_t id, const PciInfo &pciInfo) override;

	private:

		bool GetMemoryUtilization(std::uintptr_t monitor, std::uintptr_t ctx, uint32_t &total, uint32_t &free) override;
		bool GetEngineUtilization(std::uintptr_t monitor, double &encoder, double &decoder, double &engine) override;

	};

}

#endif