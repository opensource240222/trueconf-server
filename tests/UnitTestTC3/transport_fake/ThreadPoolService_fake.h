#pragma once
#include "transport/Router/VS_TransportRouterServiceBase.h"
#include <memory>
#include <queue>
#include <map>

/**

*/
class VS_PoolThreadsTask;
namespace tc3_test
{
class FakeRouter;
class TransportRouterService_fake;
class Endpoint;
class ThreadPoolService_fake : public VS_TransportRouterServiceBase
{
	TransportRouterService_fake				*impl_ = 0;
	std::weak_ptr<Endpoint>					endpoint_;
	class Task
	{
		VS_PoolThreadsTask *task_srv_ = nullptr;
		std::weak_ptr<Endpoint> endpoint_;
	public:
		Task(std::unique_ptr<VS_PoolThreadsTask> &&, const std::shared_ptr<Endpoint>&);
		Task(Task&&t);
		~Task();
		void operator()();
	};

	std::queue<Task> tasks_;
	std::map<VS_PoolThreadsTask*, Task> pending_tasks_;


public:
	ThreadPoolService_fake(const char *endpoint, const std::shared_ptr<FakeRouter>&router, const std::shared_ptr<Endpoint>&);
	~ThreadPoolService_fake();
	bool Init(const char* our_endpoint, const char* our_service, const bool permittedAll = false) override;
	bool Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;

	TransportRouterService_fake* GetImpl();
};
}
