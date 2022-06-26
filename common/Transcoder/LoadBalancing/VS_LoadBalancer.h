
#ifndef VS_LOAD_BALANCER_H
#define VS_LOAD_BALANCER_H

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "std/cpplib/EventArray.h"
#include "std-generic/cpplib/atomic_shared_ptr.h"
#include "std-generic/cpplib/string_view.h"
#include "std-generic/cpplib/synchronized.h"

class VS_MediaFormat;

namespace hardware_monitoring
{

	struct MonitoringInfo
	{
		uint32_t memoryTotal = 0;
		uint32_t memoryFree = 0;
		double utilizationEngine = 0.0;
		double utilizationEncoder = 0.0;
		double utilizationDecoder = 0.0;
		bool lockedEncoder = false;
		bool lockedDecoder = false;
	};

	class DeviceMonitoring;
}

namespace balancing_module
{

	enum Type : uint32_t
	{
		decompressor = 0,
		mediasynch = 1,
		mixer,
		compressor,
		transcoder,
		webrtc,
		max
	};

	enum Thread : uint32_t
	{
		decoder = 0,
		encoder = 1,
		synch,
		layout,
		resampler,
		control
	};

	class BalancingModule;
}

namespace load_balancing
{

	enum BalancingDevice : uint32_t
	{
		software = 0x01,
		nvidia = 0x02,
		intel = 0x04,
		amd = 0x08
	};

	enum class ErrorMessage : int8_t
	{
		noResources = -1,
		ok = 0,
		noGpuResources,
		noCpuResources,
		cpuLoad
	};

	struct CodecCapability
	{
		int32_t maxStreams = 0;
		uint32_t maxMBps = 0;
		int32_t activeStreams = 0;
		uint32_t activeMBps = 0;
		std::map<uint32_t /* fourcc */, std::pair<uint32_t /* max mb frame */, float /* reduce performance*/>> caps;
		std::string capsView;
	};

	struct DeviceDesc
	{
		std::string name;
		int32_t id = 0;
		std::uintptr_t ctx = 0;
		std::uintptr_t monitor = 0;
		uint32_t baseCoreClock = 0;
		CodecCapability encoderDesc;
		CodecCapability decoderDesc;
		vs::Synchronized<hardware_monitoring::MonitoringInfo> monitorInfo;
		DeviceDesc() = default;
	};

	struct HandleCodecInfo
	{
		BalancingDevice device = BalancingDevice::software;
		std::string name;
		int32_t id = 0;
		std::uintptr_t ctx = 0;
		uint32_t mb = 0;
		uint32_t mbps = 0;
		uint32_t fourcc = 0;
		bool encoder = true;
		ErrorMessage error = ErrorMessage::noResources;
	};

	extern bool RegisterBalancingModule(std::shared_ptr<balancing_module::BalancingModule> mod);

}

typedef std::map<std::uintptr_t /* codec handle */, load_balancing::HandleCodecInfo /* info */> mapHandleStorage;
typedef std::map<load_balancing::BalancingDevice /* type hardware */, std::unique_ptr<hardware_monitoring::DeviceMonitoring> /* monitor */ > mapDeviceMonitor;
typedef std::map<load_balancing::BalancingDevice /* type hardware */, std::vector<load_balancing::DeviceDesc /* dev desc */>> mapDeviceBalancing;
typedef std::vector<std::weak_ptr<balancing_module::BalancingModule> /* weak */ > arrayModuleBalancing;
typedef std::unique_ptr<std::uintptr_t, std::function<void(std::uintptr_t *)>> UniqueContext;

class LoadBalancingHardware
{

public:

	static LoadBalancingHardware& GetLoadBalancing();
	void SoftwareBenchmark();
	void Enable(int32_t levelHardwareBalancing, int32_t levelCpuBalancing);
	load_balancing::HandleCodecInfo HoldResources(const VS_MediaFormat &mf, bool encoder);
	void UnholdResources(const load_balancing::HandleCodecInfo &info);
	bool RegisterVideoCodec(const std::uintptr_t handle, const load_balancing::HandleCodecInfo &info);
	bool UnregisterVideoCodec(const std::uintptr_t handle);
	std::uintptr_t GetContextDevice(const std::uintptr_t handle);
	load_balancing::BalancingDevice GetTypeDevice(const std::uintptr_t handle);

public:

	bool RegisterModule(std::weak_ptr<balancing_module::BalancingModule> mod);

public:

	LoadBalancingHardware(LoadBalancingHardware const&) = delete;
	LoadBalancingHardware& operator= (LoadBalancingHardware const&) = delete;

public:

	static uint32_t HardwareFromDevice(const load_balancing::BalancingDevice &device);

private:

	LoadBalancingHardware();
	~LoadBalancingHardware();

private:

	void MonitoringWorkerThread();
	bool EnumerateResources();
	void ReleaseResources();
	void ClearModules();
	load_balancing::HandleCodecInfo GetHardwareResource(uint32_t fourcc, uint32_t mbps, uint32_t mb, bool encoder);
	load_balancing::HandleCodecInfo GetSoftwareResource(uint32_t fourcc, uint32_t mbps, uint32_t mb, bool encoder);
	bool FreeResource(const load_balancing::HandleCodecInfo & info);
	bool GetResource(load_balancing::DeviceDesc &desc, load_balancing::BalancingDevice device, uint32_t memoryFree, uint32_t mbps, uint32_t mb, uint32_t fourcc, bool encoder, load_balancing::HandleCodecInfo *info);
	void LoggingResources();
	/// software
	bool EnumerateSoftwareDevices(hardware_monitoring::DeviceMonitoring *monitor);
	/// nvidia
	bool CheckNvidiaResources();
	bool EnumerateNvidiaDevices(hardware_monitoring::DeviceMonitoring *monitor);

private:

	void RegisterDeviceContext(const load_balancing::HandleCodecInfo &info);
	void UnregisterDeviceContext(const load_balancing::HandleCodecInfo &info);

private:

	enum MonitorEvents
	{
		cme_checkUtilization = 0,
		cme_checkLogging = 1,
		cme_size
	};

	static LoadBalancingHardware m_loadBalancing;
	std::recursive_mutex m_lblock;
	std::thread m_monitorigThread;
	vs::EventArray m_monitoringEvents;
	bool m_bEnumerate = false;
	int32_t m_levelHardwareBalancing = 0;
	int32_t m_levelCpuBalancing = 0;
	int32_t m_softwareRating = 0;
	mapDeviceMonitor m_monitorDevices;
	mapHandleStorage m_handleStorage;
	mapDeviceBalancing m_balancingDevices;
	vs::atomic_shared_ptr<arrayModuleBalancing> m_balancingModules;
	std::vector<UniqueContext> m_contextDevices;
};

#endif /* VS_LOAD_BALANCER_H */