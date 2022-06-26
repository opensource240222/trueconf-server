#ifdef _WIN32
#include "ThreadPoolService_fake.h"
#include "TransportRouterService_Fakes.h"
#include "../../common/transport/Router/VS_PoolThreadsService.h"
#include "../std/cpplib/VS_Protocol.h"
#include "std-generic/cpplib/string_view.h"
#include "Endpoint.h"

#include <chrono>
#include <sstream>
#include <cassert>

namespace tc3_test
{
ThreadPoolService_fake::Task::Task(std::unique_ptr<VS_PoolThreadsTask> &&task_srv, const std::shared_ptr<Endpoint>&endpoint) : task_srv_(task_srv.get()), endpoint_(endpoint)
{
	std::stringstream ss;
	ss << "PT " << std::hex << this;	// service name;
	auto impl = new TransportRouterService_fake(task_srv_, endpoint->EndpointName(), ss.str().c_str(), endpoint->GetRouter());
	static_cast<VS_TransportRouterServiceBase*>(task_srv_)->imp.reset(impl);
	endpoint->RegisterService(ss.str(), std::move(task_srv),impl);
}

ThreadPoolService_fake::Task::Task(Task&&src) : task_srv_(src.task_srv_), endpoint_(std::move(src.endpoint_))
{
	src.task_srv_ = nullptr;
}
ThreadPoolService_fake::Task::~Task()
{
	if (!!task_srv_)
	{
		auto ep = endpoint_.lock();
		if (!ep)
			return;
		ep->UnregisterService(task_srv_->OurService());
	}
}
void ThreadPoolService_fake::Task::operator()()
{
	static_cast<VS_TransportRouterServiceBase*>(task_srv_)->Thread();
}

ThreadPoolService_fake::ThreadPoolService_fake(const char *ep_name, const std::shared_ptr<FakeRouter>&router, const std::shared_ptr<Endpoint> &ep) :endpoint_(ep)
{
	//create service;
	impl_ = new TransportRouterService_fake(this, ep_name, "POOL_THREADS", router);
	VS_TransportRouterServiceBase::imp.reset(impl_);
}
TransportRouterService_fake* ThreadPoolService_fake::GetImpl()
{
	return impl_;
}
ThreadPoolService_fake::~ThreadPoolService_fake()
{
}
bool ThreadPoolService_fake::Init(const char *our_endpoint, const char *our_service, const bool permittedAll)
{
	return true;
}

bool ThreadPoolService_fake::Processing(std::unique_ptr<VS_RouterMessage>&&mess)
{
	 string_view topic = mess->AddString();
	 if (topic == "KSAT_TUP_SDAERHT_LOOP_SV")
	 {
		 struct Head
		 {
			 unsigned   chkSum, sz, nameExtension;
			 VS_PoolThreadsTask   *task;		unsigned   lifetimeSec, priority;
		 };
		const auto head = reinterpret_cast<const Head*>(mess->Body());
		head->task->VS_PoolThreadsTask::imp = new VS_PoolThreadsTask_Implementation(head->task);
		tasks_.push(Task(std::unique_ptr<VS_PoolThreadsTask>(head->task),endpoint_.lock()));
		if (tasks_.size() > 1)
			return true;
		do
		{
			auto t = std::move(tasks_.front());
			auto it = pending_tasks_.insert({ head->task, std::move(t) });
			it.first->second();
			tasks_.pop();
		} while (!tasks_.empty());
	 }
	 else if (topic == "KSAT_DNE_SDAERHT_LOOP_SV")
	 {
		 const auto ptask = reinterpret_cast<VS_PoolThreadsTask* const *>(mess->Body());
		 delete (*ptask)->VS_PoolThreadsTask::imp;
		 pending_tasks_.erase(*ptask);
	 }
	return true;
}
}
#endif