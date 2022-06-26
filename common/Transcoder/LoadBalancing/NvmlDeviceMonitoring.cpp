#if !(defined(_WIN32) && !defined(_WIN64))

#include "NvmlDeviceMonitoring.h"
#include "Transcoder/nvidia/nvml/nvml.h"

#ifdef _WIN32
#include "windows.h"
#endif

using namespace hardware_monitoring;

NvmlDeviceMonitoring::NvmlDeviceMonitoring()
{

#ifdef _WIN32
	auto hMonitoring = LoadLibrary("nvml.dll");
	if (!hMonitoring) {
		std::string loc(getenv("ProgramW6432"));
		loc += "\\Nvidia Corporation\\nvsmi\\nvml.dll";
		hMonitoring = LoadLibrary(loc.c_str());
	}
#else
	auto hMonitoring(0);
#endif

	if (!hMonitoring) {
		return;
	}

#ifdef PORTED_CODE
	if (nvmlInit() != NVML_SUCCESS) {
		return;
	}
	m_loadResources = true;
#endif

};

NvmlDeviceMonitoring::~NvmlDeviceMonitoring()
{
#ifdef PORTED_CODE
	if (m_loadResources) {
		nvmlShutdown();
	}
#endif
};

std::uintptr_t NvmlDeviceMonitoring::GetMonitor(const std::string & targetName, const int32_t id, const PciInfo &pciInfo)
{
	std::uintptr_t monitor(0);
#ifdef PORTED_CODE
	uint32_t count(0);
	auto ret = nvmlDeviceGetCount(&count);
	if (ret != NVML_SUCCESS) {
		return 0;
	}
	for (uint32_t i = 0; i < count; i++) {
		nvmlDevice_t gpu;
		ret = nvmlDeviceGetHandleByIndex(i, &gpu);
		if (ret != NVML_SUCCESS) {
			continue;
		}
		char name[NVML_DEVICE_NAME_BUFFER_SIZE];
		ret = nvmlDeviceGetName(gpu, name, NVML_DEVICE_NAME_BUFFER_SIZE);
		if (ret != NVML_SUCCESS) {
			continue;
		}
		nvmlPciInfo_t pci;
		ret = nvmlDeviceGetPciInfo(gpu, &pci);
		if (ret != NVML_SUCCESS) {
			continue;
		}
		if (name == targetName &&
			pciInfo.bus == pci.bus &&
			pciInfo.device == pci.device &&
			pciInfo.domain == pci.domain)
		{
			monitor = reinterpret_cast<std::uintptr_t>(gpu);
			break;
		}
	}
#endif
	return monitor;
}

bool NvmlDeviceMonitoring::GetMemoryUtilization(std::uintptr_t monitor, std::uintptr_t ctx, uint32_t &total, uint32_t &free)
{
#ifdef PORTED_CODE
	nvmlMemory_t memory;
	if (nvmlDeviceGetMemoryInfo(reinterpret_cast<nvmlDevice_t>(monitor), &memory) != NVML_SUCCESS) {
		return false;
	}
	total = static_cast<uint32_t>(memory.total >> 20);
	free = static_cast<uint32_t>(memory.free >> 20);
#endif
	return true;
}

bool NvmlDeviceMonitoring::GetEngineUtilization(std::uintptr_t monitor, double &encoder, double &decoder, double &engine)
{
	bool ret(true);
#ifdef PORTED_CODE
	uint32_t utilization(0), periodUs(0);
	ret = ret && (nvmlDeviceGetEncoderUtilization(reinterpret_cast<nvmlDevice_t>(monitor), &utilization, &periodUs) == NVML_SUCCESS);
	encoder = static_cast<double>(utilization);
	ret = ret && (nvmlDeviceGetDecoderUtilization(reinterpret_cast<nvmlDevice_t>(monitor), &utilization, &periodUs) == NVML_SUCCESS);
	decoder = static_cast<double>(utilization);
	engine = (encoder + decoder) / 2.0;
#endif
	return ret;
}

#endif