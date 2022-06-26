
#include "BalancingModule.h"
#include "std/cpplib/ThreadUtils.h"
#include "std/cpplib/VS_PerformanceMonitor.h"

#include <algorithm>

namespace balancing_module
{
	string_view GetModuleName(balancing_module::Type moduleType)
	{
		switch (moduleType) {
		case Type::decompressor :
			return "Decompressor";
		case Type::mediasynch:
			return "Mediasynch";
		case Type::mixer:
			return "Mixer";
		case Type::compressor:
			return "Compressor";
		case Type::transcoder:
			return "Transcoder";
		default:
			return "Unknown";
		}
	}

	string_view GetThreadName(balancing_module::Thread threadType)
	{
		switch (threadType) {
		case Thread::decoder:
			return "Decoder";
		case Thread::encoder:
			return "Encoder";
		case Thread::synch:
			return "Synch";
		case Thread::layout:
			return "Layout";
		case Thread::resampler:
			return "Resample";
		case Thread::control:
			return "Control";
		default:
			return "Unknown";
		}
	}
}

using namespace balancing_module;

BalancingModule::BalancingModule(balancing_module::Type moduleType) : m_moduleType(moduleType)
																	, m_threads(std::make_shared<arrayThreadBalancing>())
																	, m_threadsPseudo(std::make_shared<arrayThreadBalancing>())
{

}

BalancingModule::~BalancingModule()
{

}

void BalancingModule::NotifyResourceUtilization()
{

}

void BalancingModule::CheckResourceUtilization(uint64_t tm)
{
	mapThreadUtilization baseUtilization;
	m_utilizationThreads.clear();
	{
		auto threads = m_threads.load();
		for (const auto & it : *threads) {
			double load = VS_PerformanceMonitor::Instance().GetTotalThreadTime(it->id);
			m_utilizationThreads[it->type].push_back(load);
			auto &util = baseUtilization[it->type];
			if (util.empty()) {
				util.push_back(load);
			}
		}
	}
	{
		auto threads = m_threadsPseudo.load();
		for (auto & it : *threads) {
			uint64_t overallBusyTask(0);
			overallBusyTask = it->overallTm.exchange(overallBusyTask);
			double load = (double) overallBusyTask / (double) tm * 100.0;
			auto &util = baseUtilization[it->type];
			if (!util.empty()) {
				load -= util.front();
				load = std::max(load, 0.0);
			}
			m_utilizationThreads[it->type].push_back(load);
		}
	}
}

balancing_module::Type BalancingModule::GetType() const
{
	return m_moduleType;
}

const mapThreadUtilization& BalancingModule::GetResourceUtilization()
{
	return m_utilizationThreads;
}

void BalancingModule::RegisterThread(balancing_module::Thread threadType)
{
	int32_t tid(vs::GetThreadID());
	if (!tid) {
		return;
	}
	auto threads = m_threads.load();
	auto new_threads = std::make_shared<arrayThreadBalancing>();
	do {
		new_threads->clear();
		for (const auto & it : *threads) {
			if (tid == it->id) {
				return; /// nothing to do, duplicate tid
			}
			new_threads->push_back(it);
		}
		new_threads->emplace_back(std::make_shared<balancing_module::ThreadInfo>(tid, threadType));
	} while (!m_threads.compare_exchange_strong(threads, new_threads));
	VS_PerformanceMonitor::Instance().AddMonitoringThread(tid);
}

void BalancingModule::UnregisterThread()
{
	int32_t tid(vs::GetThreadID());
	if (!tid) {
		return;
	}
	auto threads = m_threads.load();
	auto new_threads = std::make_shared<arrayThreadBalancing>();
	do {
		*new_threads = *threads;
		for (auto it = new_threads->begin(); it != new_threads->end(); ++it) {
			if ((*it)->id == tid) {
				new_threads->erase(it);
				break;
			}
		}
	} while (!m_threads.compare_exchange_strong(threads, new_threads));
	VS_PerformanceMonitor::Instance().RemoveMonitoringThread(tid);
}

void BalancingModule::RegisterPseudoThreads(balancing_module::Thread threadType, int32_t numThreads)
{
	auto threads = m_threadsPseudo.load();
	auto new_threads = std::make_shared<arrayThreadBalancing>();
	do {
		*new_threads = *threads;
		for (int32_t i = 0; i < numThreads; i++) {
			int32_t tid = static_cast<int32_t>(std::hash<int32_t>()(threadType)) + i;
			new_threads->emplace_back(std::make_shared<balancing_module::ThreadInfo>(tid, threadType));
		}
	} while (!m_threadsPseudo.compare_exchange_strong(threads, new_threads));
}

void BalancingModule::UnregisterPseudoThreads(balancing_module::Thread threadType)
{
	auto threads = m_threadsPseudo.load();
	auto new_threads = std::make_shared<arrayThreadBalancing>();
	do {
		*new_threads = *threads;
		for (auto it = new_threads->begin(); it != new_threads->end(); ) {
			if ((*it)->type == threadType) {
				it = new_threads->erase(it);
			}
			else {
				++it;
			}
		}
	} while (!m_threadsPseudo.compare_exchange_strong(threads, new_threads));
}

void BalancingModule::BeginWorkThread(balancing_module::Thread threadType)
{
	auto tm = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
	auto threads = m_threadsPseudo.load();
	for (auto & it : *threads) {
		if (it->type != threadType) {
			continue;
		}
		it->startTm = tm;
	}
}

void BalancingModule::EndWorkThread(balancing_module::Thread threadType)
{
	auto tm = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
	auto threads = m_threadsPseudo.load();
	for (auto & it : *threads) {
		if (it->type != threadType) {
			continue;
		}
		it->overallTm += tm - it->startTm;
		it->startTm = 0;
	}
}
