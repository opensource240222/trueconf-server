#include "ServiceV1Adaptor.h"
#include "IService.h"
#include "Router.h"
#include "../../transport/Message.h"
#include "../../transport/Router/VS_RouterMessage.h"
#include "../../acs/VS_AcsDefinitions.h"
#include "std/cpplib/spin_mutex.h"
#include "std/cpplib/ThreadUtils.h"
#include "std-generic/clib/vs_time.h"

#include "std-generic/compat/iomanip.h"
#include "std-generic/compat/memory.h"
#include "std-generic/compat/condition_variable.h"
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

namespace transport {

namespace {

std::atomic<uint64_t> g_last_task_id(0);

}

class ServiceV1Adaptor
	: public IService
	, public VS_TransportRouterServiceBase::Impl
	, public VS_PoolThreadsTask::Impl
{
public:
	friend bool InstallV1Service_impl(
		string_view name,
		VS_TransportRouterServiceBase* srv_base,
		VS_PoolThreadsTask* pt_task,
		bool withOwnThread,
		const std::shared_ptr<Router>& router);
	bool Uninstall();

	ServiceV1Adaptor(
		string_view name,
		VS_TransportRouterServiceBase* srv_base,
		VS_PoolThreadsTask* pt_task,
		bool withOwnThread,
		const std::shared_ptr<Router>& router);
	~ServiceV1Adaptor();

	// IService
	bool ProcessMessage(Message&& message) override;
	string_view GetName() override;
	void FillMonitorStruct(Monitor::TmReply::Service &tmreply_service) override;
	void OnEndpointConnect(bool is_client_ep, EndpointConnectReason reason, string_view cid, string_view uid) override;
	void OnEndpointDisconnect(bool is_client_ep, EndpointConnectReason reason, string_view cid, string_view uid) override;
	void OnEndpointIP(string_view uid, string_view ip) override;

	// VS_TransportRouterServiceBase::Impl
	bool SendMes(VS_RouterMessage* mes) override;
	bool PostMes(VS_RouterMessage* mes) override;
	bool RequestResponse(VS_RouterMessage* mes, ResponseCallBackT&& cb, RequestLifeTimeT&& life_time) override;
	ResponseFutureT RequestResponse(VS_RouterMessage* mes, RequestLifeTimeT&& life_time) override;
	const char* OurEndpoint() const override;
	const char* OurService() const override;
	bool GetStatistics(struct VS_TransportRouterStatistics* stat) override;
	bool IsThereEndpoint(const char* endpoint) override;
	void DisconnectEndpoint(const char* endpoint) override;
	void FullDisconnectEndpoint(const char* endpoint) override;
	void FullDisconnectAllEndpoints() override;
	bool AuthorizeClient(const char* uid, const char* new_uid) override;
	bool UnauthorizeClient(const char* uid) override;
	bool IsAuthorized(const char* uid) override;
	std::string GetCIDByUID(const char* uid) override;
	bool GetIPByCID(const char* cid, std::string& ip) override;
	bool PutTask(VS_PoolThreadsTask* task, const char* nameExtension, unsigned lifetimeSec, unsigned priority) override;

	// VS_TransportRouterService::Impl
	bool SetThread(VS_TransportRouterServiceBase::Impl* instance) override;
	void ResetThread() override;
	int ReceiveMes(std::unique_ptr<VS_RouterMessage>& mes, bool allAdopted) override;
	int ReceiveMes(std::unique_ptr<VS_RouterMessage>& mes, std::chrono::steady_clock::duration& wait_time, bool allAdopted) override;

	// VS_PoolThreadsTask::Impl
	void FinishTask() override;

private:
	void SetThreadName();

	std::string m_name;
	VS_TransportRouterServiceBase* m_srv_base;
	bool m_withOwnThread;
	VS_PoolThreadsTask* m_pt_task;
	std::shared_ptr<Router> m_router;

	std::atomic<bool> m_should_run;
	std::thread m_thread; // Service thread (VS_TransportRouterService::Thread())

	std::mutex m_mutex;
	vs::condition_variable m_cv;
	std::queue<Message> m_msg_queue;
	MessageStats m_send_stats, m_recv_stats;
};

bool InstallV1Service_impl(
	string_view name,
	VS_TransportRouterServiceBase* srv_base,
	VS_PoolThreadsTask* pt_task,
	bool withOwnThread,
	const std::shared_ptr<Router>& router)
{
	if (srv_base->imp)
		return false;

	auto adaptor = vs::make_unique<ServiceV1Adaptor>(name, srv_base, pt_task, withOwnThread, router);
	if (pt_task)
		pt_task->imp = adaptor.get();
	auto adapter_raw = adaptor.get();
	srv_base->imp = std::move(adaptor);

	if (!router->AddService(adapter_raw) || !srv_base->Init(router->EndpointName().c_str(), adapter_raw->m_name.c_str()))
	{
		if (pt_task)
			pt_task->imp = nullptr;
		srv_base->imp.reset();
		return false;
	}
	return true;
}

bool InstallV1Service(VS_TransportRouterServiceBase* srv, string_view name, bool withOwnThread, const std::shared_ptr<Router>& router)
{
	return InstallV1Service_impl(
		name,
		srv,
		nullptr,
		withOwnThread,
		router);
}

bool UninstallV1Service(VS_TransportRouterServiceBase* srv)
{
	auto adaptor = dynamic_cast<ServiceV1Adaptor*>(srv->imp.get());
	return adaptor ? adaptor->Uninstall() : false;
}

bool ServiceV1Adaptor::Uninstall()
{
	if (!m_router->RemoveService(m_name))
		return false;
	m_srv_base->Destroy(m_router->EndpointName().c_str(), m_name.c_str());
	return true;
}

ServiceV1Adaptor::ServiceV1Adaptor(
	string_view name,
	VS_TransportRouterServiceBase* srv_base,
	VS_PoolThreadsTask* pt_task,
	bool withOwnThread,
	const std::shared_ptr<Router>& router
)
	: m_name(name)
	, m_srv_base(srv_base)
	, m_withOwnThread(withOwnThread)
	, m_pt_task(pt_task)
	, m_router(router)
	, m_should_run(false)
{
	assert(m_srv_base);
}

void ServiceV1Adaptor::SetThreadName()
{
	std::string name;

	string_view srv_name(m_name);
	if (srv_name.substr(0, 8) == "PT: 001 ")
	{
		// PutTask temporary service
		srv_name.remove_prefix(8 + 9); // Remove prefix and task id

		const auto name_end_pos = srv_name.find(" - ");
		if (name_end_pos != srv_name.npos)
			srv_name.remove_suffix(srv_name.size() - name_end_pos);

		name += "PT:";
		name += srv_name;
	}
	else
	{
		name += "S:";
		name += srv_name;
	}

	if (name.size() > 15)
		name.resize(15);
	vs::SetThreadName(name.c_str());
}

ServiceV1Adaptor::~ServiceV1Adaptor()
{
	// Notify ReceiveMes() that it should stop waiting.
	m_should_run.store(false, std::memory_order_release);
	m_cv.notify_all();

	if (m_thread.joinable())
	{
		if (m_thread.get_id() == std::this_thread::get_id())
		{
			// We are called from the service thread.
			// It is safe to destroy this object now because our code won't touch this object.
			m_thread.detach();
		}
		else
		{
			// Running thread may be using this object right now, so we have to wait for the thread to finish.
			m_thread.join();
		}
	}
}

bool ServiceV1Adaptor::ProcessMessage(Message&& message)
{
	m_recv_stats.Update(message.Size());

	// 1. Services that has own thread will poll for
	//    messages themselves (using ReceiveMes()).
	if (m_withOwnThread)
	{
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_msg_queue.emplace(std::move(message));
		}
		m_cv.notify_all();
	}
	else
	{
		// In this case we can forward the message directly to the service.
		return m_srv_base->Processing(vs::make_unique<VS_RouterMessage>(std::move(message)));
	}

	// Service doesn't want to handle messages at all, just drop the message.
	return false;
}

string_view ServiceV1Adaptor::GetName()
{
	return m_name;
}

void ServiceV1Adaptor::FillMonitorStruct(Monitor::TmReply::Service& tmreply_service)
{
	//tmreply_service.service_type = TM_TYPE_PERIODIC_SERVICE;
	tmreply_service.service_name = std::string(GetName());
	tmreply_service.send_stats = m_send_stats;
	tmreply_service.recv_stats = m_recv_stats;
}

static_assert(static_cast<int>(EndpointConnectReason::hs_error) == VS_PointParams::CR_HSERROR, "");
static_assert(static_cast<int>(EndpointConnectReason::timeout) == VS_PointParams::CR_TIMEOUT, "");
static_assert(static_cast<int>(EndpointConnectReason::unknown) == VS_PointParams::CR_UNKNOWN, "");
static_assert(static_cast<int>(EndpointConnectReason::requested) == VS_PointParams::CR_REQUESTED, "");
static_assert(static_cast<int>(EndpointConnectReason::incoming) == VS_PointParams::CR_INCOMING, "");

void ServiceV1Adaptor::OnEndpointConnect(bool is_client_ep, EndpointConnectReason reason, string_view cid, string_view uid)
{
	auto cid_s = std::string(cid);
	auto uid_s = std::string(uid);
	VS_PointParams prm;
	prm.type = is_client_ep ? VS_PointParams::PT_CLIENT : VS_PointParams::PT_SERVER;
	prm.ver = 0; // unused
	prm.reazon = static_cast<int>(reason);
	prm.cid = cid_s.c_str();
	prm.uid = uid_s.c_str();
	m_srv_base->OnPointConnected_Event(&prm);
}

void ServiceV1Adaptor::OnEndpointDisconnect(bool is_client_ep, EndpointConnectReason reason, string_view cid, string_view uid)
{
	auto cid_s = std::string(cid);
	auto uid_s = std::string(uid);
	VS_PointParams prm;
	prm.type = is_client_ep ? VS_PointParams::PT_CLIENT : VS_PointParams::PT_SERVER;
	prm.ver = 0; // unused
	prm.reazon = static_cast<int>(reason);
	prm.cid = cid_s.c_str();
	prm.uid = uid_s.c_str();
	m_srv_base->OnPointDisconnected_Event(&prm);
}

void ServiceV1Adaptor::OnEndpointIP(string_view uid, string_view ip)
{
	auto uid_s = std::string(uid);
	auto ip_s = std::string(ip);
	m_srv_base->OnPointDeterminedIP_Event(uid_s.c_str(), ip_s.c_str());
}

bool ServiceV1Adaptor::SendMes(VS_RouterMessage* mes)
{
	m_send_stats.Update(mes->Size());
	std::unique_ptr<VS_RouterMessage> message(mes);
	m_router->ProcessMessage(std::move(*message));
	return true;
}

bool ServiceV1Adaptor::PostMes(VS_RouterMessage* mes)
{
	std::unique_ptr<VS_RouterMessage> message(mes);
	m_router->PostMessage(std::move(*message));
	return true;
}

bool ServiceV1Adaptor::RequestResponse(VS_RouterMessage* mes, ResponseCallBackT&& cb, RequestLifeTimeT&& life_time)
{
	m_router->RequestResponse(std::move(*mes), [cb](Message&& message) {
		if (message.IsValid())
			cb(vs::make_unique<VS_RouterMessage>(std::move(message)).get());
		else
			cb(nullptr);
	}, life_time);
	return true;
}

ResponseFutureT ServiceV1Adaptor::RequestResponse(VS_RouterMessage* mes, RequestLifeTimeT&& life_time)
{
	auto p = std::make_shared<std::promise<VS_RouterMessage*>>();
	ResponseFutureT f(p->get_future());
	m_router->RequestResponse(std::move(*mes), [p](Message&& message) {
		if (message.IsValid())
			p->set_value(new VS_RouterMessage(std::move(message)));
	}, life_time);
	return f;
}

const char* ServiceV1Adaptor::OurEndpoint() const
{
	return m_router->EndpointName().c_str();
}

const char* ServiceV1Adaptor::OurService() const
{
	return m_name.c_str();
}

bool ServiceV1Adaptor::GetStatistics(struct VS_TransportRouterStatistics* stat)
{
	stat->endpoints = m_router->GetEndpointCount();
	uint64_t total_read;
	uint64_t total_write;
	m_router->GetStatistics(total_read, total_write, stat->in_byterate, stat->out_byterate);
	stat->in_bytes = static_cast<double>(total_read);
	stat->out_bytes = static_cast<double>(total_write);
	return true;
}

bool ServiceV1Adaptor::IsThereEndpoint(const char* endpoint)
{
	return m_router->EndpointExists(endpoint);
}

void ServiceV1Adaptor::DisconnectEndpoint(const char* endpoint)
{
	m_router->DisconnectEndpoint(endpoint);
}

void ServiceV1Adaptor::FullDisconnectEndpoint(const char* endpoint)
{
	m_router->FullDisconnectEndpoint(endpoint);
}

void ServiceV1Adaptor::FullDisconnectAllEndpoints()
{
	m_router->FullDisconnectAll();
}

bool ServiceV1Adaptor::AuthorizeClient(const char* uid, const char* new_uid)
{
	return m_router->AuthorizeClient(uid, new_uid);
}

bool ServiceV1Adaptor::UnauthorizeClient(const char* uid)
{
	return m_router->UnauthorizeClient(uid);
}

bool ServiceV1Adaptor::IsAuthorized(const char* uid)
{
	return m_router->IsAuthorized(uid);
}

std::string ServiceV1Adaptor::GetCIDByUID(const char* uid)
{
	return m_router->GetClientIdByUserId(uid);
}

bool ServiceV1Adaptor::GetIPByCID(const char* cid, std::string& ip)
{
	ip = m_router->GetIpByClientId(cid);
	return !ip.empty();
}

bool ServiceV1Adaptor::PutTask(VS_PoolThreadsTask* task, const char* nameExtension, unsigned lifetimeSec, unsigned priority)
{
	const string_view name = nameExtension ? nameExtension : "unk";
	auto now(std::time(0));
	tm now_tm;
	std::ostringstream ss;
	ss << "PT: 001 "
		<< std::setw(8) << std::setfill('0') << ++g_last_task_id << ' '
		<< name.substr(0, 16) << " - "
		<< vs::put_time(localtime_r(&now, &now_tm), "%d/%m/%Y %H:%M:%S")
		;
	return InstallV1Service_impl(ss.str(), task, task, true, m_router) && task->SetThread();
}

bool ServiceV1Adaptor::SetThread(VS_TransportRouterServiceBase::Impl* instance)
{
	assert(instance == nullptr); // This functionality is not supported.

	if (m_thread.joinable())
		return false;

	m_should_run.store(true, std::memory_order_release);

	// The thread we start may begin executing before we assign new std::thread instance to m_thread.
	// This may result in this object being deleted (because the service Thread function is finished) before this thread will modify m_thread in which case it will write to freed memory.
	// Because of that we need to delay execution of the service Thread function until this thread is done modifying the object.
	auto start_mutex = vs::make_unique<vs::spin_mutex<>>();
	std::lock_guard<vs::spin_mutex<>> _(*start_mutex);
	m_thread = std::thread([this, start_mutex = std::move(start_mutex)]() mutable {
		SetThreadName();
		std::lock_guard<vs::spin_mutex<>> _(*start_mutex);
		m_srv_base->Thread();
	});
	return true;
}

void ServiceV1Adaptor::ResetThread()
{
	if (!m_thread.joinable())
		return;

	// Notify ReceiveMes() that it should stop.
	{
		// We have to set should_run flag under the mutex to ensure that thread
		// performing wait on m_cv won't get preempted after the check of the
		// flag but before wait start, in which case our notify_all would have been ignored.
		std::lock_guard<std::mutex> lock(m_mutex);
		m_should_run.store(false, std::memory_order_release);
	}
	m_cv.notify_all();
	m_thread.join();
}

int ServiceV1Adaptor::ReceiveMes(std::unique_ptr<VS_RouterMessage>& mes, bool allAdopted)
{
	std::chrono::steady_clock::duration wait_time = std::chrono::steady_clock::duration::max();
	return ReceiveMes(mes, wait_time, allAdopted);
}

int ServiceV1Adaptor::ReceiveMes(std::unique_ptr<VS_RouterMessage>& mes, std::chrono::steady_clock::duration& wait_time, bool /*allAdopted*/)
{
	using clock = std::chrono::steady_clock;

	std::unique_lock<std::mutex> lock(m_mutex);
	if (m_msg_queue.empty())
	{
		const auto end_time = clock::now()
			+ std::min<clock::duration>(wait_time, std::chrono::hours(24*365)); // Limit wait time because large value might cause overflow in STL.
		std::cv_status wait_result;
		while (true)
		{
			// Check if we should_run before starting a wait.
			if (!m_should_run.load(std::memory_order_acquire))
				return -1;
			wait_result = m_cv.wait_until(lock, end_time);

			if (!m_msg_queue.empty())
				break;

			if (wait_result == std::cv_status::timeout)
			{
				wait_time = clock::duration::zero();
				return -2;
			}
		}
		assert (!m_msg_queue.empty());
		// Wait was successful.
		wait_time = std::max(end_time - clock::now(), clock::duration::zero());
	}

	mes = vs::make_unique<VS_RouterMessage>(std::move(m_msg_queue.front()));
	m_msg_queue.pop();
	return 1;
}

void ServiceV1Adaptor::FinishTask()
{
	assert(m_pt_task); // This function shouldn't be called if wrapped service isn't a VS_PoolThreadsTask.

	Uninstall();
	delete m_srv_base;
}

}
