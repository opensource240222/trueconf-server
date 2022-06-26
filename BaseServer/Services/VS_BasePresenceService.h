#ifndef	VS_BASE_PRESENCE_SERVICE_H
#define VS_BASE_PRESENCE_SERVICE_H
#include "transport/Router/VS_TransportRouterServiceBase.h"
#include "transport/Router/VS_TransportRouterServiceHelper.h"
#include "ServerServices/VS_SubscriptionHub.h"
#include "std-generic/cpplib/macro_utils.h"
#include "OfflineStatusCache.h"
#include <boost/shared_ptr.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/tag.hpp>

namespace
{
	namespace mi = boost::multi_index;
}

class VS_BasePresenceService :
	public VS_TransportRouterServiceReplyHelper,
	public VS_SubscriptionHub,
	public UsersStatusesInterface
{
private:
	void UpdateStatus_Method(const VS_FullID& source, const VS_Container& cnt);
	std::map<std::string, std::tuple<std::string/*src server*/, VS_Container/*container*/,std::chrono::steady_clock::time_point>> m_delay_until_unregister;
	struct RegistredCallID
	{
		VS_FORWARDING_CTOR2(RegistredCallID, call_id_, server_) {}
		std::string call_id_;
		std::string server_;
	};
	using RegistredCallIDSet = boost::multi_index_container<
		RegistredCallID,
		mi::indexed_by<
		mi::ordered_unique<mi::tag<struct by_call_id>, mi::member<RegistredCallID, std::string, &RegistredCallID::call_id_>>,
		mi::ordered_non_unique < mi::tag<struct by_server>, mi::member<RegistredCallID, std::string, &RegistredCallID::server_>>
		>
	>;
	RegistredCallIDSet m_registred_call_id;
	VS_InputSync m_sub_sync;
	OfflineStatusCache m_offline_statuses_cache;
	std::map<VS_FullID,std::set<std::string>> m_unsub_cache;
	std::set<std::string> m_check_offline_chat;

	bool IsRoamingAllowed(const char *server_name = 0) override;
	bool IsUplink() const override { return true; }
	VS_CallIDInfo CheckOfflineStatus(const VS_SimpleStr&call_id) override;
	// VS_EndpointConditions implementation
	bool OnPointDisconnected_Event(const VS_PointParams* prm) override;
	bool OnPointConnected_Event(const VS_PointParams* prm) override;
	// UsersStatusesInterface
	void ListOfOnlineUsers(UsersList &users) override;
	bool UsersStatuses(UsersList &users) override;

	void UnregisterStatus_Method(const VS_Container &cnt, const char *src_server);
	void Subscribe_Method(const VS_Container &cnt);
	void Unsubscribe_Method(const VS_Container &cnt);
	void GetAllUserStatus_Method(const VS_Container &cnt);
	void SubscribeLocated(const std::string &srcServer, const std::string&srcUser, const VS_Container&cnt);
	VS_UserPresence_Status NetworkResolve(const char* uplink, const VS_SimpleStr& call_id, VS_CallIDInfo& ci, VS_TransportRouterServiceBase* caller);
	void RegisterStatus(const char* src_server, const VS_Container&);
	void RegisterDelayedStatus(string_view call_id);
	void SendRegResult(VS_UserLoggedin_Result result, const char *server, const char *temp_id, const char *seq);
	void PushWithCheck(const std::string& call_id, VS_CallIDInfo&& ci, const VS_FullID &source, bool cause);
	void CleanServer(const char* server);
	void Resolve_Method(const VS_Container &cnt);
	void ResolveAll_Method(const VS_Container &cnt);

	void ResolveImpl(const VS_RouterMessage& msg_for_replay, const char * resolve_method,  const std::map<std::string, std::set<std::string>> &location_info);
	void SaveToStorageAndPush(std::string call_id, VS_CallIDInfo&&ci, bool set_server = false, std::string source_server = std::string(), std::string source_user = std::string());
	void OnExtStatusUpdated(OfflineStatusCache::ExtStatusUpdateInfo&& upd);

public:
	VS_BasePresenceService();
	virtual ~VS_BasePresenceService(){}
	bool Init(const char* our_endpoint, const char* our_service, const bool permittedAll = false) override;
	bool Timer(unsigned long tickcount) override;
	bool Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;

	VS_UserPresence_Status Resolve(VS_SimpleStr& call_id, VS_CallIDInfo& ci, bool use_cache, VS_TransportRouterServiceBase* caller);
	std::list<VS_CallIDInfo> ResolveAllSync(std::list<std::string> &&ids, VS_TransportRouterServiceBase* caller);

	//debug
	auto offline_status_cache() -> std::add_lvalue_reference_t<decltype(m_offline_statuses_cache)> { return m_offline_statuses_cache; }

};

extern boost::shared_ptr<VS_BasePresenceService> g_BasePresenceService;

#endif //VS_BASE_PRESENCE_SERVICE_H