
#ifndef BALANCING_MODULE_H
#define BALANCING_MODULE_H

#include "VS_LoadBalancer.h"
#include "std-generic/cpplib/string_view.h"

namespace balancing_module
{

	struct ThreadInfo
	{
		int32_t id;
		balancing_module::Thread type;
		uint64_t startTm = 0;
		std::atomic<uint64_t> overallTm { 0 };
		ThreadInfo(int32_t tid, balancing_module::Thread t) : id(tid), type(t) { };
	};

	typedef std::vector<std::shared_ptr<balancing_module::ThreadInfo> /* thread info */> arrayThreadBalancing;
	typedef std::map<balancing_module::Thread /* thread type */, std::vector<double> /* load */ > mapThreadUtilization;

	extern string_view GetModuleName(balancing_module::Type moduleType);
	extern string_view GetThreadName(balancing_module::Thread threadType);

	class BalancingModule
	{

	public:

		BalancingModule(balancing_module::Type moduleType);
		virtual ~BalancingModule();
		virtual void NotifyResourceUtilization();
		balancing_module::Type GetType() const;
		const mapThreadUtilization& GetResourceUtilization();
		void CheckResourceUtilization(uint64_t tm);

	protected:

		void RegisterThread(balancing_module::Thread threadType);
		void UnregisterThread();
		void RegisterPseudoThreads(balancing_module::Thread threadType, int32_t numThreads);
		void UnregisterPseudoThreads(balancing_module::Thread threadType);
		void BeginWorkThread(balancing_module::Thread threadType);
		void EndWorkThread(balancing_module::Thread threadType);

	private:

		balancing_module::Type m_moduleType;
		vs::atomic_shared_ptr<arrayThreadBalancing> m_threads;
		vs::atomic_shared_ptr<arrayThreadBalancing> m_threadsPseudo;
		mapThreadUtilization m_utilizationThreads;

	};

}

#endif /* BALANCING_MODULE_H */