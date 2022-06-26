/**
 **************************************************************************************
 *
 * (c) 2005 Visicron Inc.  http://www.visicron.net/
 * \brief Subscription support service helper
 *
 * \file VS_SubscriptionHub.h
 * \note
 **************************************************************************************/

#ifndef VS_SUBSCRIPTION_HUB_H
#define VS_SUBSCRIPTION_HUB_H

#include "../common/transport/Router/VS_TransportRouterServiceBase.h"
#include "../common/transport/Router/VS_TransportRouterServiceHelper.h"
#include "../common/transport/Router/VS_PoolThreadsService.h"
#include "../common/transport/Router/VS_RouterMessage.h"
#include "../common/std/cpplib/VS_MapTpl.h"
#include "../common/std/cpplib/VS_Lock.h"
#include "../common/statuslib/VS_ResolveServerFinder.h"
#include "../common/statuslib/VS_SyncPool.h"
#include "../common/statuslib/VS_CallIDInfo.h"
#include "Common.h"
#include "ServerServices/VS_ConfRestrictInterface.h"

#include "time.h"

#include "std-generic/compat/map.h"
#include <chrono>



typedef VST_Map< VS_FullID,int,TDefPredicate<VS_FullID>,
				 TDefFactory<VS_FullID>, VS_RBTree::VoidFactory,
				 TDefDestructor<VS_FullID>,VS_RBTree::VoidDestructor > VS_IDNullMap;

typedef VST_Map< VS_FullID,long,TDefPredicate<VS_FullID>,
				 TDefFactory<VS_FullID>, TDefFactory<long>,
				 TDefDestructor<VS_FullID>,VS_RBTree::VoidDestructor > VS_IDLongMap;

typedef VST_Map<VS_FullID, std::chrono::system_clock::time_point> VS_IDTimeMap;


typedef VST_Map< VS_FullID,VS_SimpleStr> VS_SubsList;

struct VS_CallIDSub
{
	VS_IDNullMap m_subs;

	VS_CallIDSub ()	{};
	VS_CallIDSub (const VS_CallIDSub& s)
		:m_subs(s.m_subs)
	{	}
};
struct VS_EndpointSub
{
  typedef VS_StrINullMap CallIdMap;
  CallIdMap       m_callid;

	VS_EndpointSub ()	{};
	VS_EndpointSub (const VS_EndpointSub& s)
		:m_callid(s.m_callid)
	{	}
};

struct VS_ServerSubID //Список callID с этого сервера
{
	unsigned long	m_status; //Стату сервера
	time_t			m_lastSubscribtionTime;
	unsigned long	m_current_sub_interval;
	VS_SimpleStr	m_srvName;
	VS_StrINullMap  m_callIDList;	//Список call id с этого сервера, на которых подписываться

	VS_ServerSubID():m_status(0),m_lastSubscribtionTime(0),m_current_sub_interval(0)
	{}
};

typedef VST_Map<char,VS_CallIDSub,VS_Map::StrIPredicate,
                  VS_Map::StrFactory,TDefFactory<VS_CallIDSub>,
                  VS_Map::StrDestructor,TDefDestructor<VS_CallIDSub> >
									VS_CallIDSubMap;

typedef VST_Map<VS_FullID,VS_EndpointSub>
									VS_IDSubMap;

typedef VST_Map<char,VS_CallIDInfo,VS_Map::StrIPredicate,
                VS_Map::StrFactory,TDefFactory<VS_CallIDInfo>,
                VS_Map::StrDestructor,TDefDestructor<VS_CallIDInfo> >
								VS_StatusMap;

typedef VST_StrIMap<VS_StrI_IntMap>	VS_CallIdDataMap;

typedef VST_Map<char,VS_ServerSubID, VS_Map::StrIPredicate,
				VS_Map::StrFactory, TDefFactory<VS_ServerSubID>,
				VS_Map::StrDestructor>
							VS_ServersListForSub; //Список серверов

class VS_SubscriptionHub : public virtual VS_TransportRouterServiceHelper
{
	static	const VS_FullID	empty_id;
	static const unsigned status_timeout = 120000;
	static const unsigned start_interval_for_srv_sub = 5 * 60; ///5 минут, возможно сделать 15
	static const unsigned max_interval_for_srv_sub = 60 * 60 * 24 * 2;///2 суток
	static const std::chrono::steady_clock::duration resubscribe_timeout_for_unknown_callids;
	using FindStatusT = struct { VS_CallIDInfo info; bool found; };

protected:
	static const unsigned DEFAULT_HOPS = 2;		// value two to fix love triangle of 3 servers: not more than 2 TCS servers to PushStatus

private:
	using VS_ContainerMap = vs::map<std::string, VS_Container>;
	class RemoteSubscribe_Task : public VS_PoolThreadsTask, public VS_TransportRouterServiceHelper
	{
		VS_ContainerMap m_list_for_sub;
	public:
		RemoteSubscribe_Task(VS_SubscriptionHub::VS_ContainerMap &&list_for_sub);
		~RemoteSubscribe_Task();
		void Run() override;
	};
	class CheckDomainsCacheForLifetime_Taks : public VS_PoolThreadsTask
	{
	public:
		void Run() override;
	};

public:
	class Listener
	{
	public:
		static const char	LISTENER_USER[];
		virtual void OnServerChange(const char* call_id, const char* server_id) =0;
	};

	VS_SubscriptionHub():m_isRoamingOn(false)
	{}
	void AddListener(const VS_SimpleStr &key, Listener* listener)
	{
		m_listeners[key] = listener;
	}
	VS_CallIDInfo GetStatus(const char* call_id); /*const*/
	FindStatusT FindStatus(const char *call_id); /*const*/
	bool FindStatuses(UsersStatusesInterface::UsersList &users_list, bool gw_status);

	virtual VS_CallIDInfo CheckOfflineStatus ( const VS_SimpleStr& /*call_id*/)
	{	return VS_CallIDInfo(); };
	virtual bool IsRoamingAllowed(const char *server_name = 0) = 0;
	virtual bool IsUplink() const = 0;

protected:
	UsersStatusesInterface::UsersList ListOfOnlineUsers(); /*const*/
	bool Subscribe(const VS_Container& in_cnt, const VS_FullID&  subscriber, bool reset = false,const std::set<std::string> &exclude_id = std::set<std::string>());
	bool Subscribe(const VS_SimpleStr &call_id, const VS_FullID& subscriber);
	bool Unsubscribe(const VS_Container& in_cnt, const VS_FullID&  subscriber, std::set<std::string> *unsub_idx = nullptr);
	bool UnsubscribeID(const VS_FullID& subscriber);
	bool Unsubscribe(const VS_SimpleStr &call_id, const VS_FullID& subscriber);
	bool IsSubscribed(const char *call_id, const VS_FullID &subsscriber) const;

	VS_UserPresence_Status PushStatusFull(const char* call_id, VS_CallIDInfo& ci, const VS_CallIDInfo::Source &source = VS_CallIDInfo::LOCAL, bool set_server = false, const char* source_server = 0, const char* source_user = 0);
	// push status for call_id and alias;
	VS_UserPresence_Status PushStatus(const char* call_id, VS_CallIDInfo& ci, const VS_CallIDInfo::Source &source = VS_CallIDInfo::LOCAL, bool set_server = false, const char* source_server = 0, const char* source_user = 0, const unsigned hops = DEFAULT_HOPS);
	VS_UserPresence_Status PushStatus(const char* call_id, VS_UserPresence_Status status, const VS_ExtendedStatusStorage &extStatus = VS_ExtendedStatusStorage(),
		const VS_CallIDInfo::Source &source = VS_CallIDInfo::LOCAL, const VS_SimpleStr& server_id = 0, bool set_server = false,
		int user_type = -1, const char* source_server = 0, const char* source_user = 0, const unsigned hops = DEFAULT_HOPS, const char* home_server = 0, const char* displayName = 0)
	{
		VS_CallIDInfo ci(status, extStatus, std::string { server_id.m_str, (size_t)server_id.Length() }, user_type, {}, !!home_server ? home_server : std::string{} , !!displayName ? home_server : std::string{});
		ci.m_ml_cap = VS_MultiLoginCapability::SINGLE_USER;
		return
			PushStatus(call_id,
				ci,
			source,
			set_server,
			source_server,
			source_user,
			hops);
	}
	void CleanUplink(const std::string &server_name);

	bool Processing(std::unique_ptr<VS_RouterMessage> &&recvMess) override;
	bool Timer(unsigned long tickcount) override;

	void ServerConnected_Method(const char *srv_name);
	void SeqPost(const VS_FullID& out_sub, VS_Container& cnt, const VS_FullID &id_for_seq = VS_FullID());
	void SeqPostCopy(const VS_FullID& out_sub, const VS_Container& cnt, const VS_FullID &id_for_seq = VS_FullID())
	{
		VS_Container scnt;
		cnt.CopyTo(scnt);
		SeqPost(out_sub, scnt, id_for_seq);
	}

	void RegisterAliases(const VS_SimpleStr real_id, const VS_StrI_IntMap& aliases);
	void RegisterAliases(const VS_Container &cnt);
	void RegisterAliases(const std::string &call_id, const std::set<std::string> &aliases);
	void UnregisterAliases(const VS_SimpleStr call_id, bool del_from_statusCache = true);
	void UnregisterAliases(const VS_SimpleStr call_id, VS_StrI_IntMap to_unreg, bool del_from_statusCache = true);

	bool IsCurrentSubscriberStatusesExist();
	int	 GetNextSubscriberStatuses(const VS_FullID& subscriber, VS_Container& statuses, bool send_offline, bool send_invalid, int max_count);

	VS_StatusMap		m_statusCache; /// основной массив для статусов
	VS_CallIDSubMap		m_callidSub;/// мап тех, кто подписан на key
	VS_IDSubMap			m_epSub; /// мап всех на кого подписан key
	VS_StrIStrMap		m_aliasesMap;  /// соответствие alias->RealID;
	VS_CallIdDataMap	m_callIdMap;   /// соответсвие call_id->список алиасов

	VS_InputSync m_in_sync;
	VS_SyncPool	m_out_sync;
	std::multimap<std::string, std::string>		m_registry_aliases;			// [key=user_id,value=aliases] only registry values

	// only RS. Delete thease methods later
	void GetServerCallIDs(const char *server, std::set<std::string> &call_id_set);
private:

	void ServerDisconnected_Method(const char *srv_name);
	void SubscribeFromOthersServers_Method(const VS_Container &cnt);
	void ExtPresenceStarted_Method();
	void ResendStatus_Method(const VS_Container &cnt);
	void SetLocatorBS_Method(const VS_Container&cnt);

	void ServerOffline(const char *srv_name);
	void ServerOnline(const char *srv_name);
	void ResendStatus(const char *call_id);

	bool	OnPointConnected_Event(const VS_PointParams* prm) override;
	bool	OnPointDisconnected_Event(const VS_PointParams* prm) override;

	void DoSubscribe(const char* call_id, const VS_FullID& subscriber,
		VS_Container& status_cnt, int& status_count, VS_FullID& status_dest,
		bool reset = false);
	void DoUnsubscribe(const char* call_id, const VS_FullID& subscriber);
	//Roaming
	void SubscribeFromOthersServers(VS_ContainerMap&& sub_cnts);
	void PrepareForRemoteSubscribe(const char *callid, VS_ContainerMap &for_sub, VS_ContainerMap &for_task);

	VS_FullID			m_currentSubscriber;
	VS_SimpleStr		m_currentCI;
	std::map<VS_SimpleStr, Listener*>		m_listeners;

	/**
		rouming
	*/
	VS_ServersListForSub	m_srvListForSub; //server list where user statuses are located
	std::set<std::string>	m_connectedServers;



	bool				m_isRoamingOn;
	boost::shared_ptr <VS_ConfRestrictInterface> m_confRestriction;
	std::map < std::string, std::chrono::steady_clock::time_point> m_unknown_call_id_for_sub; /** key = callId, value = expiration time
																							  someone is subscribed for these call ids but locator is unknow for them. Try resubscribe by timeout;
																							  */
	vs::fast_recursive_mutex m_lock;	// TODO: make separate lock for different purposes (not recursive if possible)
};
// end VS_SubscriptionHub class

#endif // VS_SUBSCRIPTION_HUB_H
