
#include "DeviceMonitoring.h"
#include "std/cpplib/VS_PerformanceMonitor.h"

#if !(defined(_WIN32) && !defined(_WIN64))
#include "NvmlDeviceMonitoring.h"
#endif

#ifdef _WIN32
#include "NvApiDeviceMonitoring.h"
#include "windows.h"
#endif

using namespace hardware_monitoring;

std::unique_ptr<DeviceMonitoring> DeviceMonitoring::CreateDeviceMonitoring(const load_balancing::BalancingDevice &device)
{
	std::unique_ptr<DeviceMonitoring> monitor;
	if (device == load_balancing::BalancingDevice::software) {
		monitor = std::make_unique<SoftwareDeviceMonitoring>();
	}
	else if (device == load_balancing::BalancingDevice::nvidia) {
#ifdef PORTED_CODE
#if !(defined(_WIN32) && !defined(_WIN64))
		monitor = std::make_unique<NvmlDeviceMonitoring>();
#endif
#endif
#ifdef _WIN32
		if (!monitor || !monitor->ResourcesLoaded()) {
			monitor = std::make_unique<NvApiDeviceMonitoring>();
		}
#endif
	}
	if (!monitor->ResourcesLoaded()) {
		monitor.reset();
	}
	return monitor;
}

bool DeviceMonitoring::ResourcesLoaded()
{
	return m_loadResources;
}

bool DeviceMonitoring::GetDeviceUtilization(std::uintptr_t monitor, std::uintptr_t ctx, MonitoringInfo &info)
{
	if (!m_loadResources) {
		return false;
	}
	bool ret(true);
	info.memoryTotal = info.memoryFree = 0;
	ret = ret && GetMemoryUtilization(monitor, ctx, info.memoryTotal, info.memoryFree);
	info.utilizationEncoder = info.utilizationDecoder = info.utilizationEngine = 0;
	ret = ret && GetEngineUtilization(monitor, info.utilizationEncoder, info.utilizationDecoder, info.utilizationEngine);
	return ret;
}

SoftwareDeviceMonitoring::SoftwareDeviceMonitoring()
{
	m_loadResources = true;
}

std::uintptr_t SoftwareDeviceMonitoring::GetMonitor(const std::string & targetName, const int32_t id, const PciInfo &pci)
{
	return 0;
}

bool SoftwareDeviceMonitoring::GetMemoryUtilization(std::uintptr_t monitor, std::uintptr_t ctx, uint32_t &total, uint32_t &free)
{

#ifdef _WIN32
	MEMORYSTATUSEX info;
	info.dwLength = sizeof(info);
	if (GlobalMemoryStatusEx(&info) != TRUE) {
		return false;
	}
#ifdef _WIN64
	total = static_cast<uint32_t>(info.ullTotalPhys >> 20);
	free = static_cast<uint32_t>(info.ullAvailPhys >> 20);
#else
	total = static_cast<uint32_t>(info.ullTotalVirtual >> 20);
	free = static_cast<uint32_t>(info.ullAvailVirtual >> 20);
#endif
	return true;
#else
	return false;
#endif

}

bool SoftwareDeviceMonitoring::GetEngineUtilization(std::uintptr_t monitor, double &encoder, double &decoder, double &engine)
{
	engine = VS_PerformanceMonitor::Instance().GetTotalProcessorTime();
	encoder = VS_PerformanceMonitor::Instance().GetTotalProcessTime();
	decoder = 0.0;
	return true;
}