#ifdef _WIN32
#include "TransportRouterService_Fakes.h"
#include "FakeRouter.h"

#include "../std/cpplib/VS_SimpleStr.h"
#include "std-generic/cpplib/string_view.h"
#include <cassert>

namespace tc3_test
{
std::atomic<size_t> TransportRouterCallService_Fake::cout_of_instanses_ = 0;
TransportRouterCallService_Fake::TransportRouterCallService_Fake(const std::shared_ptr<FakeRouter> &router) : router_(router)
{
	++cout_of_instanses_;
	threadId = GetCurrentThreadId();
	waiting_response_processing_conn_ = router_->RegisterExtHandler([&](VS_RouterMessage*m) {
		return TryWaitingResponseProcessing(m);
	});
}
TransportRouterCallService_Fake::~TransportRouterCallService_Fake()
{
	waiting_response_processing_conn_.disconnect();
	--cout_of_instanses_;
}
bool TransportRouterCallService_Fake::CallServiceSendMes(VS_RouterMessage *mes)
{
	router_->SendMsg(mes);
	return true;
}
bool TransportRouterCallService_Fake::CallServicePostMes(VS_RouterMessage *mes)
{
	router_->SendMsg(mes);
	return true;
}
bool TransportRouterCallService_Fake::CallServiceRequestResponse(VS_RouterMessage *mess, ResponseCallBackT&&resp_cb, RequestLifeTimeT&&req_life_time)
{
	return PrepareAndStoreRequestResponse(mess, std::move(resp_cb), std::move(req_life_time));
}
ResponseFutureT TransportRouterCallService_Fake::CallServiceRequestResponse(VS_RouterMessage*mess, RequestLifeTimeT&&req_life_time)
{
	std::promise<VS_RouterMessage*> p;
	auto res = p.get_future();
	if (PrepareAndStoreRequestResponse(mess, std::move(p), std::move(req_life_time)))
	{
		router_->ProcessQueue();
		return res;
	}
	return ResponseFutureT();
}
bool TransportRouterCallService_Fake::PrepareAndStoreRequestResponse(VS_RouterMessage *m, ResponseNotifyMethodT &&notify, RequestLifeTimeT &&lt)
{
	const static string_view prefix = "REQRESP: ";
	std::string name { prefix };
	name += std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
	const auto sz = name.length();
	auto counter = 0;
	while (m_waiting_for_response_storage.count(name) > 0)
	{
		if (name.length() == sz)
		{
			name += "#";
			name += std::to_string(counter);
		}
		else
		{
			name.replace(name.begin() + sz + 1, name.end(), std::to_string(counter));
			name.insert(name.at(sz), std::to_string(counter));
		}
		++counter;
	}
	bool bres = m->SetSrcService(name.c_str());
	string_view src_service = m->SrcService();
	string_view dst_service = m->DstService();
	const RequestLifeTimeT msg_timelife = std::chrono::milliseconds(m->TimeLimit());
	if (!CallServicePostMes(m))
		return false;
	m_waiting_for_response_storage.emplace(std::move(name), std::move(notify), std::chrono::steady_clock::now() + std::max(lt, msg_timelife));
	return true;
}
bool TransportRouterCallService_Fake::TryWaitingResponseProcessing(VS_RouterMessage *mess)
{
	if (!mess || !mess->DstService() || !mess->DstServer() /*|| !IsOurEndpointName(mess->DstServer())*/)
		return false;
	const auto it = m_waiting_for_response_storage.find(mess->DstService());
	if (m_waiting_for_response_storage.end() == it)
		return false;
	m_waiting_for_response_storage.modify(it, [mess](WaitingForResponseT &item) {boost::apply_visitor(notify_visitor(mess), item.notify_method);});
	m_waiting_for_response_storage.erase(it);
	return true;
}

bool TransportRouterCallService_Fake::AddService(const char *serviceName, VS_TransportRouterServiceBase *service, bool isPermittedAll)
{
	assert(false);
	return false;
}
bool TransportRouterCallService_Fake::RemoveService(const char *serviceName)
{
	assert(false);
	return false;
}
bool TransportRouterCallService_Fake::BaseServiceGetStatistics(VS_TransportRouterStatistics *stat)
{
	assert(false);
	return false;
}

bool TransportRouterCallService_Fake::BaseServiceIsThereEndpoint(const char *endpoint)
{
	return router_->IsEndpointExist(endpoint);
}
bool TransportRouterCallService_Fake::BaseServiceDisconnectEndpoint(const char *endpoint)
{
	return router_->DisconnectEndpoint(endpoint);;
}
bool TransportRouterCallService_Fake::BaseServiceFullDisconnectEndpoint(const char *endpoint)
{
	assert(false);
	return false;
}
bool TransportRouterCallService_Fake::BaseServiceFullDisconnectAllEndpoints(const CheckEndpointPredT &)
{
	assert(false);
	return false;
}
bool TransportRouterCallService_Fake::BaseServiceAddMessageStatistics(const char *name, unsigned long body)
{
	return true;
}
bool TransportRouterCallService_Fake::AuthorizeClient(const char *uid, const char *new_uid)
{
	assert(false);
	return false;
}
bool TransportRouterCallService_Fake::UnauthorizeClient(const char *uid)
{
	assert(false);
	return false;
}
std::string TransportRouterCallService_Fake::GetCIDByUID(const char* uid)
{
	assert(false);
	return {};
}
bool TransportRouterCallService_Fake::GetIPByCID(const char* cid, std::string& ip)
{
	assert(false);
	return false;
}

void TransportRouterCallService_Fake::ShowPeriod()
{
	assert(false);
}
std::atomic<size_t> TransportRouterService_fake::cout_of_instanses_ = 0;
TransportRouterService_fake::TransportRouterService_fake(VS_TransportRouterServiceBase *trs, const char *endpointName, const char*service_name, const std::shared_ptr<FakeRouter> &router) : VS_TransportRouterService_Implementation(trs, endpointName, service_name, new TransportRouterCallService_Fake(router)), router_(router)
{
	++cout_of_instanses_;
}
TransportRouterService_fake::~TransportRouterService_fake()
{
	delete VS_TransportRouterServiceBase_Implementation::tr;
	--cout_of_instanses_;
}

void TransportRouterService_fake::ForwardMsg(VS_RouterMessage*m)
{
	msg_queue_.push(m);
}
int TransportRouterService_fake::ReceiveMes(std::unique_ptr<VS_RouterMessage>& mes, std::chrono::steady_clock::duration& /*wait_time*/, bool allAdopted)
{
	std::unique_lock<std::mutex> l(recursion_lock_, std::try_to_lock);
	if (!l.owns_lock())
		return -1;
	if (msg_queue_.empty())
	{
		router_->ProcessQueue();
		if (msg_queue_.empty())
			return -1;
	}
	mes.reset(msg_queue_.front());
	msg_queue_.pop();
	FakeRouter::ChangeCurrentEndpoint(endpointName);
	return 1;
}
}
#endif