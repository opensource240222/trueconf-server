/**
 **************************************************************************
 * \file VS_VS_PerformanceMonitorNT.cpp
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Windows NT OS performance monitor class implementation
 * By Retriving type of OS created corresponding class (now NT only)
 *
 * \b Project Standart Libraries
 * \author SMirnovK
 * \date 15.07.03
 *
 * $Revision: 6 $
 *
 * $History: VS_VS_PerformanceMonitorNT.cpp $
 *
 * *****************  Version 6  *****************
 * User: Dront78      Date: 25.05.11   Time: 18:53
 * Updated in $/VSNA/std/cpplib
 * - armadillo optimizations disabled totally
 *
 * *****************  Version 5  *****************
 * User: Melechko     Date: 1.02.10    Time: 16:30
 * Updated in $/VSNA/std/cpplib
 *
 * *****************  Version 4  *****************
 * User: Dront78      Date: 25.01.10   Time: 19:37
 * Updated in $/VSNA/std/cpplib
 *
 * *****************  Version 3  *****************
 * User: Melechko     Date: 21.01.10   Time: 18:18
 * Updated in $/VSNA/std/cpplib
 * fix nanomites
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 18.01.10   Time: 14:44
 * Updated in $/VSNA/std/cpplib
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/std/cpplib
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/std/cpplib
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 20.07.05   Time: 18:55
 * Updated in $/VS/std/cpplib
 * - fix for Win 2000 and other..
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 19.07.05   Time: 19:35
 * Updated in $/VS/std/cpplib
 * on Win2k bug fix
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 24.06.05   Time: 19:11
 * Updated in $/VS/std/cpplib
 * - added russian OS support
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 29.12.04   Time: 15:11
 * Updated in $/VS/std/cpplib
 * files headers
 ****************************************************************************/

#include "VS_PerformanceMonitor.h"
#include "VS_RegistryKey.h"
#include "VS_RegistryConst.h"
#include "event.h"
#include "std-generic/cpplib/atomic_shared_ptr.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std/cpplib/ThreadUtils.h"

#ifdef WIN32
#include <windows.h>
#endif

// MSVC 12.0 runtime works badly with static threads:
//   1. It deadlocks when std::thread::join is called after main is exited: https://connect.microsoft.com/VisualStudio/feedback/details/747145
//   2. It sometimes crashes at startup when thread isn't created on the heap.
//   3. It sometimes crashes at shutdown if thread stops while destruction of static objects is happening.
#if defined(_MSC_VER) && _MSC_VER < 1900
#include <boost/thread/thread.hpp>
#else
#include <thread>
#endif

#include "std-generic/compat/memory.h"
#include <atomic>
#include <cinttypes>
#include <map>

#define DEFAULT_PERFORMANCE_MONITOR_TIMEOUT 5000;

namespace monitoring
{

#ifdef WIN32

	std::uintptr_t OpenThreadHandle(int32_t tid)
	{
		return reinterpret_cast<std::uintptr_t>(::OpenThread(THREAD_QUERY_INFORMATION, FALSE, tid));
	}

	void CloseThreadHandle(std::uintptr_t handle)
	{
		if (handle) {
			::CloseHandle(reinterpret_cast<HANDLE>(handle));
		}
	}

#else

	std::uintptr_t OpenThreadHandle(int32_t tid)
	{
		return 0;
	}

	void CloseThreadHandle(std::uintptr_t handle)
	{

	}

#endif

	struct ThreadStat
	{
		std::atomic<double> cpuUtilization = ATOMIC_VAR_INIT(0.);
		uint64_t prevKernelTime = 0;
		uint64_t prevUserTime = 0;
		uint64_t prevCycleCount = 0;
		std::uintptr_t handle = 0;
		ThreadStat(int32_t tid)
		{
			handle = OpenThreadHandle(tid);
		}
		~ThreadStat()
		{
			CloseThreadHandle(handle);
		}
	};
}

typedef std::map<int32_t, std::shared_ptr<monitoring::ThreadStat>> mapMonitoringThreads;

class VS_PerformanceMonitor::Impl
{
public:

	Impl()
		: m_stop_event(true)
		, m_monitoringThreads(std::make_shared<mapMonitoringThreads>())
		, m_thread(&Impl::MeasureCpuLoadWorkerThread, this)
	{
	}

	~Impl() {
		m_stop_event.set();
		m_thread.join();
	}

	double GetTotalProcessorTime() const {
		return 100 * m_cpu_load;
	}

	double GetTotalProcessTime() const {
		return 100 * m_process_load;
	}

	void AddMonitoringThread(int32_t tid) {
		auto shared = std::make_shared<monitoring::ThreadStat>(tid);
		auto monitors = m_monitoringThreads.load();
		auto new_monitors = std::make_shared<mapMonitoringThreads>();
		do {
			*new_monitors = *monitors;
			new_monitors->emplace(tid, shared);
		} while (!m_monitoringThreads.compare_exchange_weak(monitors, new_monitors));
	}

	void RemoveMonitoringThread(int32_t tid) {
		auto monitors = m_monitoringThreads.load();
		auto new_monitors = std::make_shared<mapMonitoringThreads>();
		do {
			*new_monitors = *monitors;
			new_monitors->erase(tid);
		} while (!m_monitoringThreads.compare_exchange_weak(monitors, new_monitors));
	}

	double GetTotalThreadTime(int32_t tid) const {
		auto monitors = m_monitoringThreads.load();
		auto it = monitors->find(tid);
		if (it == monitors->end()) {
			return 0.;
		}
		return it->second->cpuUtilization.load() * 100.0;
	}

private:

	void MeasureCpuLoadWorkerThread()
	{
		vs::SetThreadName("PerfMonitor");
		Initialize();
		VS_RegistryKey key(false, CONFIGURATION_KEY, false, true);
		for (;;) {
			double cpuUtilization(0.0), processUtiliaztion(0.0);
			if (CalculateCPULoad(cpuUtilization, processUtiliaztion)) {
				m_cpu_load.store(cpuUtilization);
				if (processUtiliaztion != 0.0) {
					m_process_load.store(processUtiliaztion);
				}
			}
			CalculateThreadsUtilization();
			unsigned timeout_ms = DEFAULT_PERFORMANCE_MONITOR_TIMEOUT;
			key.GetValue(&timeout_ms, sizeof(unsigned), VS_REG_INTEGER_VT, "PerformanceMonitorTimeout");
			if (m_stop_event.wait_for(std::chrono::milliseconds(timeout_ms)) == true) {
				break;
			}
		}
	}

#ifdef WIN32

	uint64_t m_prevSysTotal = 0;
	uint64_t m_prevSysIdle = 0;
	uint64_t m_prevProcTotal = 0;
	uint64_t m_prevThreadCycleTime = 0;
	uint64_t m_prevQpcThread = 0;
	uint64_t m_performanceCounterFrequency = 0;
	double m_auxiliaryCounterFrequency = 0.0;

	static uint64_t FileTimeToUint64(const FILETIME & ft) {
		return (((uint64_t)(ft.dwHighDateTime)) << 32) | ((uint64_t)ft.dwLowDateTime);
	}

	void Initialize() {
		m_auxiliaryCounterFrequency = GetAuxiliaryFrequency();
	}

	double GetAuxiliaryFrequency() {
		double auxiliaryFrequency(0.0);
		LARGE_INTEGER qpc = {};
		int priority = ::GetThreadPriority(::GetCurrentThread());
		::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
		VS_SCOPE_EXIT{ ::SetThreadPriority(::GetCurrentThread(), priority); };
		if (::QueryPerformanceFrequency(&qpc) == FALSE) {
			return 0.0;
		}
		m_performanceCounterFrequency = qpc.QuadPart;
		if (::QueryPerformanceCounter(&qpc) == FALSE) {
			return 0.0;
		}
		uint64_t tsc_initial = __rdtsc();
		uint64_t qpc_initial = qpc.QuadPart;
		double kMinimumEvaluationPeriodSeconds = 0.05;
		for (int i = 0; i < 10; i++) {
			::Sleep(10);
			uint64_t tsc_now = __rdtsc();
			if (tsc_now <= tsc_initial) {
				continue;
			}
			if (::QueryPerformanceCounter(&qpc) == FALSE) {
				break;
			}
			if (qpc.QuadPart <= qpc_initial) {
				continue;
			}
			double elapsed_time_seconds = (qpc.QuadPart - qpc_initial) / static_cast<double>(m_performanceCounterFrequency);
			if (elapsed_time_seconds < kMinimumEvaluationPeriodSeconds) {
				continue;
			}
			auxiliaryFrequency = (tsc_now - tsc_initial) / elapsed_time_seconds;
			break;
		}
		return auxiliaryFrequency;
	}

	bool CalculateCPULoad(double &cpuUtilization, double &processUtilization)
	{
		FILETIME sysIdle, sysKernel, sysUser;
		FILETIME procCreate, procExit, procKernel, procUser;

		cpuUtilization = 0.;
		processUtilization = 0.;

		if (::GetSystemTimes(&sysIdle, &sysKernel, &sysUser) == FALSE ||
			::GetProcessTimes(GetCurrentProcess(), &procCreate, &procExit, &procKernel, &procUser) == FALSE)
		{
			return false;
		}

		uint64_t cpuIdle = FileTimeToUint64(sysIdle);
		uint64_t cpuTotal = FileTimeToUint64(sysKernel) + FileTimeToUint64(sysUser);
		uint64_t procTotal = FileTimeToUint64(procKernel) + FileTimeToUint64(procUser);

		if (m_prevSysTotal == 0 && m_prevSysIdle == 0 && m_prevProcTotal == 0) {
			// First call, previous values not known, can't compute the difference.
			m_prevSysTotal = cpuTotal;
			m_prevSysIdle = cpuIdle;
			m_prevProcTotal = procTotal;
			return false;
		}

		if (cpuTotal == m_prevSysTotal) {
			// Not enough time has passed, disacard this measurement.
			return false;
		}

		cpuUtilization = 1.0 - ((double)(cpuIdle - m_prevSysIdle)) / (cpuTotal - m_prevSysTotal);
		if (procTotal != m_prevProcTotal) {
			processUtilization = (double) (procTotal - m_prevProcTotal) / (double) (cpuTotal - m_prevSysTotal);
			m_prevProcTotal = procTotal;
		}
		m_prevSysTotal = cpuTotal;
		m_prevSysIdle = cpuIdle;

		return true;
	}

	bool CalculateThreadsUtilization() {
		if (m_auxiliaryCounterFrequency == 0.) {
			return false;
		}
		auto monitors = m_monitoringThreads.load();
		if (monitors->empty()) {
			return false;
		}
		LARGE_INTEGER qpc;
		if (::QueryPerformanceCounter(&qpc) == FALSE) {
			return false;
		}
		if (qpc.QuadPart == m_prevQpcThread) {
			return false;
		}
		uint64_t cycleCount(0);
		for (auto & it : *monitors) {
			auto info = it.second;
			if (!info->handle) {
				continue;
			}
			if (::QueryThreadCycleTime(reinterpret_cast<HANDLE>(info->handle), &cycleCount) == FALSE) {
				continue;
			}
			if (info->prevCycleCount != 0) {
				info->cpuUtilization.store((double) (cycleCount - info->prevCycleCount) / m_auxiliaryCounterFrequency / (double) (qpc.QuadPart - m_prevQpcThread) * (double) m_performanceCounterFrequency);
			}
			info->prevCycleCount = cycleCount;
		}
		m_prevQpcThread = qpc.QuadPart;
		return true;
	}

#else

	struct CpuStat {
		CpuStat(uint64_t u, uint64_t n, uint64_t s, uint64_t i0, uint64_t i1, uint64_t i2, uint64_t i3, uint64_t z0) :
			user(u), nice(n), system(s), idle(i0), iowait(i1), irq(i2), softirq(i3), z(z0) {}
		CpuStat() :user(0), nice(0), system(0), idle(0), iowait(0), irq(0), softirq(0), z(0){}
		uint64_t user, nice, system, idle, iowait, irq, softirq, z;
		uint64_t sum() {
			return user + nice + system + idle + iowait + irq + softirq + z;
		}
		bool is_unset() const {
			return user == 0 && nice == 0 && system == 0 && idle == 0 && iowait == 0 && irq == 0 && softirq == 0 && z == 0;
		}
		int read(FILE *f) {
			int scanned_fields = fscanf(f, "cpu %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64, &user, &nice, &system, &idle, &iowait, &irq, &softirq, &z);
			return scanned_fields < 8 ? -1 : +1;
		}
		CpuStat operator-(const CpuStat& stat) {
			return CpuStat(user - stat.user, nice - stat.nice, system - stat.system, idle - stat.idle, iowait - stat.iowait, irq - stat.irq, softirq - stat.softirq, z - stat.z);
		}
	};
	CpuStat m_prev_cpu_stat;

	void Initialize() {

	}

	bool CalculateCPULoad(double &cpuUtilization, double &processUtilization) {

		FILE *f = fopen("/proc/stat", "rb");

		cpuUtilization = 0.0;
		processUtilization = 0.0;

		if (f == nullptr) {
			return false;
		}
		VS_SCOPE_EXIT { fclose(f); };

		CpuStat cpu_stat;
		if (cpu_stat.read(f) < 0) {
			return false;
		}

		if (m_prev_cpu_stat.is_unset())
		{
			// First call, previous values not known, can't compute the difference.
			m_prev_cpu_stat = cpu_stat;
			return false;
		}

		auto diff = cpu_stat - m_prev_cpu_stat;
		if (diff.sum() == 0)
		{
			// Not enough time has passed, disacard this measurement.
			return false;
		}

		cpuUtilization = 1.0 - (double)diff.idle / diff.sum();
		m_prev_cpu_stat = cpu_stat;
		return true;
	}

	bool CalculateThreadsUtilization() {
		return false;
	}

#endif

	std::atomic<double> m_cpu_load = ATOMIC_VAR_INIT(0.);
	std::atomic<double> m_process_load = ATOMIC_VAR_INIT(0.);
	vs::event m_stop_event;
	// It is important to keep m_thread as the last member because it may
	// start to execute before later members are initialized.
#if defined(_MSC_VER) && _MSC_VER < 1900
	boost::thread m_thread;
#else
	std::thread m_thread;
#endif
	vs::atomic_shared_ptr<mapMonitoringThreads> m_monitoringThreads;
};

VS_PerformanceMonitor::VS_PerformanceMonitor() {}
VS_PerformanceMonitor::~VS_PerformanceMonitor() {}

void VS_PerformanceMonitor::Start() {
	if (!m_impl)
		m_impl = vs::make_unique<Impl>();
}

void VS_PerformanceMonitor::Stop() {
	m_impl.reset();
}

double VS_PerformanceMonitor::GetTotalProcessorTime() const {
	return m_impl ? m_impl->GetTotalProcessorTime() : 0.;
}

double VS_PerformanceMonitor::GetTotalProcessTime() const {
	return m_impl ? m_impl->GetTotalProcessTime() : 0.;
}

double VS_PerformanceMonitor::GetTotalThreadTime(int32_t tid) const{
	return m_impl ? m_impl->GetTotalThreadTime(tid) : 0.;
}

void VS_PerformanceMonitor::AddMonitoringThread(int32_t tid)
{
	if (!m_impl) {
		return;
	}
	m_impl->AddMonitoringThread(tid);
}

void VS_PerformanceMonitor::RemoveMonitoringThread(int32_t tid)
{
	if (!m_impl) {
		return;
	}
	m_impl->RemoveMonitoringThread(tid);
}
