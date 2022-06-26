#include "../../common/std/cpplib/VS_RegistryKey.h"
#include "../../common/std/cpplib/VS_RegistryConst.h"
#include "std-generic/cpplib/VS_Container.h"
#include "../../common/std/cpplib/VS_Map.h"
#include "../../common/std/cpplib/VS_SimpleStr.h"

#include "../../ServerServices/Common.h"
#include "../../common/std/cpplib/VS_MemoryLeak.h"

#include "VS_RoutingService.h"
#include "std/debuglog/VS_Debug.h"

#define DEBUG_CURRENT_MODULE VS_DM_DIRS

static const auto c_reset_timeout = std::chrono::seconds(6);

////////////////////////////////////////////////////////////////////////////////
// Constructor/Destructor
////////////////////////////////////////////////////////////////////////////////
VS_RoutingService::VS_RoutingService(void)
{
	m_SmLastCheckTime = 0;
	m_connserv.SetPredicate(VS_SimpleStr::Predicate);
	m_connserv.SetKeyFactory(VS_SimpleStr::Factory, VS_SimpleStr::Destructor);
}

VS_RoutingService::~VS_RoutingService()
{
}
#pragma warning( disable : 4312 4311) //< void* <-> int conversion
int VS_RoutingService::AddConnectedServer(const char* server)
{
	if (!server)
		return -1;
	VS_Map::Iterator it = m_connserv.Find(server);
	if (it!=m_connserv.End()) {
		int res = (int)it->data;
		it->data = (void*)++res;
		return res;
	}
	else {
		m_connserv.Insert(server, 0);
		return 0;
	}
}

#pragma warning( default : 4312 4311)

bool VS_RoutingService::IsConnectedServer(const char* server)
{
	if (!server)
		return false;
	return m_connserv.Find(server)!=m_connserv.End();
}

void VS_RoutingService::DelConnectedServer(const char* server)
{
	if (!server)
		return;
	m_connserv.Erase(server);
}

bool VS_RoutingService::Init(const char *our_endpoint, const char *our_service, const bool permitAll)
{
	// set server manager
	char buff[256] = {0};
	VS_RegistryKey cfg(false, CONFIGURATION_KEY, false, true);
	if (cfg.GetValue(buff, 256, VS_REG_STRING_VT, SERVER_MANAGER_TAG) > 0 && *buff) {
		m_SM = buff;
	}
	else {
		dprint0("invalid server manager specified \n");
		return false;
	}
	ConnectServer(m_SM);
    return true;
}


bool VS_RoutingService::Processing(std::unique_ptr<VS_RouterMessage>&&recvMess)
{
    if (recvMess == 0)
		return true;
	VS_FullID full_id(recvMess->SrcServer(),recvMess->SrcUser());//= cid;

    switch (recvMess->Type())
    {
	case transport::MessageType::Invalid:
        break;
	case transport::MessageType::Request:
	case transport::MessageType::Reply:
		m_recvMess = recvMess.get();
        {
            VS_Container cnt;
            if (cnt.Deserialize(recvMess->Body(), recvMess->BodySize()))
			{
				const char* method = 0;
                if ((method = cnt.GetStrValueRef(METHOD_PARAM)) != 0)
				{
					dprint3("Method %20s| srv %20s | user %20s\n", method, recvMess->SrcServer(), recvMess->SrcUser());
					//Process methods
                    if (_stricmp(method, RESOLVE_METHOD) == 0)
					{
						Resolve_Method(cnt.GetStrValueRef(CALLID_PARAM));
					}
					else if (_stricmp(method, RESOLVEALL_METHOD) == 0)
					{
						ResolveAll_Method(cnt);
					}
					else if (_stricmp(method, UPDATESTATUS_METHOD) == 0)
					{
						VS_FullID id(recvMess->SrcServer(),recvMess->SrcUser());
						UpdateStatus_Method(cnt,id);
					}
					else if (_stricmp(method, SUBSCRIBE_METHOD) == 0)
					{
						Subscribe_Method(cnt);
					}
					else if (_stricmp(method, UNSUBSCRIBE_METHOD) == 0)
					{
						Unsubscribe_Method(cnt);
					}
					else if (_stricmp(method, REGISTERSTATUS_METHOD) == 0)
					{
						RegisterStatus_Method(cnt);
					}
					else if (_stricmp(method, UNREGISTERSTATUS_METHOD) == 0)
					{
						UnregisterStatus_Method(cnt.GetStrValueRef(CALLID_PARAM));
					}
					else if (_stricmp(method, GETALLUSERSTATUS_METHOD) == 0)
					{
						VS_FullID id(recvMess->SrcServer(),recvMess->SrcUser());
						GetAllUserStatus_Method(id);
					}
				}
			}
		}
        break;
	case transport::MessageType::Notify:
        break;
    }
    m_recvMess = nullptr;
    return true;
}


bool VS_RoutingService::Timer( unsigned long tickcount )
{
	if (tickcount - m_SmLastCheckTime > 30000) {
		if (!IsConnectedServer(m_SM))
			ConnectServer(m_SM);
		m_SmLastCheckTime = tickcount;
	}
	return true;
}


////////////////////////////////////////////////////////////////////////////
// Notifications from Transport
////////////////////////////////////////////////////////////////////////////////
bool VS_RoutingService::OnPointConnected_Event(const VS_PointParams* prm)
{

	if (prm->reazon <= 0) {
		dprint1("Error {%d} while Connect to (%s)\n", prm->reazon, prm->uid);
	}
	else if (prm->type==VS_PointParams::PT_SERVER) {
		int res = AddConnectedServer(prm->uid);
		if (res==0) {
			dprint1("Server (%s) NEW CONNECT | reason: %2d\n", prm->uid, prm->reazon);

			VS_Container cnt;
			cnt.AddValue(METHOD_PARAM,GETALLUSERSTATUS_METHOD);
			PostRequest(prm->uid, 0, cnt, 0, PRESENCE_SRV);

			VS_Container cnt2;
			cnt2.AddValue(METHOD_PARAM,GETSUBSCRIPTIONS_METHOD);
			PostRequest(prm->uid, 0, cnt2, 0, PRESENCE_SRV);
		}
		else {
			dprint1("Server (%s) RECONNECT #%d | reason: %2d\n", prm->uid, res, prm->reazon);
		}

	}
	else {
		dprint1("NOT SERVER Connect!: uid=%s, reason: %2d\n", prm->uid, prm->reazon);
	}
	return true;
}

bool VS_RoutingService::OnPointDisconnected_Event(const VS_PointParams* prm)
{
	if (prm->type==VS_PointParams::PT_SERVER) {
		DelConnectedServer(prm->uid);
		dprint1("Server (%s) DISCONNECT | reason: %2d\n", prm->uid, prm->reazon);

		CleanServer(prm->uid);
		VS_FullID subs(prm->uid, 0);
		UnsubscribeID(subs);
		//clean register items

		RegisterQueue::Iterator i_ri=m_reg_queue.Begin();

		while(!!i_ri)
		{
			if(i_ri->data->m_server==prm->uid)
			{
				i_ri=m_reg_queue.Erase(i_ri);
			}
			else
				++i_ri;
		}
	}
	return true;
}


void VS_RoutingService::CleanServer(const char* server)
{
	dprint3("@CleanServer %s\n",server);
	VS_IDNullMap ids;

	VS_AutoLock lock(this);

	int status_count=0;
	VS_Container status_cnt;
	status_cnt.AddValue(METHOD_PARAM, UPDATESTATUS_METHOD);
	long hops = VS_SubscriptionHub::DEFAULT_HOPS - 1;		// minus one for myself
	status_cnt.AddValueI32(HOPS_PARAM, hops);

	VS_StatusMap::Iterator i=m_statusCache.Begin();
	while(!!i)
	{
		if(server == nullptr || *server == 0 ? i->data->m_serverID.empty() : i->data->m_serverID == server)
		{
			dprint3("@CleanBroker: Cleaned status of %s\n",i->key);
			status_count++;
			status_cnt.AddValue(CALLID_PARAM, i->key);
			status_cnt.AddValueI32(USERPRESSTATUS_PARAM, USER_LOGOFF);
			status_cnt.AddValue(SERVER_PARAM, "" );

			VS_CallIDSubMap::ConstIterator isub=m_callidSub[(const char*)i->key];
			if(!!isub)
				for(VS_IDNullMap::ConstIterator ii=isub->data->m_subs.Begin();!!ii;ii++)
				{
					if(ii->key->m_userID != Listener::LISTENER_USER)
						ids.Assign(ii->key,1);
				}

			{
				VS_AutoLock regLock(&m_reg_lock);

				RegisterQueue::Iterator i_ri=m_reg_queue[i->key];

				if(!!i_ri)
				{
					RegisterItem* ri=i_ri->data;

					SendRegResult(USER_LOGGEDIN_OK, *ri);
					PushStatus(i->key,USER_AVAIL,VS_ExtendedStatusStorage(),VS_CallIDInfo::LOCAL,ri->m_server,true,ri->m_type,0,0,VS_SubscriptionHub::DEFAULT_HOPS,0,ri->m_displayName.c_str());

					m_reg_queue.Erase(i_ri);
				}
			}
			VS_SimpleStr call_id = i->key;
			i=m_statusCache.Erase(i);
			VS_CallIdDataMap::Iterator alias_lst_i = m_callIdMap[call_id];
			if(!!alias_lst_i)
				UnregisterAliases(call_id,false);

		}
		else
			i++;
	};

	if(status_count>0 && ids.Size()>0)
	{
		for(VS_IDNullMap::ConstIterator ii=ids.Begin();!!ii;ii++)
		{
			dprint3("@Pushing offline statuses to %s:%s\n",(const char*)ii->key->m_serverID,(const char*)ii->key->m_userID);
			SeqPostCopy(ii.key(),status_cnt);
		};
	}

}

////////////////////////////////////////////////////////////////////////////////
// RESOLVE_METHOD(CALLID_PARAM)
////////////////////////////////////////////////////////////////////////////////
void VS_RoutingService::Resolve_Method(const VS_SimpleStr& call_id)
{
	if(call_id.Length()==0)
		return;

	VS_CallIDInfo ci;

	{
		VS_AutoLock lock(this);
		VS_StatusMap::ConstIterator i=m_statusCache[call_id];
		if(!!i)
			ci=i;
	}

   // Make Body
	VS_Container cnt;
    cnt.AddValue(METHOD_PARAM,RESOLVE_METHOD);
	ci.ToContainer(cnt,true,true);

	dprint3("resolving '%s' got %d,'%s',type=%d\n", call_id.m_str, ci.m_status, ci.m_serverID.c_str(), ci.m_type);

	PostReply(cnt);
}

void VS_RoutingService::ResolveAll_Method(VS_Container& cnt)
{
	if (!cnt.IsValid())
		return ;

	VS_Container rCnt;
	cnt.AddValue(METHOD_PARAM,RESOLVEALL_METHOD);

	unsigned int total = 0;
	unsigned int resolved = 0;
	cnt.Reset();
	while(cnt.Next())
	{
		if(_stricmp(cnt.GetName(),CALLID_PARAM) == 0)
		{
			total++;
			VS_SimpleStr call_id = cnt.GetStrValueRef();

			VS_CallIDInfo ci;
			VS_AutoLock lock(this);
			VS_StatusMap::ConstIterator i=m_statusCache[call_id];
			if(!!i)
			{
				ci=i;
				resolved++;
			}
			ci.ToContainer(rCnt, true, true);
		}
	}

	dprint3("resolved %d from %d users\n",resolved,total);
	PostReply(rCnt);
}

void VS_RoutingService::PushWithCheck(const VS_SimpleStr& call_id, VS_CallIDInfo& ci)
{
	VS_AutoLock lock(this);
	std::string			curr_server;
	VS_StatusMap::ConstIterator it=m_statusCache[call_id];
	if(!!it)
		curr_server=it->data->m_serverID;

	if(curr_server==ci.m_serverID)
	{
		VS_CallIdDataMap::Iterator alias_i = m_callIdMap[call_id];
		if(!!alias_i)
		{
			VS_CallIDInfo	push_ci = ci;
			PushStatus(call_id,push_ci,VS_CallIDInfo::LOCAL,false,ci.m_serverID.c_str(),0,VS_SubscriptionHub::DEFAULT_HOPS);

			for(VS_StrI_IntMap::Iterator ii = alias_i->data->Begin();!!ii;ii++)
			{
				VS_SimpleStr	alias = ii->key;
				push_ci = ci;
				PushStatus(alias,push_ci,VS_CallIDInfo::LOCAL,false,ci.m_serverID.c_str(),0,VS_SubscriptionHub::DEFAULT_HOPS);
			}
		}
		else
		{
			PushStatus(call_id,ci,VS_CallIDInfo::LOCAL,false,ci.m_serverID.c_str(),0,VS_SubscriptionHub::DEFAULT_HOPS);
		}
	}
	else
	{
		if(curr_server.empty())
		{
			if(ci.m_status>=USER_AVAIL)
			{
				if(RegisterStatus(call_id,ci.m_serverID.c_str(),0,0,ci.m_type,ci.m_displayName))
				{
					VS_CallIdDataMap::Iterator alias_i = m_callIdMap[call_id];
					if(!!alias_i)
					{
						VS_CallIDInfo	push_ci = ci;
						for (VS_StrI_IntMap::Iterator ii = alias_i->data->Begin(); !!ii; ii++)
						{
							VS_SimpleStr	alias = ii->key;
							VS_CallIDInfo push_ci = ci;
							PushStatus(alias,push_ci,VS_CallIDInfo::LOCAL,true,ci.m_serverID.c_str(),0,VS_SubscriptionHub::DEFAULT_HOPS);
						}
						PushStatus(call_id,ci,VS_CallIDInfo::LOCAL,true,ci.m_serverID.c_str(),0,VS_SubscriptionHub::DEFAULT_HOPS);
					}
					else
						PushStatus(call_id,ci,VS_CallIDInfo::LOCAL,true,ci.m_serverID.c_str(),0,VS_SubscriptionHub::DEFAULT_HOPS);
				}
			}
			else
				UnregisterStatus_Method(call_id);
		}
		else
		{
			if(ci.m_status>=USER_AVAIL)
			{
				VS_Container cnt;
				cnt.AddValue(METHOD_PARAM, LOGOUTUSER_METHOD);
				cnt.AddValue(USERNAME_PARAM, call_id);

				PostRequest(ci.m_serverID.c_str(), 0, cnt, 0, AUTH_SRV);
			}
			else
			{
				dprint3("call_id %s: current server %s exists - offline status set from %s ignored\n", call_id.m_str, curr_server.c_str(), ci.m_serverID.c_str());
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////
////// UDPATESTATUS_METHOD(CALLID_PARAM,USERPRESSTATUS_PARAM,ENDPOINT_PARAM)
////////////////////////////////////////////////////////////////////////////////////
void VS_RoutingService::UpdateStatus_Method(VS_Container& cnt,const VS_FullID& src_id)
{
	long status=0;
	int32_t lval;
	VS_SimpleStr call_id;
	std::set<std::string> current_statuses;
	if(!!src_id.m_userID || !VS_SyncPool::CheckExistenceParamsForSync(cnt))
	{
		dprint0("No status update from users allowed %s:%s\n", src_id.m_serverID.m_str, src_id.m_userID.m_str);
		return;
	}

	int32_t cause(0);
	if(!cnt.GetValue(CAUSE_PARAM,cause))
		cause=0;
	if(cause==1)
		GetServerCallIDs(src_id.m_serverID, current_statuses);

	if (!m_in_sync.ConsistentCheck(src_id, cnt))
	{
		VS_Container reset_cnt;
		reset_cnt.AddValue(METHOD_PARAM, GETALLUSERSTATUS_METHOD);
		PostRequest(src_id.m_serverID, src_id.m_userID, reset_cnt, NULL, PRESENCE_SRV);
	}

	int i=0;

	VS_CallIDInfo ci;

	cnt.Reset();
	VS_StrI_IntMap	aliases;
	while(cnt.Next())
	{
		if(_stricmp(cnt.GetName(),CALLID_PARAM)==0 || _stricmp(cnt.GetName(),USERNAME_PARAM)==0)
		{
			if(i>0) //not first item
			{
				VS_AutoLock	lock(this);

				current_statuses.erase(call_id.m_str);
				VS_CallIdDataMap::Iterator	alias_i = m_callIdMap[call_id];
				if(!!alias_i)
				{
					for (VS_StrI_IntMap::Iterator iter = alias_i->data->Begin(); !!iter; iter++)
					{
						VS_SimpleStr alias = iter->key;
						current_statuses.erase(alias.m_str);
					}
				}
				if(aliases.Size() > 0)
				{
					RegisterAliases(call_id,aliases);
					aliases.Clear();
				}
				if (ci.m_realID.empty())
					ci.m_realID = std::string{ call_id.m_str, call_id.Length() };
				PushWithCheck(call_id,ci);
			}
			i++;
			call_id=cnt.GetStrValueRef();
			ci.Empty();
			ci.m_serverID = src_id.m_serverID;
		}
		else if(_stricmp(cnt.GetName(),USERPRESSTATUS_PARAM)==0)
		{
			if(cnt.GetValue(lval))
				ci.m_status=(VS_UserPresence_Status)lval;
		}
		else if(_stricmp(cnt.GetName(), EXTSTATUS_PARAM) == 0)
		{
			VS_Container st_cnt;
			if (cnt.GetValue(st_cnt))
				ci.m_extStatusStorage.UpdateStatus(st_cnt);
		}
		/* SERVER_PARAM not supported */
		else if(_stricmp(cnt.GetName(),TYPE_PARAM)==0)
		{
			if(cnt.GetValue(lval))
				ci.m_type=lval;
		}
		else if(_stricmp(cnt.GetName(),REALID_PARAM)==0)
		{
			ci.m_realID=cnt.GetStrValueRef();
		}
		else if(_stricmp(cnt.GetName(),TIME_PARAM) == 0)
		{
			size_t size = 0;
			const void * time = cnt.GetBinValueRef(size);
			if (time && size==sizeof(int64_t))
			{
				ci.m_logginTime = tu::WindowsTickToUnixSeconds(*static_cast<const int64_t*>(time));
			}
		}
		else if(_stricmp(cnt.GetName(), ALIAS_PARAM) == 0)
		{
			aliases.Insert(cnt.GetStrValueRef(),1);
		}
		else if(_stricmp(cnt.GetName(),DISPLAYNAME_PARAM)==0)
		{
			auto pVal = cnt.GetStrValueRef();
			if (pVal != nullptr) ci.m_displayName = pVal;
		}
	};

	if(i>0)
	{
		VS_AutoLock	lock(this);

		current_statuses.erase(call_id.m_str);
		VS_CallIdDataMap::Iterator	alias_i = m_callIdMap[call_id];
		if(!!alias_i)
		{
			for (VS_StrI_IntMap::Iterator iter = alias_i->data->Begin(); !!iter; iter++)
			{
				VS_SimpleStr alias = iter->key;
				current_statuses.erase(alias.m_str);
			}
		}
		if(aliases.Size() > 0)
		{
			RegisterAliases(call_id,aliases);
			aliases.Clear();
		}
		if(ci.m_realID.empty())
			ci.m_realID = std::string{ call_id.m_str, (size_t)call_id.Length() };
		PushWithCheck(call_id,ci);
	}

	if( cause==1 && current_statuses.size()>0)
	{
		ci.Empty();
		ci.m_serverID = std::string{ src_id.m_serverID.m_str, src_id.m_serverID.Length() };
		ci.m_status=USER_LOGOFF;

		for(auto &ii:current_statuses)
		{

			dprint3("checking %s for offline status\n",ii.c_str());
			PushWithCheck(ii.c_str(),ci);
		};

	}


	dprint3("UpdateStatus %d statuses arrived from %s (seq_id=%08x)\n",i,m_recvMess->SrcServer(),m_in_sync.GetCurrentSeqId(src_id));
};

////////////////////////////////////////////////////////////////////////////////////
////// SUBSCRIBE_METHOD((CALLID_PARAM,ENDPOINT_PARAM) [])
////////////////////////////////////////////////////////////////////////////////////
void VS_RoutingService::Subscribe_Method( VS_Container& cnt )
{
	VS_FullID server_id(m_recvMess->SrcServer(),m_recvMess->SrcUser());

	int32_t cause = 0;
	cnt.GetValue(CAUSE_PARAM,cause);

	int32_t seq_id;
	if(!cnt.GetValue(SEQUENCE_ID_PARAM,seq_id))
		seq_id=-1;

	int32_t seq_test = -1;
	if(seq_id>0)
		seq_test=m_sub_sync.Inc(server_id);

	if(cause==1)
	{
		UnsubscribeID(server_id);
		dprint2("subs reset for %s, init seq_id=%08x\n",m_recvMess->SrcServer(),seq_id);
		m_sub_sync.Init(server_id, seq_id);
		seq_test=seq_id;
	}

	if((seq_id>0 && seq_test!=seq_id)||(m_sub_sync.IsWaitForReset(server_id)))
	{
		if(seq_id>0 && seq_test!=seq_id)
		{
			/**
				Пометить для резета и задать seq_id, который пришел;
			*/
			m_sub_sync.SetSeqId(server_id,seq_id);
			m_sub_sync.MarkForReset(server_id);

			dprint1("sync lost in sub for %s L%08x!=R%08x\n",m_recvMess->SrcServer(), seq_test, seq_id);
		}

		auto now = std::chrono::system_clock::now();
		auto it=m_sub_reset_time[server_id];
		if (!!it && now - *it->data < c_reset_timeout)
		{
		  dprint3("Reset subs supressed\n") ;
		}
		else
		{
		  it=now;

			  VS_Container reset_cnt;
			  reset_cnt.AddValue(METHOD_PARAM,GETSUBSCRIPTIONS_METHOD);
			  PostRequest(server_id.m_serverID,server_id.m_userID,reset_cnt,NULL,PRESENCE_SRV);
		}
	};

	dprint3("subscribe from %s, seq_id=%08x\n", m_recvMess->SrcServer(),seq_id);
	Subscribe(cnt,server_id,cause==1);
}

////////////////////////////////////////////////////////////////////////////////////
////// UNSUBSCRIBE_METHOD((CALLID_PARAM,ENDPOINT_PARAM) [])
////////////////////////////////////////////////////////////////////////////////////
void VS_RoutingService::Unsubscribe_Method( VS_Container& cnt )
{
	VS_FullID server_id(m_recvMess->SrcServer(),m_recvMess->SrcUser());
	int32_t seq_id;
	if(!cnt.GetValue(SEQUENCE_ID_PARAM,seq_id))
		seq_id=-1;

	int32_t seq_test = -1;
	if(seq_id>0)
		seq_test=m_sub_sync.Inc(server_id);

	dprint3("unsubscribe from %s, seq_id=%08x\n", m_recvMess->SrcServer(),seq_id);

	if((seq_id>0 && seq_test!=seq_id) ||(m_sub_sync.IsWaitForReset(server_id)))
	{
		if(seq_id>0 && seq_test!=seq_id)
		{
			m_sub_sync.SetSeqId(server_id,seq_id);
			m_sub_sync.MarkForReset(server_id);
			dprint1("sync lost in unsub for %s L%08x!=R%08x\n",m_recvMess->SrcServer(), seq_test, seq_id);
		}
		auto now = std::chrono::system_clock::now();
		auto it=m_sub_reset_time[server_id];
		if (!!it && now - *it->data < c_reset_timeout)
		{
			dprint3("Reset subs supressed\n") ;
		}
		else
		{
			it=now;
			VS_Container reset_cnt;
			reset_cnt.AddValue(METHOD_PARAM,GETSUBSCRIPTIONS_METHOD);
			PostRequest(server_id.m_serverID,server_id.m_userID,reset_cnt,NULL,PRESENCE_SRV);
		}
	};

	Unsubscribe(cnt,server_id);
}

///////////////////////////////////////////////////
// REGISTERSTATUS_METHOD((CALLID_PARAM,ENDPOINT_PARAM,SEQUENCE_PARAM) [])
///////////////////////////////////////////////////

void VS_RoutingService::RegisterStatus_Method(VS_Container &cnt)
{
	VS_SimpleStr	serverID = m_recvMess->SrcServer();
	VS_SimpleStr	call_id = cnt.GetStrValueRef(REALID_PARAM);
	VS_SimpleStr	homeServer = cnt.GetStrValueRef(LOCATORBS_PARAM);
	const char*		displayName = cnt.GetStrValueRef(DISPLAYNAME_PARAM); if (!displayName) displayName = "";
	int32_t			user_type = -1;

	cnt.Reset();
	while(cnt.Next())
	{
		if(_stricmp(cnt.GetName(),TYPE_PARAM) == 0)
		{
			cnt.GetValue(user_type);
			break;
		}
	}

	if(!call_id || !serverID)
		return;

	RegisterAliases(cnt);
	if(RegisterStatus(call_id,serverID,cnt.GetStrValueRef(ENDPOINT_PARAM),cnt.GetStrValueRef(SEQUENCE_PARAM),user_type,displayName))
	{
		VS_AutoLock	lock(this);

		VS_CallIdDataMap::Iterator alias_i = m_callIdMap[call_id];
		if(!!alias_i)
		{
			PushStatus(call_id,USER_AVAIL,VS_ExtendedStatusStorage(),VS_CallIDInfo::LOCAL,serverID,true,user_type,serverID,0,VS_SubscriptionHub::DEFAULT_HOPS,homeServer,displayName);
			for (VS_StrI_IntMap::Iterator ii = alias_i->data->Begin(); !!ii; ii++)
			{
				VS_SimpleStr	alias = ii->key;
				PushStatus(alias,USER_AVAIL,VS_ExtendedStatusStorage(),VS_CallIDInfo::LOCAL,serverID,true,user_type,serverID,0,VS_SubscriptionHub::DEFAULT_HOPS,homeServer,displayName);
			}
		}
		else
			PushStatus(call_id,USER_AVAIL,VS_ExtendedStatusStorage(),VS_CallIDInfo::LOCAL,serverID,true,user_type,serverID,0,VS_SubscriptionHub::DEFAULT_HOPS,homeServer,displayName);
	}
}

bool VS_RoutingService::RegisterStatus(const VS_SimpleStr& call_id, const VS_SimpleStr& serverID,const char* temp_id, const char* seq, long user_type, const std::string& displayName)
{
	bool registered=true;

	VS_CallIDInfo ci;

	RegisterItem new_ri(serverID, temp_id, seq, user_type, displayName);

	VS_AutoLock lock(this);
	{
		VS_AutoLock regLock(&m_reg_lock);

		VS_StatusMap::ConstIterator i=m_statusCache[call_id];
		if(!!i)
			ci=i;

		if (!ci.m_serverID.empty() && string_view{ serverID.m_str, (size_t)serverID.Length() } != ci.m_serverID)
		{
			RegisterQueue::Iterator i_ri=m_reg_queue[call_id];

			if(!!i_ri)
			{
				RegisterItem old_ri=i_ri;

				/**
					Старому серверу отправляем, что такому-то юзеру реджект
				*/
				if (string_view{ old_ri.m_server.m_str, (size_t)old_ri.m_server.Length() } != ci.m_serverID)
					SendRegResult(SILENT_REJECT_LOGIN, old_ri);
			}

			/**
				Добавляем новое соответствие клиент-сервер;
			*/
			i_ri=new_ri;

			VS_Container cnt;
			cnt.AddValue(METHOD_PARAM, LOGOUTUSER_METHOD);
			cnt.AddValue(USERNAME_PARAM, call_id);

			PostRequest(ci.m_serverID.c_str(), 0, cnt, 0, AUTH_SRV);

			registered=false;
		}
	}

	if(registered)
	{
		SendRegResult(USER_LOGGEDIN_OK,new_ri);
	}

	return registered;

}

///////////////////////////////////////////////////
// UNREGISTERSTATUS_METHOD((CALLID_PARAM,ENDPOINT_PARAM) [])
///////////////////////////////////////////////////
void VS_RoutingService::UnregisterStatus_Method(const VS_SimpleStr& call_id)
{
	/**
		Пользователь ушел с сервера, надо снять с регистрации
	*/
	if(!call_id)
		return;

	VS_SimpleStr src_server=m_recvMess->SrcServer();

	if(!src_server)
		return;

	{
		VS_AutoLock lock(this);

		VS_SimpleStr			curr_server;

		/**
			Отвязать от статусов
		*/
		VS_StatusMap::ConstIterator it=m_statusCache[call_id];
		if (!!it && string_view{ src_server.m_str, (size_t)src_server.Length() } != it->data->m_serverID)
		{
			dprint1("server %s is trying to unreg user %s from %s - blocked\n", src_server.m_str, call_id.m_str, it->data->m_serverID.c_str());
			return;
		}
	}
	/**
		Отвязать от статусов
	*/

	VS_AutoLock	lock(this);

	VS_CallIdDataMap::Iterator	alias_i = m_callIdMap[call_id];
	if(!!alias_i)
	{
		PushStatus(call_id,USER_LOGOFF,VS_ExtendedStatusStorage(),VS_CallIDInfo::LOCAL,0,true,-1,src_server,0,VS_SubscriptionHub::DEFAULT_HOPS);
		for (VS_StrI_IntMap::Iterator ii = alias_i->data->Begin(); !!ii; ii++)
		{
			VS_SimpleStr	alias = ii->key;
			PushStatus(alias,USER_LOGOFF,VS_ExtendedStatusStorage(),VS_CallIDInfo::LOCAL,0,true,-1,src_server,0,VS_SubscriptionHub::DEFAULT_HOPS);
		}
	}
	else
		PushStatus(call_id,USER_LOGOFF,VS_ExtendedStatusStorage(),VS_CallIDInfo::LOCAL,0,true,-1,src_server,0,VS_SubscriptionHub::DEFAULT_HOPS);



	VS_AutoLock regLock(&m_reg_lock);

	RegisterQueue::Iterator i_ri=m_reg_queue[call_id];

	if(!!i_ri)
	{
		RegisterItem ri=i_ri;

		SendRegResult(USER_LOGGEDIN_OK, ri);
		VS_CallIdDataMap::Iterator	alias_i = m_callIdMap[call_id];
		if(!!alias_i)
		{
			PushStatus(call_id,USER_AVAIL,VS_ExtendedStatusStorage(),VS_CallIDInfo::LOCAL,ri.m_server,true,ri.m_type,0,0,VS_SubscriptionHub::DEFAULT_HOPS,0,ri.m_displayName.c_str());
			for (VS_StrI_IntMap::Iterator ii = alias_i->data->Begin(); !!ii; ii++)
			{
				VS_SimpleStr	alias = ii->key;
				PushStatus(alias,USER_AVAIL,VS_ExtendedStatusStorage(),VS_CallIDInfo::LOCAL,ri.m_server,true,ri.m_type,0,0,VS_SubscriptionHub::DEFAULT_HOPS,0,ri.m_displayName.c_str());
			}
		}
		else
			PushStatus(call_id,USER_AVAIL,VS_ExtendedStatusStorage(),VS_CallIDInfo::LOCAL,ri.m_server,true,ri.m_type,0,0,VS_SubscriptionHub::DEFAULT_HOPS,0,ri.m_displayName.c_str());

		m_reg_queue.Erase(i_ri);
	}
	else
		UnregisterAliases(call_id); //убрать алиасы
}

void VS_RoutingService::SendRegResult(VS_UserLoggedin_Result result, const RegisterItem& ri)
{
	if (!ri.m_temp_id || !ri.m_seq)
		return;

	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, USERLOGGEDIN_METHOD);
	cnt.AddValueI32(RESULT_PARAM, result);
	cnt.AddValue(ENDPOINT_PARAM, ri.m_temp_id);
	cnt.AddValue(SEQUENCE_PARAM, ri.m_seq);

	PostRequest(ri.m_server, 0, cnt, 0, AUTH_SRV);

}
///////////////////////////////////////////////////
// GETALLUSERSTATUS_METHOD(ENDPOINT_PARAM,...)
///////////////////////////////////////////////////
void VS_RoutingService::GetAllUserStatus_Method(const VS_FullID& id)
{

	dprint3("@GetAllUserStatus from %s\n",m_recvMess->SrcServer());

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, GETALLUSERSTATUS_METHOD);
	rCnt.AddValueI32(CAUSE_PARAM, 1);

	int users= GetNextSubscriberStatuses(id, rCnt, false, false, 200 );

	int32_t seq_id = m_out_sync.Reset(id);
	rCnt.AddValueI32(SEQUENCE_ID_PARAM, seq_id);
	PostReply(rCnt);
	dprint3(" sending seq_id=%08x, found %d\n", seq_id, users);

	while(IsCurrentSubscriberStatusesExist())
	{
		VS_Container	cnt;
		cnt.AddValue(METHOD_PARAM,UPDATESTATUS_METHOD);
		long hops = VS_SubscriptionHub::DEFAULT_HOPS - 1;		// minus one for myself
		cnt.AddValueI32(HOPS_PARAM, hops);
		users+= GetNextSubscriberStatuses(id, cnt, false, false, 200 );
		SeqPost(id,cnt);
	}
	dprint3(" found %d users\n", users);
}

bool VS_RoutingService::IsRoamingAllowed(const char *for_server_name)
{
	return VS_SubscriptionHub::IsRoamingAllowed(for_server_name);
}
