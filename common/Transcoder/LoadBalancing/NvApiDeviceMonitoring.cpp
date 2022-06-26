#ifdef _WIN32

#include "NvApiDeviceMonitoring.h"
#include "Transcoder/nvidia/nvapi/nvapi.h"
#include "Transcoder/nvidia/include/cuda.h"

#include "windows.h"

using namespace hardware_monitoring;

NvApiDeviceMonitoring::NvApiDeviceMonitoring()
{

#ifdef _WIN64
	auto hMonitoring = LoadLibrary("nvapi64.dll");
#else
	auto hMonitoring = LoadLibrary("nvapi.dll");
#endif

	if (!hMonitoring) {
		return;
	}
	if (NvAPI_Initialize() != NVAPI_OK) {
		return;
	}
	m_loadResources = true;
};

NvApiDeviceMonitoring::~NvApiDeviceMonitoring()
{
	if (m_loadResources) {
		NvAPI_Unload();
	}
};

std::uintptr_t NvApiDeviceMonitoring::GetMonitor(const std::string & targetName, const int32_t id, const PciInfo &pciInfo)
{
	std::uintptr_t monitor(0);
	NvPhysicalGpuHandle gpus[NVAPI_MAX_PHYSICAL_GPUS] = { 0 };
	NvU32 count(0);
	auto ret = NvAPI_EnumPhysicalGPUs(gpus, &count);
	if (ret != NVAPI_OK) {
		return 0;
	}
	for (uint32_t i = 0; i < count; i++) {
		NvAPI_ShortString name;
		if (NvAPI_GPU_GetFullName(gpus[i], name) != NVAPI_OK) {
			continue;
		}
		NvU32 bus(0);
		if (NvAPI_GPU_GetBusId(gpus[i], &bus) != NVAPI_OK) {
			continue;
		}
		NvU32 domain(0);
		if (NvAPI_GPU_GetBusSlotId(gpus[i], &domain) != NVAPI_OK) {
			continue;
		}
		if (name == targetName
			&& pciInfo.bus == bus
			&& pciInfo.domain == domain)
		{
			monitor = reinterpret_cast<std::uintptr_t>(gpus[i]);
			break;
		}
	}
	return monitor;
}

bool NvApiDeviceMonitoring::GetMemoryUtilization(std::uintptr_t monitor, std::uintptr_t ctx, uint32_t &total, uint32_t &free)
{
	if (cuCtxPushCurrent(reinterpret_cast<CUcontext>(ctx)) != CUresult::CUDA_SUCCESS) {
		false;
	}
	size_t max(0), aval(0);
	cuMemGetInfo(&aval, &max);
	cuCtxPopCurrent(nullptr);
	total = static_cast<uint32_t>(max >> 20);
	free = static_cast<uint32_t>(aval >> 20);
	return true;
}

bool NvApiDeviceMonitoring::GetEngineUtilization(std::uintptr_t monitor, double &encoder, double &decoder, double &engine)
{
	auto handle = reinterpret_cast<NvPhysicalGpuHandle>(monitor);
	NV_GPU_CLOCK_FREQUENCIES_V2 clkFreq = { 0 };
	clkFreq.ClockType = NV_GPU_CLOCK_FREQUENCIES_CURRENT_FREQ;
	clkFreq.version = NV_GPU_CLOCK_FREQUENCIES_VER;
	if (NvAPI_GPU_GetAllClockFrequencies(handle, &clkFreq) != NVAPI_OK) {
		return false;
	}
	if (!clkFreq.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].bIsPresent) {
		return false;
	}
	auto coreClock = clkFreq.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].frequency / 1000;
	NV_GPU_DYNAMIC_PSTATES_INFO_EX info;
	info.version = NV_GPU_DYNAMIC_PSTATES_INFO_EX_VER;
	if (NvAPI_GPU_GetDynamicPstatesInfoEx(handle, &info) != NVAPI_OK) {
		return false;
	}
	if (!info.utilization->bIsPresent) {
		return false;
	}
	{
		engine = static_cast<float>(info.utilization[2].percentage);
		encoder = decoder = engine;
	}
	NvU32 maxCoreClock(0);
	NV_GPU_PERF_PSTATE_ID state;
	if (NvAPI_GPU_GetCurrentPstate(handle, &state) != NVAPI_OK) {
		return true;
	}
	NV_GPU_PERF_PSTATES20_INFO_V2 pstate20;
	pstate20.version = NV_GPU_PERF_PSTATES20_INFO_VER;
	if (NvAPI_GPU_GetPstates20(handle, &pstate20) != NVAPI_OK) {
		return true;
	}
	for (uint32_t i = 0; i < pstate20.numPstates; i++) {
		if (pstate20.pstates[i].pstateId != state) {
			continue;
		}
		for (uint32_t j = 0; j < pstate20.numClocks; j++) {
			if (pstate20.pstates[state].clocks[i].domainId != NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS) {
				continue;
			}
			maxCoreClock = pstate20.pstates[state].clocks[i].data.range.maxFreq_kHz / 1000;
			break;
		}
		if (maxCoreClock != 0) {
			break;
		}
	}
	if (maxCoreClock == 0) {
		maxCoreClock = coreClock;
	}
	{
		double k = static_cast<double>(coreClock) / static_cast<double>(maxCoreClock);
		engine *= k;
		encoder = decoder = engine;
	}
	return true;
}

#endif