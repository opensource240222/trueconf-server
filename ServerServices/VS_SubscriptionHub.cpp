/**
 **************************************************************************************
 *
 * (c) 2005 Visicron Inc.  http://www.visicron.net/
 * \brief Subscription support service helper
 *
 * \file VS_SubscriptionHub.cpp
 * \note
 **************************************************************************************/
#include "VS_SubscriptionHub.h"
#include "std-generic/cpplib/scope_exit.h"
#include "VS_ReadLicense.h"
#include "std-generic/cpplib/string_view.h"
#include "../common/statuslib/VS_ExternalPresenceInterface.h"
#include "std/cpplib/event.h"
#include "std/cpplib/VS_CallIDUtils.h"
#include "std/cpplib/VS_Utils.h"
#include "std/debuglog/VS_Debug.h"

#include "std-generic/compat/map.h"
#include "math.h"
#include "stdlib.h"
#include <sstream>

const char VS_CallIDInfo::EMPTY_EXT[]=" ";
const char VS_SubscriptionHub::Listener::LISTENER_USER[]="@@local";

const std::chrono::steady_clock::duration VS_SubscriptionHub::resubscribe_timeout_for_unknown_callids = std::chrono::hours(1);


#define DEBUG_CURRENT_MODULE VS_DM_USPRS

#define assert_service_tread( where ) do { \
	VS_TransportRouterServiceBase *x = dynamic_cast<VS_TransportRouterServiceBase *>(this); \
	if (x && !x->IsInProcessingThread() ) \
	{ \
		dprint0( "call in invalid thread %s", where ); \
	} \
} while (0)

VS_SubscriptionHub::RemoteSubscribe_Task::RemoteSubscribe_Task(VS_ContainerMap &&list_for_sub) :m_list_for_sub(std::move(list_for_sub))
{
}
VS_SubscriptionHub::RemoteSubscribe_Task::~RemoteSubscribe_Task()
{}

void VS_SubscriptionHub::RemoteSubscribe_Task::Run()
{
	VS_ResolveServerFinder *resolve_server = VS_ResolveServerFinder::Instance();
	for (const auto &iter : m_list_for_sub)
	{
		std::string server_name;
		if (resolve_server->GetServerForResolve(iter.first.c_str(), server_name, false, true))
		{
			if (!server_name.empty() && (server_name != OurEndpoint()))
			{
				VS_Container cnt(std::move(iter.second));
				cnt.AddValue(SERVERNAME_PARAM, server_name.c_str());
				PostRequest(OurEndpoint(), 0, cnt, 0, PRESENCE_SRV);
			}
		}
	}
}

void VS_SubscriptionHub::CheckDomainsCacheForLifetime_Taks::Run()
{
	VS_ResolveServerFinder	* server_finder = VS_ResolveServerFinder::Instance();
	server_finder->CheckDomainsCacheForLifetime();
}

void VS_SubscriptionHub::DoSubscribe ( const char* call_id,const VS_FullID& subscriber,
										VS_Container& status_cnt, int& status_count, VS_FullID& status_dest,
										bool reset)
{
		assert_service_tread( __FUNCTION__ );

		if(call_id==0 ||*call_id==0 || !subscriber)
		{
			dprint1("invalid subscription %s:%s <- '%s' \n",!subscriber?"(null)":subscriber.m_serverID.m_str,!subscriber?"(null)":subscriber.m_userID.m_str,call_id==0?"(null)":call_id);
			return;
		}
		std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
		VS_CallIDSubMap::Iterator ci=m_callidSub[call_id];
		if(!ci)
			ci=VS_CallIDSub();
		ci->data->m_subs.Assign(subscriber,1);

		VS_IDSubMap::Iterator ei=m_epSub[subscriber];
		if(!ei)
			ei=VS_EndpointSub();

	    VS_EndpointSub::CallIdMap::Iterator epsub=ei->data->m_callid[call_id];
		if(!epsub)
		{
			if(status_dest!=subscriber)
			{
				if(status_count>0)
				{
					dprint3("@Sending %d new statuses to %s:%s\n",status_count, status_dest.m_serverID.m_str, status_dest.m_userID.m_str);
					if(reset)
						m_out_sync.Reset(status_dest);
					SeqPost(status_dest, status_cnt);

					status_cnt.Clear();
					status_cnt.AddValue(METHOD_PARAM, UPDATESTATUS_METHOD);
					if(reset)
						status_cnt.AddValueI32(CAUSE_PARAM, 1);
					long hops = VS_SubscriptionHub::DEFAULT_HOPS - 1;		// minus one for myself
					status_cnt.AddValueI32(HOPS_PARAM, hops);
				};
				status_count=0;
				status_dest=subscriber;
			}
			VS_StatusMap::Iterator st = m_statusCache[call_id];

			auto ss = dstream4;
			ss << "new status of " << call_id;

			if (!st)
				st = CheckOfflineStatus(call_id);

			if (st->data->m_status <= USER_LOGOFF)
			{
				if (!!subscriber.m_userID) //sending not to broker
				{
					auto listener_pt = m_listeners.find(subscriber.m_userID);
					if (listener_pt != end(m_listeners)) {
						dprint3("Pushing empty server to listener in subscribe\n");
						if (listener_pt->second)
							listener_pt->second->OnServerChange(call_id, 0);
					}
					else
					{
						status_count++;
						status_cnt.AddValue(CALLID_PARAM, call_id);
						st->data->ToContainer(status_cnt, false,true);
						ss << " OFFLINE";
					}
				}
				else if (USER_INVALID == st->data->m_status)
				{
					status_count++;
					status_cnt.AddValue(CALLID_PARAM, call_id);
					status_cnt.AddValueI32(USERPRESSTATUS_PARAM, st->data->m_status);
					status_cnt.AddValue(SERVER_PARAM, "");
					ss << " INVALID";
				}
				else
				{
					if (!!st->data->m_extStatusStorage)
					{
						status_count++;
						status_cnt.AddValue(CALLID_PARAM, call_id);
						st->data->ToContainer(status_cnt, false, true);
						ss << " OFFLINE.";
					}
					else
						ss << " NO INFO";
				}
			}
			else
			{
				VS_CallIDInfo* push_info = st->data;
				VS_SimpleStr serverID{ (int)push_info->m_serverID.length(), push_info->m_serverID.c_str() };

				auto listener_pt = m_listeners.find(subscriber.m_userID);
				if (subscriber.m_serverID == OurEndpoint() && listener_pt != end(m_listeners))
				{
					dprint3("Pushing server change to listener in subscribe\n");
					if (listener_pt->second)
						listener_pt->second->OnServerChange(call_id, serverID);
				}
				else if (!!subscriber.m_userID
					|| subscriber.m_serverID != serverID || !!push_info->m_extStatusStorage)
				{
					status_count++;
					status_cnt.AddValue(CALLID_PARAM, call_id);
					push_info->ToContainer(status_cnt, true, true);

					ss << " " << st->data->m_status;
				}
				else
				{
					dprint3("@blocked push back to %s:%s\n", subscriber.m_serverID.m_str, subscriber.m_userID.m_str);
				}
			}
			ss << '\n';

			epsub = 1;
		}
		dprint4("@Subscribing %s:%s to %s\n",subscriber.m_serverID.m_str, subscriber.m_userID.m_str, call_id);
}

bool VS_SubscriptionHub::Subscribe(const VS_Container& in_cnt, const VS_FullID&  subscriber, bool reset, const std::set<std::string> &exclude_id)
{
	assert_service_tread( __FUNCTION__ );

	VS_Container status_cnt;

	status_cnt.AddValue(METHOD_PARAM, UPDATESTATUS_METHOD);
	if(reset)
		status_cnt.AddValueI32(CAUSE_PARAM, 1);
	long hops = VS_SubscriptionHub::DEFAULT_HOPS - 1;		// minus one for myself
	status_cnt.AddValueI32(HOPS_PARAM, hops);

	int status_count=0; VS_FullID status_dest;

	in_cnt.Reset();

	VS_ContainerMap	sub_cnts;
	VS_ContainerMap remote_sub; //domain->список call_id для подписки

	while(in_cnt.Next())
	{
		if(strcmp(in_cnt.GetName(),CALLID_PARAM)==0 &&
			in_cnt.GetStrValueRef() != nullptr &&
			exclude_id.find(in_cnt.GetStrValueRef()) == exclude_id.end())
		{
			DoSubscribe(in_cnt.GetStrValueRef(), subscriber, status_cnt, status_count, status_dest);
			VS_ResolveServerFinder * resolve_finder = VS_ResolveServerFinder::Instance();
			auto ext_presence = resolve_finder->GetExternalPresence(in_cnt.GetStrValueRef());
			if (ext_presence)
			{
				ext_presence->Subscribe(in_cnt.GetStrValueRef());
				continue;
			}

			if(!IsRoamingAllowed())
				continue;
			PrepareForRemoteSubscribe(in_cnt.GetStrValueRef(), sub_cnts, remote_sub);
		}
	}
	if(IsRoamingAllowed())
	{
		SubscribeFromOthersServers(std::move(sub_cnts));
		if(!remote_sub.empty())
		{
			PutTask(new RemoteSubscribe_Task(std::move(remote_sub)), "RemoteSubscribe1", 180); /// 3 минуты
		}
	}
	if(status_count>0)
	{
		dprint3("@Sending %d new statuses to %s:%s\n",status_count, status_dest.m_serverID.m_str, status_dest.m_userID.m_str);
		if(reset)
			m_out_sync.Reset(status_dest);
		SeqPost(status_dest, status_cnt);
	}
	return true;
}

void VS_SubscriptionHub::PrepareForRemoteSubscribe(const char *callid, VS_ContainerMap  &for_sub, VS_ContainerMap  &for_task)
{
	auto * resolve_finder = VS_ResolveServerFinder::Instance();
	if (!resolve_finder || !callid)
		return;
	std::string user_srvName;
	if (resolve_finder->GetServerForResolve(callid, user_srvName, true))
	{
		if (!user_srvName.empty() && (user_srvName != OurEndpoint()))
		{
			VS_ServersListForSub::Iterator	srv_sub_i = m_srvListForSub[user_srvName.c_str()];
			if (!srv_sub_i)
				srv_sub_i = VS_ServerSubID();
			if (!!srv_sub_i->data->m_callIDList.Find(callid)) // we are subscribed already
			{
				dprint4(" we are subscribed already. call_id = %s; server = %s", callid, user_srvName.c_str());
				return;
			}
			auto cnt_iter = for_sub.find(user_srvName);
			if (cnt_iter == for_sub.end())
			{
				cnt_iter = for_sub.emplace(user_srvName, VS_Container()).first;
				cnt_iter->second.AddValue(METHOD_PARAM, SUBSCRIBE_METHOD);
				cnt_iter->second.AddValueI32(MULTI_LOGIN_CAPABILITY_PARAM, VS_MultiLoginCapability::SINGLE_USER);
			}
			cnt_iter->second.AddValue(CALLID_PARAM, callid);
			srv_sub_i->data->m_srvName = user_srvName.c_str();
			srv_sub_i->data->m_callIDList.Assign(callid, 1);
		}
	}
	else
	{
		string_view call_id_view = callid;
		auto pos = call_id_view.find("@");
		if (pos == string_view::npos)
			return;
		try
		{
			auto domain = call_id_view.substr(pos + 1);
			if (domain.size() > 0)
			{
				auto for_task_i = for_task.find(std::string(domain));
				if (for_task.end() == for_task_i)
				{
					for_task_i = for_task.emplace(domain, VS_Container()).first;
					for_task_i->second.AddValue(METHOD_PARAM, "SubsFromOtherServers");
				}
				for_task_i->second.AddValue(CALLID_PARAM, call_id_view);
			}
		}
		catch (std::out_of_range&)
		{
		}
	}
}
bool VS_SubscriptionHub::Subscribe( const VS_SimpleStr &_call_id, const VS_FullID& subscriber)
{
	assert_service_tread( __FUNCTION__ );
	VS_Container cnt;
	cnt.AddValue(CALLID_PARAM, _call_id);
	return Subscribe(cnt, subscriber);
}


void VS_SubscriptionHub::DoUnsubscribe( const char* call_id,const VS_FullID& subscriber){
	assert_service_tread( __FUNCTION__ );
	dprint4("@Unsubscribing %s from %s:%s\n",call_id, subscriber.m_serverID.m_str, subscriber.m_userID.m_str);
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	VS_IDSubMap::Iterator ei=m_epSub[subscriber];
	if(!!ei)
		if(ei->data->m_callid.Erase(call_id)>0)
			if(ei->data->m_callid.Size()==0)
				m_epSub.Erase(ei);

	VS_CallIDSubMap::Iterator ci=m_callidSub[call_id];
	if(!!ci)
		if(ci->data->m_subs.Erase(subscriber)>0)
			if(ci->data->m_subs.Size()==0)
			{
				m_unknown_call_id_for_sub.erase(ci->key);
				m_callidSub.Erase(ci);
				auto resolver = VS_ResolveServerFinder::Instance();
				std::string uplink;
				resolver->GetServerByUser(call_id, uplink);
				if(uplink!=OurEndpoint())
				{
					VS_StatusMap::Iterator	sti = m_statusCache[call_id];
					if((!!sti)&&(sti->data->m_serverID != OurEndpoint() ) )
						m_statusCache.Erase(call_id);
				}
			}
}

bool VS_SubscriptionHub::Unsubscribe( const VS_Container& in_cnt, const VS_FullID& subscriber, std::set<std::string> *unsub_idx)
{
	assert_service_tread( __FUNCTION__ );
	in_cnt.Reset();
	const char* call_id=0;
	VS_ContainerMap	unsub_cnts;

	while(in_cnt.Next())
	{
		if(strcmp(in_cnt.GetName(),CALLID_PARAM)==0 && in_cnt.GetStrValueRef()!=nullptr)
		{
			// skip if not subscribed;
			if(!IsSubscribed(in_cnt.GetStrValueRef(),subscriber))
				continue;
			if(unsub_idx!= nullptr)
				unsub_idx->insert(in_cnt.GetStrValueRef());
			DoUnsubscribe(in_cnt.GetStrValueRef(), subscriber);
			VS_ResolveServerFinder *resolve_server = VS_ResolveServerFinder::Instance();
			auto ext_presence = resolve_server->GetExternalPresence(in_cnt.GetStrValueRef());
			if (ext_presence && !!m_callidSub.Find(in_cnt.GetStrValueRef()))
			{
				ext_presence->Unsubscribe(in_cnt.GetStrValueRef());
				continue;
			}
			std::string user_srvName;
			if (resolve_server->GetServerForResolve(in_cnt.GetStrValueRef(), user_srvName, true))
			{
				if(!user_srvName.empty() && (user_srvName!=OurEndpoint()))
				{
					/**
						skip unsubscribe from remote server if somebody is subscribed to the call_id
					*/
					if (!!m_callidSub.Find(in_cnt.GetStrValueRef()))
						continue;
					auto cnt_iter = unsub_cnts.find(user_srvName);
					if (cnt_iter == unsub_cnts.end())
					{
						cnt_iter = unsub_cnts.emplace(user_srvName, VS_Container()).first;
						cnt_iter->second.AddValue(METHOD_PARAM, UNSUBSCRIBE_METHOD);
					}
					cnt_iter->second.AddValue(CALLID_PARAM,in_cnt.GetStrValueRef());
					VS_ServersListForSub::Iterator	srv_sub_i = m_srvListForSub[user_srvName.c_str()];
					if(!!srv_sub_i)
					{
						srv_sub_i->data->m_callIDList.Erase(in_cnt.GetStrValueRef());
						if(srv_sub_i->data->m_callIDList.Empty())
							m_srvListForSub.Erase(srv_sub_i);
					}
					else
					{
						dprint1("Unsubscribe user %s but users server %s is not in list\n",in_cnt.GetStrValueRef(),user_srvName.c_str());
					}
				}
			}
		}
	};
	for(auto &iter :unsub_cnts)
	{
		auto & srv_name = iter.first;
		VS_FullID	srv_full_id(srv_name.c_str(),0);
		VS_FullID	seq_full_id(srv_full_id);
		seq_full_id.m_serverID +="sub";
		dprint4("@Unsubscription from server %s\n",srv_name.c_str());
		SeqPost(srv_full_id,iter.second,seq_full_id);
	}
	return true;
}


bool VS_SubscriptionHub::Unsubscribe( const VS_SimpleStr &_call_id, const VS_FullID& subscriber )
{
	assert_service_tread( __FUNCTION__ );
	VS_Container cnt;
	cnt.AddValue(CALLID_PARAM, _call_id);
	return Unsubscribe(cnt, subscriber);
}

bool VS_SubscriptionHub::IsSubscribed(const char * call_id, const VS_FullID & subsscriber) const
{
	const auto sub = m_epSub[subsscriber];
	if (!sub)
		return false;
	const auto sub_id = sub->data->m_callid[call_id];
	if (!sub_id)
		return false;
	return true;
}

bool VS_SubscriptionHub::UnsubscribeID ( const VS_FullID& subscriber )
{
	int up_unsubs=0;
	VS_ResolveServerFinder *resolve_server = VS_ResolveServerFinder::Instance();
	std::vector<std::string> to_unsub_from_ext_pres;		// do it without AutoLock

	m_lock.lock();
	VS_SCOPE_EXIT{
		m_lock.unlock();
		for (const auto& call_id : to_unsub_from_ext_pres)
		{
			auto ext_presence = resolve_server->GetExternalPresence(call_id.c_str());
			if (ext_presence)
				ext_presence->Unsubscribe(call_id.c_str());
		}
	};

	VS_IDSubMap::Iterator ei=m_epSub[subscriber];
	if(!ei)
		return false;

	VS_ContainerMap	unsub_cnts;
	VS_EndpointSub::CallIdMap& map=ei->data->m_callid;
	for(VS_EndpointSub::CallIdMap::Iterator ci=map.Begin();!!ci;ci++)
	{
		VS_CallIDSubMap::Iterator csub=m_callidSub[*ci->key];

		if(!csub)
			continue;
		csub->data->m_subs.Erase(subscriber);
		dprint3("@Unsubscribe %s:%s from %s\n", subscriber.m_serverID.m_str, subscriber.m_userID.m_str, csub->key);
		if(csub->data->m_subs.Size()==0)
		{
			const char* call_id=csub->key;

			std::string user_srvName;
			if (call_id && *call_id && resolve_server->GetExternalPresence(call_id))
				to_unsub_from_ext_pres.emplace_back(call_id);
			else if(resolve_server->GetServerForResolve(call_id,user_srvName,true))
			{
				if(!user_srvName.empty() && (user_srvName!=OurEndpoint()))
				{
					//если не наш юзер, то отписаться
					auto cnt_iter = unsub_cnts.find(user_srvName);
					if (cnt_iter == unsub_cnts.end())
					{
						cnt_iter = unsub_cnts.emplace(user_srvName, VS_Container()).first;
						cnt_iter->second.AddValue(METHOD_PARAM, UNSUBSCRIBE_METHOD);
					}
					cnt_iter->second.AddValue(CALLID_PARAM,call_id);
					VS_ServersListForSub::Iterator	srv_sub_i = m_srvListForSub[user_srvName.c_str()];
					if(!!srv_sub_i)
					{
						srv_sub_i->data->m_callIDList.Erase(call_id);
						if(srv_sub_i->data->m_callIDList.Empty())
							m_srvListForSub.Erase(srv_sub_i);
					}
				}
			}
			auto resolver = VS_ResolveServerFinder::Instance();
			std::string uplink;
			resolver->GetServerByUser(call_id, uplink);
			if(uplink!=OurEndpoint())
			{
				///Если это RS, то удалять статусы нельзя
				VS_StatusMap::Iterator	sti = m_statusCache[ci->key];
				if( (!!sti) && (sti->data->m_serverID != OurEndpoint()) )
					m_statusCache.Erase(ci->key);
			}
			up_unsubs++;
			m_unknown_call_id_for_sub.erase(csub->key);
			m_callidSub.Erase(csub);
		}
	}
	m_epSub.Erase(ei);

	for(auto &iter:unsub_cnts)
	{
		auto &srv_name = iter.first;
		VS_FullID	srv_full_id(srv_name.c_str(),0);
		VS_FullID	seq_full_id(srv_full_id);
		seq_full_id.m_serverID +="sub";

		dprint4("@Usubscription from server %s\n",srv_name.c_str());
		SeqPost(srv_full_id,iter.second,seq_full_id);
	}
	dprint3( "@Unsubscribing %s:%s\n", subscriber.m_serverID.m_str, subscriber.m_userID.m_str );
	return true;
}

static const char src_names[]="-LPU!@#$%^";

VS_UserPresence_Status VS_SubscriptionHub::PushStatus(const char* call_id, VS_CallIDInfo& ci,
	const VS_CallIDInfo::Source &source, bool set_server, const char* source_server, const char* source_user, const unsigned hops)
{
	assert_service_tread( __FUNCTION__ );

	auto ss = dstream3;
	ss << "@PushStatus " << call_id << " : " << (int)ci.m_status << " source:" << src_names[source] << " source_server:"<<source_server << " ml_cap:" << (
		(ci.m_ml_cap == VS_MultiLoginCapability::UNKNOWN)? "unknown":
		(ci.m_ml_cap == VS_MultiLoginCapability::SINGLE_USER)? "single":
		(ci.m_ml_cap == VS_MultiLoginCapability::MULTI_USER)? "multi": "xz");

	if(!call_id || !*call_id) return USER_STATUS_UNDEF;
	if(set_server)
	{
		if(!ci.m_serverID.empty())
		{
			ss << " REG_AT(" << ci.m_serverID << ") ";
			if (ci.m_status<USER_AVAIL)
			{
				ci.m_status=USER_AVAIL;
				ss << " RAISESTATUS ";
			}
		}
		else
		{
			ss << " UNREG ";
			if (ci.m_status>USER_LOGOFF)
			{
				ci.m_status=USER_LOGOFF;
				ss << " LOWERSTATUS ";
			}
		}
	}
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	const VS_CallIDInfo* push_info=&ci;
	VS_StatusMap::Iterator sti=m_statusCache[call_id];
	const auto our_endpoint = OurEndpoint();
	if(!sti)
	{
		if (source > ci.LOCAL && (our_endpoint == nullptr || *our_endpoint == 0 ? ci.m_serverID.empty() : ci.m_serverID == our_endpoint))
		{
			ss << " Uplink imposes not my user. Skip;\n";
			return USER_STATUS_UNDEF;
		}
		sti=ci;
		push_info=sti->data;
		ss << " NEW ";
	}
	else
	{
		VS_CallIDInfo* curr=sti->data;
		push_info=curr;
		bool update_only_ext_statuses = false;
		if ((our_endpoint == nullptr || *our_endpoint == 0 ? curr->m_serverID.empty() : curr->m_serverID == our_endpoint) && source > ci.LOCAL || !curr->CanBeChangedBy(set_server, ci)) // block only setting identical status
		{
			if (curr->m_extStatusStorage == ci.m_extStatusStorage)
			{
				ss << " BLOCK(push identical status) ";
				ss << " IGNORED\n";
				return USER_STATUS_UNDEF;
			}
			else
			{
				update_only_ext_statuses = true; // update only extStatus;
				auto ds = dstream4;
				ds << "Push status from uplink for local user. Extended status updated, update only ext status.\n";
			}
		}
		UpdateCallIDInfo(ci, *curr, set_server, update_only_ext_statuses);
	}
	// set uplink for call id;
	if (!push_info->m_homeServer.empty())
	{
		auto resolver = VS_ResolveServerFinder::Instance();
		if (resolver)
			resolver->SetServerForUser(call_id,	push_info->m_homeServer, true);
	}
	ss << " PASSED\n";

	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, UPDATESTATUS_METHOD);
	cnt.AddValue(CALLID_PARAM, call_id);
	cnt.AddValueI32(HOPS_PARAM, hops);
	push_info->ToContainer(cnt,set_server,false);

	vs::map<std::string /*vs_user_id*/, std::string> notify_listeners;
	VS_SCOPE_EXIT{
		for (const auto& i : notify_listeners)
		{
			auto it = m_listeners.find(i.first.c_str());
			if (it != m_listeners.end())
			{
				dstream3 << "Pushing server change to listener " << i.first << " (" << call_id << " at " << i.second << ")";
				if (it->second)
					it->second->OnServerChange(call_id, push_info->m_serverID.c_str());
			}
		}
	};

	VS_CallIDSubMap::Iterator isub=m_callidSub[call_id];
	if(!!isub)
	{
		const VS_CallIDSub* sub=isub->data;
		for(VS_IDNullMap::ConstIterator ii=sub->m_subs.Begin();!!ii;++ii)
		{
			const VS_FullID* key=ii->key;
			if((key->m_serverID==source_server && key->m_userID==source_user &&
				!key->m_userID&&!IsUplink()) || /* If user migrates from server1 to server2 it is possible old ext status will be pushed from server1 to server2 but actual ext status from server2 to server2 will be blocked after that.
												It is reason do not block status pushback from uplink server.*/
				(key->m_serverID == OurEndpoint() && !key->m_userID))
			{
				dprint4("Blocked push back to %s:%s\n",key->m_serverID.m_str,key->m_userID.m_str);
			}
			else
			{
				auto listener_pt = m_listeners.find(key->m_userID);
				if(key->m_serverID==OurEndpoint() && listener_pt != end(m_listeners))
				{
					if(set_server)
					{
						notify_listeners.emplace(key->m_userID, push_info->m_serverID);
					}
				}
				else
				{
					if(key->m_serverID != OurEndpoint() && m_confRestriction && m_confRestriction->IsVCS() && !IsRoamingAllowed())
						continue;
					dprint4("Pushing to %s:%s\n",key->m_serverID.m_str,key->m_userID.m_str);
					SeqPostCopy(*key, cnt);
				}
			}
		}
	}
	///propagate local source UP
	if (source <= ci.LOCAL)
	{
		// send to uplink
		auto alias_iter = m_aliasesMap[call_id];
		VS_SimpleStr real_call_id;
		if (!!alias_iter && (alias_iter->data && *alias_iter->data))
			real_call_id = alias_iter->data;
		else
			real_call_id = call_id;

		if (real_call_id == call_id)
		{
			auto resolver = VS_ResolveServerFinder::Instance();
			assert(resolver);
			std::string uplink_server;
			if (resolver->GetServerByUser(call_id, uplink_server) && uplink_server != OurEndpoint())
				SeqPost(VS_FullID(uplink_server.c_str(), 0), cnt);
		}
	} //propagate local soucre
	return push_info->m_status;
}
VS_UserPresence_Status VS_SubscriptionHub::PushStatusFull(const char* call_id, VS_CallIDInfo& ci, const VS_CallIDInfo::Source &source, bool set_server, const char* source_server, const char* source_user)
{
	auto alias_i = m_callIdMap[call_id];
	if (!!alias_i)
	{
		for (VS_StrI_IntMap::Iterator ii = alias_i->data->Begin(); !!ii; ii++)
		{
			VS_SimpleStr	alias = ii->key;
			PushStatus(alias, ci, source, set_server, source_server, source_user, VS_SubscriptionHub::DEFAULT_HOPS);
		}
	}
	return PushStatus(call_id, ci, source, set_server, source_server, source_user, VS_SubscriptionHub::DEFAULT_HOPS);
}

VS_CallIDInfo VS_SubscriptionHub::GetStatus(const char* call_id )
{
	VS_CallIDInfo result;
	vs::event done { true };
	if (CallInProcessingThread([&]() {
		VS_SCOPE_EXIT { done.set(); };
		auto cache = m_statusCache[call_id];
		result = !!cache ? *cache->data : CheckOfflineStatus(call_id);
	}))
	{
		done.wait();
	}
	return result;
}

VS_SubscriptionHub::FindStatusT VS_SubscriptionHub::FindStatus(const char *call_id)
{
	FindStatusT result { VS_CallIDInfo(), false };
	vs::event done { true };
	dstream4 << "VS_SubscriptionHub::FindStatus: call_id = " << call_id;
	auto start_time = std::chrono::steady_clock::now();
	if (CallInProcessingThread([&]() {
		VS_SCOPE_EXIT { done.set(); };
		auto cache = m_statusCache[call_id];
		if (!!cache)
			result = { *cache->data, true };
		else
			result = { VS_CallIDInfo(), false };
	}))
	{
		done.wait();
		dstream4 << "VS_SubscriptionHub::FindStatus: completed, it took " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count() << " ms\n";
	}
	else
		dstream1 << "VS_SubscriptionHub::FindStatus: CallInProcessingThread return false;\n";

	return result;
}

bool VS_SubscriptionHub::FindStatuses(UsersStatusesInterface::UsersList &users_list, bool gw_status)
{
	vs::event done { true };
	auto start_time = std::chrono::steady_clock::now();
	bool result = CallInProcessingThread([&]() {
		auto rtp_user_status = gw_status ? USER_AVAIL : USER_LOGOFF;
		for (auto &ii : users_list) {
			auto it_cache = m_statusCache.Find(ii.first.c_str());
			if (it_cache != m_statusCache.End())
				ii.second = it_cache->data->m_status;
			else if (VS_IsRTPCallID(ii.first))
				ii.second = rtp_user_status;
			else
				ii.second = USER_LOGOFF;
		}
		done.set();
	});

	if (result) {
		done.wait();
		dstream4 << "VS_SubscriptionHub::FindStatuses: " << users_list.size()
			<< " users completed, it took " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count() << " ms\n";
	}
	else
		dstream1 << "VS_SubscriptionHub::FindStatuses: CallInProcessingThread return false;\n";

	return result;
}

UsersStatusesInterface::UsersList VS_SubscriptionHub::ListOfOnlineUsers()
{
	UsersStatusesInterface::UsersList result;
	vs::event done { true };
	if (CallInProcessingThread([&]() {
		VS_SCOPE_EXIT { done.set(); };
		for (auto itUser = m_callIdMap.Begin(); !!itUser; ++itUser)
		{
			auto itStatus = m_statusCache[itUser->key];
			auto status = !!itStatus ? itStatus->data->m_status : VS_UserPresence_Status::USER_LOGOFF;
			if (status != VS_UserPresence_Status::USER_LOGOFF && status != VS_UserPresence_Status::USER_STATUS_UNDEF)
				result.emplace_back(itUser->key, status);
		}
	}))
	{
		done.wait();
	}

	return result;
}

int VS_SubscriptionHub::GetNextSubscriberStatuses( const VS_FullID& subscriber, VS_Container& statuses,
											   bool send_offline, bool send_invalid, int max_count )
{
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	int res(0);
	if(subscriber != m_currentSubscriber)
	{
		m_currentSubscriber = subscriber;
		m_currentCI = 0;
	}
	VS_IDSubMap::Iterator isub=m_epSub[subscriber];
	if(!isub)
		return 0;
	VS_EndpointSub* sub=isub->data;
	VS_EndpointSub::CallIdMap::ConstIterator ci = !m_currentCI ? sub->m_callid.Begin() : sub->m_callid[m_currentCI];
	for (; !!ci && res < max_count; ci++)
	{
		VS_StatusMap::Iterator i = m_statusCache[ci->key];
		VS_UserPresence_Status status;
		VS_SimpleStr serverID;
		VS_CallIDInfo offline_info;
		VS_CallIDInfo* ci_ptr(0);
		if (!!i)
		{
			status = i->data->m_status;
			serverID = VS_SimpleStr{ (int)i->data->m_serverID.length(), i->data->m_serverID.c_str() };
			ci_ptr = i->data;
		}
		else
		{
			offline_info = CheckOfflineStatus(ci->key);
			status = offline_info.m_status;
			ci_ptr = &offline_info;
		}
		if (!!subscriber.m_userID ||
			(status > USER_LOGOFF && (serverID == 0 || serverID != subscriber.m_serverID)))

			if ((send_offline && status == USER_LOGOFF) ||
				(send_invalid && status == USER_INVALID) ||
				status > USER_LOGOFF)
			{
				statuses.AddValue(CALLID_PARAM, ci->key);
				if (!!ci_ptr)
					ci_ptr->ToContainer(statuses, true, true);
				res++;
			}

	}
	if(!ci || !res)
	{
		m_currentSubscriber.m_serverID = 0;
		m_currentSubscriber.m_userID = 0;
		m_currentCI  = 0;
	}
	else
		m_currentCI = ci->key;
	return res;
}
bool VS_SubscriptionHub::IsCurrentSubscriberStatusesExist()
{
	assert_service_tread( __FUNCTION__ );

	if(!m_currentCI)
		return false;
	else
		return true;
}


void VS_SubscriptionHub::CleanUplink(const std::string&uplink_name)
{
	assert_service_tread(__FUNCTION__);
	dprint3("Clean uplink name = %s;\n",uplink_name.c_str());
	std::map<VS_FullID, VS_Container> send_conts;
	auto resolve_finder = VS_ResolveServerFinder::Instance();
	auto ids = resolve_finder->GetUsersForUplink(uplink_name);
	for (const auto&id : ids)
	{
		auto st_i = m_statusCache[id.c_str()];
		if (!!st_i && st_i->data->m_serverID != OurEndpoint())
		{
			const char *key = st_i->key;
			dprint3("Cleaned status of %s\n", key);
			auto sub_i = m_callidSub[key];
			if (!!sub_i)
			{
				for (VS_IDNullMap::ConstIterator ii = sub_i->data->m_subs.Begin(); !!ii; ii++)
				{
					if (ii->key->m_userID != Listener::LISTENER_USER)
					{
						auto cnt_i = send_conts.find(*ii->key);
						if (cnt_i == send_conts.end())
						{
							cnt_i = send_conts.emplace(*ii->key, VS_Container()).first;
							cnt_i->second.AddValue(METHOD_PARAM, UPDATESTATUS_METHOD);
							long hops = VS_SubscriptionHub::DEFAULT_HOPS - 1;		// minus one for myself
							cnt_i->second.AddValueI32(HOPS_PARAM, hops);
						}
						cnt_i->second.AddValue(CALLID_PARAM, st_i->key);
						cnt_i->second.AddValueI32(USERPRESSTATUS_PARAM, CheckOfflineStatus(st_i->key).m_status);
						st_i->data->m_extStatusStorage.ToContainer(cnt_i->second,false);
						cnt_i->second.AddValue(SERVER_PARAM, "");
					}
				}
			}
			m_statusCache.Erase(st_i);
		}
	}
	for (auto &i : send_conts)
	{
		dprint3("@Pushing offline statuses to %s:%s\n", i.first.m_serverID.m_str, i.first.m_userID.m_str);
		SeqPost(i.first, i.second);
	}
}
void VS_SubscriptionHub::SeqPost(const VS_FullID& out_subs, VS_Container& cnt, const VS_FullID &id_for_seq)
{
	int32_t seq_id = m_out_sync.Inc(!id_for_seq ? out_subs : id_for_seq);
	if(seq_id<0)
	{
		cnt.AddValueI32(CAUSE_PARAM, 1);
		seq_id=m_out_sync.Reset(!id_for_seq?out_subs:id_for_seq);
	}
	cnt.AddValueI32(SEQUENCE_ID_PARAM, seq_id);

	void* body;
	size_t bodySize;
	cnt.SerializeAlloc(body, bodySize);
	dprint3("@SeqPost to %s:%s seq_id=%08x bytes=%zu\n", out_subs.m_serverID.m_str, out_subs.m_userID.m_str, seq_id, bodySize);

	VS_RouterMessage *msg = new VS_RouterMessage(OurService(), NULL, OurService(), out_subs.m_userID, 0,out_subs.m_serverID, OurEndpoint(), status_timeout, body, bodySize);
	if(!PostMes(msg))
		delete msg;

	free(body);
}
void VS_SubscriptionHub::GetServerCallIDs(const char *server, std::set<std::string> &call_id_set)
{
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	VS_StatusMap::ConstIterator i = m_statusCache.Begin();
	while (!!i)
	{
		if (i->data->m_serverID == server)
			call_id_set.insert(i->key);
		++i;
	}
	dprint3("@GetServer Statuses %s found %zu\n", server, call_id_set.size());
}


void VS_SubscriptionHub::RegisterAliases(const VS_Container &cnt)
{
	assert_service_tread( __FUNCTION__ );

	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	if(!cnt.IsValid())
		return;

	VS_SimpleStr	real_id = cnt.GetStrValueRef(REALID_PARAM);
	VS_StrI_IntMap a;
	cnt.Reset();
	while(cnt.Next())
	{
		if(strcasecmp(cnt.GetName(),ALIAS_PARAM) == 0)
			a.Insert(cnt.GetStrValueRef(),1);
	}
	RegisterAliases(real_id, a);
}

void VS_SubscriptionHub::RegisterAliases(const VS_SimpleStr real_id, const VS_StrI_IntMap& aliases)
{
	assert_service_tread( __FUNCTION__ );
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	VS_CallIdDataMap::Iterator	ii = m_callIdMap[real_id];
	m_aliasesMap.Erase(real_id);
	if(!!ii)
	{
		VS_StrI_IntMap can_delete_alias;
		//// Удаляем все предыдущие алиасы для real_id
		for(VS_StrI_IntMap::ConstIterator i=ii->data->Begin();!!i;++i)
		{
			const char *alias = i->key;
			if (*(i->data) == 0)		// UserAliases=0 (not SystemAliases=1)
			{
				m_aliasesMap.Erase(alias);
				can_delete_alias.Insert(alias,*i->data);
			}
		}
		for(VS_StrI_IntMap::Iterator it=can_delete_alias.Begin(); it!=can_delete_alias.End(); ++it)
			ii->data->Erase(it->key);
	}
	else
		ii = VS_StrI_IntMap();
	m_aliasesMap[real_id] = real_id;
	for (VS_StrI_IntMap::ConstIterator i = aliases.Begin(); !!i; ++i)
	{
		const char *alias = i->key;
		m_aliasesMap[alias] = real_id;
		ii->data->Insert(alias,*i->data);
	}
}

void VS_SubscriptionHub::RegisterAliases(const std::string &call_id, const std::set<std::string> &aliases)
{
	assert_service_tread(__FUNCTION__);
	VS_StrI_IntMap a;
	for (const auto &i : aliases)
		a.Insert(i.c_str(), 1);
	RegisterAliases(call_id.c_str(), a);
}

void VS_SubscriptionHub::UnregisterAliases(const VS_SimpleStr call_id, bool del_from_statusCache)
{
	assert_service_tread( __FUNCTION__ );
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	m_aliasesMap.Erase(call_id);
	VS_CallIdDataMap::Iterator	ii = m_callIdMap[call_id];
	if(!!ii)
	{
		VS_StrI_IntMap can_delete_alias;
		//// Удаляем все предыдущие алиасы для real_id
		for(VS_StrI_IntMap::ConstIterator i=ii->data->Begin();!!i;++i)
		{
			const char *alias = i->key;
			if (*i->data == 0)		// UserAliases=0 (not SystemAliases=1)
			{
				m_aliasesMap.Erase(alias);
				////Удалить из статуса
				if(del_from_statusCache)
					m_statusCache.Erase(alias);
				can_delete_alias.Insert(alias,*i->data);
			}
		}
		for(VS_StrI_IntMap::Iterator it=can_delete_alias.Begin(); it!=can_delete_alias.End(); ++it)
			ii->data->Erase(it->key);
	}
	if (!!ii&&ii->data->Empty())
		m_callIdMap.Erase(ii);
	if(del_from_statusCache)
		m_statusCache.Erase(call_id);
}
void VS_SubscriptionHub::UnregisterAliases(const VS_SimpleStr call_id, VS_StrI_IntMap to_unreg, bool del_from_statusCache)
{
	assert_service_tread( __FUNCTION__ );
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	VS_CallIdDataMap::Iterator	ii = m_callIdMap[call_id];
	if(!!ii)
	{
		for (VS_StrI_IntMap::ConstIterator i = to_unreg.Begin(); i != to_unreg.End(); ++i)
		{
			const char *alias = i->key;
			m_aliasesMap.Erase(alias);
			////Удалить из статуса
			if(del_from_statusCache)
				m_statusCache.Erase(alias);
			ii->data->Erase(alias);
		}
		// The data in 'm_callIdMap' is going to be erased in the call to UnregisterAliases() in VS_PresenceService::OnUserLogoff_Event() or VS_BasePresenceService::UnregisterStatus_Method().
		// We need to have some data in m_callIdMap for an online user for online users listing functionality to work.
		//if (ii->data->Empty())
			//m_callIdMap.Erase(ii);
	}
}
bool VS_SubscriptionHub::OnPointConnected_Event(const VS_PointParams *prm)
{
	if (!prm) return true;

	if (prm->reazon <= 0) {
		dprint1("Error {%d} while Connect to (%s)\n", prm->reazon, prm->uid);
		return true;
	}

	if(prm->type==VS_PointParams::PT_SERVER)
	{
		VS_Container	cnt;
		cnt.AddValue(METHOD_PARAM, POINTCONNECTED_METHOD);
		cnt.AddValue(NAME_PARAM, prm->uid);
		PostRequest(OurEndpoint(), 0, cnt);
	}
	return true;
}
bool VS_SubscriptionHub::OnPointDisconnected_Event(const VS_PointParams *prm)
{
	if(prm&&prm->type==VS_PointParams::PT_SERVER)
	{
		VS_Container	cnt;
		cnt.AddValue(METHOD_PARAM, POINTDISCONNECTED_METHOD);
		cnt.AddValue(NAME_PARAM, prm->uid);
		PostRequest(OurEndpoint(), 0, cnt);
	}
	return true;
}


bool VS_SubscriptionHub::Processing(std::unique_ptr<VS_RouterMessage> &&recvMess)
{
	assert_service_tread( __FUNCTION__ );
	if (!recvMess)
		return true;
	switch (recvMess->Type())
	{
	case transport::MessageType::Invalid:	// Skip
		break;
	case transport::MessageType::Request:
		{
			VS_Container cnt;
			if (cnt.Deserialize(recvMess->Body(), recvMess->BodySize()))
			{
				const char* m = nullptr;
				if ((m= cnt.GetStrValueRef(METHOD_PARAM)) != 0)
				{
					const string_view method = m;
					if(method == POINTCONNECTED_METHOD)
					{
						ServerConnected_Method(cnt.GetStrValueRef(NAME_PARAM));
					}
					else if(method == POINTDISCONNECTED_METHOD)
					{
						ServerDisconnected_Method(cnt.GetStrValueRef(NAME_PARAM));
					}
					else if(method == "SubsFromOtherServers")
					{
						SubscribeFromOthersServers_Method(cnt);
					}
					else if(method == EXTERNALPRESENCESTARTED_METHOD)
					{
						ExtPresenceStarted_Method();
					}
					else if(method == RESENDSTATUS_METHOD)
					{
						ResendStatus_Method(cnt);
					}
					else if (method == SETLOCATORBS_METHOD)
					{
						SetLocatorBS_Method(cnt);
					}

				}
			}
		}
		break;
	case transport::MessageType::Reply:
		break;
	case transport::MessageType::Notify:
		break;
	}
	return true;
}

void VS_SubscriptionHub::SetLocatorBS_Method(const VS_Container &cnt)
{
	assert_service_tread(__FUNCTION__);
	/**
		check, that we are subscribed
		resubscribe from server for cnt
		handle cause==1 (home server was not found)
	*/
	cnt.Reset();
	uint8_t counter(0);
	static const string_view callid_param = CALLID_PARAM;
	static const string_view cause_param = CAUSE_PARAM;
	static const string_view locatorbs_param = LOCATORBS_PARAM;

	const char* call_id = nullptr;
	const char* locator = nullptr;

	VS_ContainerMap	sub_cnts;
	while (cnt.Next())
	{
		const string_view name = cnt.GetName();
		if (callid_param == name)
			call_id = cnt.GetStrValueRef();
		else if (locatorbs_param == name)
			locator = cnt.GetStrValueRef();
		else if (cause_param == name)
		{
			if (!!m_callidSub.Find(call_id))
				m_unknown_call_id_for_sub.emplace(call_id, std::chrono::steady_clock::now() + resubscribe_timeout_for_unknown_callids); //subscribe periodicly until unsubscribe
		}
		if (call_id && *call_id && locator && *locator)
		{
			//resubscribe
			// if not subscribe then skip
			auto sub = m_callidSub[call_id];
			if (!!sub)
			{
				auto cnt_iter = sub_cnts.find(locator);
				if (cnt_iter == sub_cnts.end())
				{
					cnt_iter = sub_cnts.emplace(locator, VS_Container()).first;
					cnt_iter->second.AddValue(METHOD_PARAM, SUBSCRIBE_METHOD);
				}
				cnt_iter->second.AddValue(CALLID_PARAM, call_id);
				std::string old_server;
				auto resolve_finder = VS_ResolveServerFinder::Instance();
				if (resolve_finder && (!resolve_finder->GetServerByUser(call_id, old_server) || locator != old_server))
				{
					auto srv_it = m_srvListForSub[old_server.c_str()];
					if (!!srv_it)
					{
						srv_it->data->m_callIDList.Erase(call_id);
						if (srv_it->data->m_callIDList.Size() == 0)
							m_srvListForSub.Erase(srv_it);
					}
					srv_it = m_srvListForSub[locator];
					if (!srv_it)
						srv_it = VS_ServerSubID();
					srv_it->data->m_srvName = locator;
					srv_it->data->m_callIDList.Assign(call_id, 1);
					resolve_finder->SetServerForUser(call_id, locator);
				}
			}
			m_unknown_call_id_for_sub.erase(call_id);
			call_id = nullptr;
			locator = nullptr;
		}
	}
	SubscribeFromOthersServers(std::move(sub_cnts));
}
void VS_SubscriptionHub::ResendStatus_Method(const VS_Container &cnt)
{
	assert_service_tread( __FUNCTION__ );
	cnt.Reset();
	while(cnt.Next())
	{
		if(!strcasecmp(cnt.GetName(),CALLID_PARAM))
		{
			std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
			const char *call_id = cnt.GetStrValueRef();
			if(!call_id || !*call_id)
				continue;
			VS_CallIdDataMap::Iterator	alias_i = m_callIdMap[call_id];
			if(!!alias_i)
			{
				for(VS_StrI_IntMap::Iterator i = alias_i->data->Begin();!!i;i++)\
					ResendStatus(i->key);
			}
			ResendStatus(call_id);
		}
	}
}


void VS_SubscriptionHub::ResendStatus(const char *call_id)
{
	assert_service_tread( __FUNCTION__ );
	/**
		check status cache
	*/
	long hops = VS_SubscriptionHub::DEFAULT_HOPS - 1;		// minus one for myself
	VS_CallIDInfo::Source source = VS_CallIDInfo::LOCAL;
	VS_StatusMap::Iterator st=m_statusCache[call_id];
	if(!!st)
	{
		if(st->data->m_status<=USER_LOGOFF)
		{
			VS_UserPresence_Status		offline_status(USER_STATUS_UNDEF);
			st = CheckOfflineStatus(call_id);
		}
		PushStatus(call_id,*st->data,VS_CallIDInfo::LOCAL,false,0,0,hops);
	}
}

bool VS_SubscriptionHub::Timer(unsigned long tick)
{
	assert_service_tread(__FUNCTION__);
	{
		std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
		bool RoamingNew = IsRoamingAllowed();
		bool RoamingChanged = m_isRoamingOn != RoamingNew;
		if ((m_isRoamingOn = RoamingNew))
		{
			if (RoamingChanged)
			{
				for (const auto& i : m_connectedServers)
				{
					VS_Container	cnt;
					cnt.AddValue(METHOD_PARAM, GETSUBSCRIPTIONS_METHOD);
					PostRequest(i.c_str(), 0, cnt, 0, PRESENCE_SRV, status_timeout);
				}
			}
			VS_StrINullMap	srv_for_unsub;
			for (auto iter = m_srvListForSub.Begin(); iter != m_srvListForSub.End(); ++iter)
			{
				if (0 == iter->data->m_status)
				{
					if (IsAuthorized(iter->key))
					{
						ServerOnline(iter->key);
						continue;
					}
					time_t now;
					time(&now);
					if (!iter->data->m_lastSubscribtionTime)
					{
						iter->data->m_lastSubscribtionTime = now;
						iter->data->m_current_sub_interval = start_interval_for_srv_sub;
						dprint4("@Trying connect to %s by timeout. Next time through %ld mins\n", iter->data->m_srvName.m_str, iter->data->m_current_sub_interval / 60);
						ConnectServer(iter->data->m_srvName);
					}
					else if (now - iter->data->m_lastSubscribtionTime >= iter->data->m_current_sub_interval)
					{
						if (iter->data->m_current_sub_interval < max_interval_for_srv_sub)
						{
							iter->data->m_current_sub_interval *= 2;
							if (iter->data->m_current_sub_interval > max_interval_for_srv_sub)
								iter->data->m_current_sub_interval = max_interval_for_srv_sub;
						}
						else
							iter->data->m_current_sub_interval = max_interval_for_srv_sub;
						iter->data->m_lastSubscribtionTime = now;
						dprint4("@Trying connect to %s by timeout. Next time through %ld mins\n", iter->data->m_srvName.m_str, iter->data->m_current_sub_interval / 60);
						ConnectServer(iter->data->m_srvName);
					}
				}
			}
			VS_ResolveServerFinder	*resolve_server = VS_ResolveServerFinder::Instance();
			if (!resolve_server->IsCacheActual())
			{
				PutTask(new CheckDomainsCacheForLifetime_Taks(), "CheckDomainsCacheForLifetime", 30);
			}
		}
		else
		{
			if (RoamingChanged)
			{
				for (auto iter = m_epSub.Begin(); iter != m_epSub.End(); iter++)
				{
					if (!iter->key->m_userID && iter->key->m_serverID != OurEndpoint())
					{
						VS_Container status_cnt;
						status_cnt.AddValue(METHOD_PARAM, UPDATESTATUS_METHOD);
						long hops = VS_SubscriptionHub::DEFAULT_HOPS - 1;		// minus one for myself
						status_cnt.AddValueI32(HOPS_PARAM, hops);
						unsigned status_count(0);
						for (VS_EndpointSub::CallIdMap::Iterator callId_iter = iter->data->m_callid.Begin();
							callId_iter != iter->data->m_callid.End(); callId_iter++)
						{
							status_cnt.AddValue(CALLID_PARAM, callId_iter->key);
							status_cnt.AddValueI32(USERPRESSTATUS_PARAM, CheckOfflineStatus(callId_iter->key).m_status);
							status_cnt.AddValue(SERVER_PARAM, "");
							status_count++;
						}
						if (status_count > 0)
							SeqPost(*iter->key, status_cnt);
					}
				}
			}
			for (VS_ServersListForSub::Iterator iter = m_srvListForSub.Begin(); iter != m_srvListForSub.End(); iter++)
			{
				if (1 == iter->data->m_status)
					ServerOffline(iter->key);
			}
		}
	}
	VS_ContainerMap for_sub, for_task;
	for (auto i = m_unknown_call_id_for_sub.begin(); i != m_unknown_call_id_for_sub.end();)
	{
		if (std::chrono::steady_clock::now() >= i->second)
		{
			PrepareForRemoteSubscribe(i->first.c_str(), for_sub, for_task);
			i = m_unknown_call_id_for_sub.erase(i);
		}
		else
			++i;
	}
	if (IsRoamingAllowed())
	{
		SubscribeFromOthersServers(std::move(for_sub));
		if (!for_task.empty())
		{
			PutTask(new RemoteSubscribe_Task(std::move(for_task)), "RemoteSubscribe1", 180); /// 3 минуты
		}
	}
	return true;
}
void VS_SubscriptionHub::ServerConnected_Method(const char *srv_name)
{
	assert_service_tread( __FUNCTION__ );
	dprint4("@Server %s connected\n",srv_name);
	m_connectedServers.insert(srv_name);
	ServerOnline(srv_name);
}
void VS_SubscriptionHub::ServerOnline(const char *srv_name)
{
	assert_service_tread( __FUNCTION__ );
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	if(IsRoamingAllowed())
	{
		dprint4("Server %s online\n",srv_name);
		VS_Container	cnt;
		cnt.AddValue(METHOD_PARAM,SUBSCRIBE_METHOD);
		cnt.AddValueI32(CAUSE_PARAM, 1);
		VS_ServersListForSub::Iterator	server_i = m_srvListForSub.Find(srv_name);
		VS_FullID	server_full_id(srv_name,0);
		VS_FullID	seq_full_id(server_full_id);
		seq_full_id.m_serverID +="sub";
		if(!!server_i)
		{
			dprint4("@Subscribe to:\n");
			for(VS_StrINullMap::Iterator call_id_i = server_i->data->m_callIDList.Begin();call_id_i!=server_i->data->m_callIDList.End();call_id_i++)
			{
				dprint4("\t%s\n",call_id_i->key);
				cnt.AddValue(CALLID_PARAM,call_id_i->key);
			}
			server_i->data->m_status = 1;
			server_i->data->m_lastSubscribtionTime =0;
			server_i->data->m_current_sub_interval = 0;
			dprint4("\tend subscribe\n");
			m_out_sync.Reset(server_full_id);
			SeqPost(server_full_id,cnt,seq_full_id);
		}
	}
}
void VS_SubscriptionHub::ServerDisconnected_Method(const char *srv_name)
{
	assert_service_tread( __FUNCTION__ );
	dprint4("@Server %s is disconnected.\n",srv_name);
	m_connectedServers.erase(srv_name);
	ServerOffline(srv_name);
}
void VS_SubscriptionHub::ServerOffline(const char *srv_name)
{
	assert_service_tread( __FUNCTION__ );
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	dprint4("@Server %s is offline.\n",srv_name);
	VS_ServersListForSub::Iterator	server_i = m_srvListForSub.Find(srv_name);
	if(!server_i)
		return;
	if(!!server_i)
		server_i->data->m_status = 0;
	long hops = VS_SubscriptionHub::DEFAULT_HOPS - 1;		// minus one for myself
	for(VS_StrINullMap::Iterator i = server_i->data->m_callIDList.Begin();i!=server_i->data->m_callIDList.End();++i)
	{
		auto cache_status = m_statusCache.Find(i->key);
		const auto our_endpoint = OurEndpoint();
		if (!!cache_status && (our_endpoint == nullptr || *our_endpoint == 0 ? cache_status->data->m_serverID.empty() : cache_status->data->m_serverID == our_endpoint))
		{
			dstream4 << "call_id = " << i->key << " is local user," << " skip pushing OFFLINE status\n";
			continue;
		}

		auto iter = m_callIdMap.Find(i->key);
		if(!!iter)
		{
			for (VS_StrI_IntMap::Iterator j = iter->data->Begin(); j != iter->data->End(); j++)
				PushStatus(j->key, CheckOfflineStatus(j->key).m_status, VS_ExtendedStatusStorage(), VS_CallIDInfo::UPLINK, 0, 0, hops);
		}
		else
			PushStatus(i->key, CheckOfflineStatus(i->key).m_status, VS_ExtendedStatusStorage(), VS_CallIDInfo::UPLINK, 0, 0, hops);
	}
}
void VS_SubscriptionHub::SubscribeFromOthersServers_Method(const VS_Container &cnt)
{
	assert_service_tread( __FUNCTION__ );
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	const char *server_name = cnt.GetStrValueRef(SERVERNAME_PARAM);
	VS_ContainerMap sub_cnts;
	VS_Container	sub;
	cnt.Reset();
	VS_ServersListForSub::Iterator	srv_sub_i = m_srvListForSub[server_name];
	while(cnt.Next())
	{
		if(strcasecmp(cnt.GetName(),CALLID_PARAM) == 0)
		{
			if(!srv_sub_i)
			{
				srv_sub_i = VS_ServerSubID();
				srv_sub_i->data->m_srvName = server_name;
			}
			sub.AddValue(CALLID_PARAM,cnt.GetStrValueRef());
			srv_sub_i->data->m_callIDList.Assign(cnt.GetStrValueRef(),1);
			auto resolver_finder = VS_ResolveServerFinder::Instance();
			if (resolver_finder)
				resolver_finder->SetServerForUser(cnt.GetStrValueRef(),server_name);
		}
	}
	if(!sub.IsEmpty())
	{
		sub.AddValue(METHOD_PARAM,SUBSCRIBE_METHOD);
		sub_cnts.emplace(server_name, std::move(sub));
		SubscribeFromOthersServers(std::move(sub_cnts));
	}
}
void VS_SubscriptionHub::SubscribeFromOthersServers(VS_ContainerMap && sub_cnts)
{
	assert_service_tread( __FUNCTION__ );
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	for (auto &cnt_iter:std::move(sub_cnts))
	{
		auto&	srv_name = cnt_iter.first;
		VS_FullID	srv_full_id(srv_name.c_str(),0);
		VS_FullID	seq_full_id(srv_full_id);
		seq_full_id.m_serverID +="sub";
		auto srv_sub_i = m_srvListForSub[srv_name.c_str()];
		if(!!srv_sub_i)
		{
			if(IsThereEndpoint(srv_name.c_str()))
			{
				dprint4("@Server %s already connected.\n",srv_name.c_str());
				srv_sub_i->data->m_status = 1;
			}
			if(1 == srv_sub_i->data->m_status)
			{
				dprint4("@Subscription from server %s\n",srv_name.c_str());
				SeqPost(srv_full_id,cnt_iter.second,seq_full_id);
			}
			else
			{
				if(!srv_sub_i->data->m_current_sub_interval)
				{
					dprint4("@Connecting to server %s for subscribe\n",srv_name.c_str());
					ConnectServer(srv_name.c_str());
					time(&srv_sub_i->data->m_lastSubscribtionTime);
					srv_sub_i->data->m_current_sub_interval = start_interval_for_srv_sub;
				}
				else
				{
					dprint4("@Connecting to server %s for subscribe is delayed\n",srv_name.c_str());
				}
			}
		}
	}
}
void VS_SubscriptionHub::ExtPresenceStarted_Method()
{
	assert_service_tread( __FUNCTION__ );
	std::vector<std::string> to_sub_from_ext_pres;
	{
		std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
		for (VS_StatusMap::Iterator iter = m_statusCache.Begin(); iter != m_statusCache.End(); iter++)
		{
			if (iter->key && *(iter->key))
				to_sub_from_ext_pres.emplace_back(iter->key);
		}
	}
	VS_ResolveServerFinder *resolve_server = VS_ResolveServerFinder::Instance();
	for (const auto& call_id : to_sub_from_ext_pres)
	{
		auto ext_presence = resolve_server->GetExternalPresence(call_id.c_str());
		if (!!ext_presence)
			ext_presence->Subscribe(call_id.c_str());
	}
}
bool VS_SubscriptionHub::IsRoamingAllowed(const char *server_name)
{
	return false;
}
