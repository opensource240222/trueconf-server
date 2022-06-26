#pragma once
#include "../transport/Router/VS_TransportRouterServiceTypes.h"
#include "../transport/typedefs.h"
#include "std-generic/cpplib/macro_utils.h"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/variant.hpp>
#include <boost/signals2.hpp>

#include <memory>
#include <mutex>
#include <queue>
#include <atomic>

namespace tc3_test
{
class FakeRouter;

class TransportRouterCallService_Fake : public VS_TransportRouter_CallService
{
	using ResponseNotifyMethodT = boost::variant<std::promise<VS_RouterMessage*>, ResponseCallBackT>;

	struct WaitingForResponseT
	{
		WaitingForResponseT(WaitingForResponseT&&src) :service_name(std::move(src.service_name)), notify_method(std::move(src.notify_method)), time_of_death(std::move(src.time_of_death))
		{}
		VS_FORWARDING_CTOR3(WaitingForResponseT, service_name, notify_method, time_of_death) {}
		std::string service_name;
		ResponseNotifyMethodT notify_method;
		std::chrono::steady_clock::time_point	time_of_death;
	};
	class notify_visitor : public boost::static_visitor<>
	{
		VS_RouterMessage *mess;
	public:
		notify_visitor(VS_RouterMessage *m) :mess(m)
		{}
		void operator()(std::promise<VS_RouterMessage*> &p) const
		{
			p.set_value(mess);
		}
		void operator()(ResponseCallBackT &cb) const
		{
			cb(mess);
		}
	};
	boost::multi_index_container<
		WaitingForResponseT,
		boost::multi_index::indexed_by<
		boost::multi_index::ordered_unique<boost::multi_index::tag<struct name>, boost::multi_index::member<WaitingForResponseT, std::string, &WaitingForResponseT::service_name>>,
		boost::multi_index::ordered_non_unique<boost::multi_index::tag<struct expiration>, boost::multi_index::member<WaitingForResponseT, std::chrono::steady_clock::time_point, &WaitingForResponseT::time_of_death>>
		>
	> m_waiting_for_response_storage;

	std::shared_ptr<FakeRouter> router_;
	boost::signals2::connection waiting_response_processing_conn_;
	bool PrepareAndStoreRequestResponse(VS_RouterMessage *m, ResponseNotifyMethodT &&notify, RequestLifeTimeT &&lt);
	bool TryWaitingResponseProcessing(VS_RouterMessage*mess);
public:
	static std::atomic<size_t> cout_of_instanses_;

	TransportRouterCallService_Fake(const std::shared_ptr<FakeRouter> &router);
	~TransportRouterCallService_Fake();

	bool			CallServiceSendMes(VS_RouterMessage *mes) override;;
	bool			CallServicePostMes(VS_RouterMessage *mes) override;
	bool			CallServiceRequestResponse(VS_RouterMessage *mess, ResponseCallBackT&&resp_cb, RequestLifeTimeT&&req_life_time) override;
	ResponseFutureT CallServiceRequestResponse(VS_RouterMessage*mess, RequestLifeTimeT&&req_life_time) override;
	bool			AddService(const char *serviceName, VS_TransportRouterServiceBase *service, bool isPermittedAll = false) override;
	bool			RemoveService(const char *serviceName) override;
	bool			BaseServiceGetStatistics(VS_TransportRouterStatistics *stat) override;

	bool			BaseServiceIsThereEndpoint(const char *endpoint) override;
	bool			BaseServiceDisconnectEndpoint(const char *endpoint) override;
	bool			BaseServiceFullDisconnectEndpoint(const char *endpoint) override;
	bool			BaseServiceFullDisconnectAllEndpoints(const CheckEndpointPredT &pred) override;
	bool			BaseServiceAddMessageStatistics(const char *name, unsigned long body) override;
	bool			AuthorizeClient(const char *uid, const char *new_uid) override;
	bool			UnauthorizeClient(const char *uid) override;
	std::string		GetCIDByUID(const char *uid) override;
	bool			GetIPByCID(const char* cid, std::string& ip) override;
	void			ShowPeriod() override;
};

class TransportRouterService_fake : public VS_TransportRouterService_Implementation
{
	std::queue<VS_RouterMessage*> msg_queue_;
	std::shared_ptr<FakeRouter> router_;
	std::mutex	recursion_lock_;
public:
	static std::atomic<size_t> cout_of_instanses_;

	TransportRouterService_fake(VS_TransportRouterServiceBase *trs, const char *endpointName, const char*service_name, const std::shared_ptr<FakeRouter> &router);
	~TransportRouterService_fake();

	void ForwardMsg(VS_RouterMessage *m);

	int ReceiveMes(std::unique_ptr<VS_RouterMessage>& mes, std::chrono::steady_clock::duration& wait_time, bool allAdopted) override;
};
}
