
#ifndef DEVICE_MONITORING_H
#define DEVICE_MONITORING_H

#include <string>
#include <memory>

#include "VS_LoadBalancer.h"

#ifdef _WIN32
#define PORTED_CODE
#else
#undef PORTED_CODE
#endif

namespace hardware_monitoring
{

	enum MonitoringModule : uint32_t
	{
		none = 0x00,
		software = 0x01,
		nvapi = 0x02,
		nvml = 0x04
	};

	struct PciInfo
	{
		int32_t bus = 0;
		int32_t domain = 0;
		int32_t device = 0;
	};

	class DeviceMonitoring
	{

	public:

		static std::unique_ptr<DeviceMonitoring> CreateDeviceMonitoring(const load_balancing::BalancingDevice &device);

	public:

		virtual ~DeviceMonitoring() {};
		bool ResourcesLoaded();
		virtual std::uintptr_t GetMonitor(const std::string & targetName, const int32_t id, const PciInfo &pciInfo) = 0;
		bool GetDeviceUtilization(std::uintptr_t monitor, std::uintptr_t ctx, MonitoringInfo &info);

	protected:

		virtual bool GetMemoryUtilization(std::uintptr_t monitor, std::uintptr_t ctx, uint32_t &total, uint32_t &free) = 0;
		virtual bool GetEngineUtilization(std::uintptr_t monitor, double &encoder, double &decoder, double &engine) = 0;
		DeviceMonitoring() {};

	protected:

		bool m_loadResources = false;

	};

	class SoftwareDeviceMonitoring : public DeviceMonitoring
	{

	public:

		SoftwareDeviceMonitoring();
		~SoftwareDeviceMonitoring() {};
		std::uintptr_t GetMonitor(const std::string & targetName, const int32_t id, const PciInfo &pciInfo) override;

	private:

		bool GetMemoryUtilization(std::uintptr_t monitor, std::uintptr_t ctx, uint32_t &total, uint32_t &free) override;
		bool GetEngineUtilization(std::uintptr_t monitor, double &encoder, double &decoder, double &engine) override;

	};

}

#endif