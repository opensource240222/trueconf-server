#ifdef _WIN32 // not ported
#include "VS_BaseConferenceService.h"

#include "../../ServerServices/Common.h"
#include "../../common/transport/Router/VS_TransportRouterServiceHelper.h"
#include "transport/Router/VS_RouterMessage.h"
#include "../../common/std/cpplib/VS_Utils.h"
#include "../../common/std/cpplib/VS_MemoryLeak.h"
#include "std-generic/clib/vs_time.h"
#include "VS_ManagerService.h"
#include "VS_BasePresenceService.h"
#include "CheckSrv/CheckSrv/VS_ClientCheckSrv.h"
#include "acs/ConnectionManager/VS_ConnectionManager.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std-generic/cpplib/VS_Container_io.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_RegistryKey.h"

#define DEBUG_CURRENT_MODULE VS_DM_MCS


//////////////////// TASKS ////////////////////
class InviteParticipants_Task: public VS_PoolThreadsTask, public virtual VS_TransportRouterServiceReplyHelper
{
public:
	ConferenceInvitation	m_ci;

	InviteParticipants_Task(ConferenceInvitation& ci)
	{
		m_ci = ci;
	}

	void Run() {
		if (!g_BasePresenceService)
			return;

		std::list<std::string> call_ids;
		for (const auto &i : m_ci.m_invitation_parts)
			call_ids.push_back(i.first.GetID());
		auto call_ids_copy = call_ids;
		auto statuses = g_BasePresenceService->ResolveAllSync(std::move(call_ids_copy), this);
		// roaming tcs users or aliases
		for (const auto &i : statuses)
			if (!i.m_realID.empty())
				call_ids.remove(i.m_realID);
		for (auto const& i : call_ids)
		{
			VS_SimpleStr tmp = i.c_str();
			VS_CallIDInfo ci;
			g_BasePresenceService->Resolve(tmp, ci, false, this);
			statuses.emplace_back(ci);
		}
		for (const auto &i : statuses)
		{
			if (i.m_status == VS_UserPresence_Status::USER_AVAIL)
			{
				dprint3("INV: Invite %s(%d) on %s to %s conference\n", i.m_realID.c_str(), i.m_status, i.m_serverID.c_str(), m_ci.m_conf_id.c_str());
				if (!VS_GetConfEndpoint(m_ci.m_conf_id.c_str()))
				{
					m_ci.m_conf_id += "@";
					m_ci.m_conf_id += OurEndpoint();
				}
				VS_Container rCnt;
				rCnt.AddValue(METHOD_PARAM, INVITETOMULTI_METHOD);
				rCnt.AddValue(CALLID2_PARAM, i.m_realID);
				rCnt.AddValue(CONFERENCE_PARAM, m_ci.m_conf_id);
				if (!m_ci.m_owner.empty())
					rCnt.AddValue(USERNAME_PARAM, m_ci.m_owner);
				else
					rCnt.AddValue(USERNAME_PARAM, "Administrator");
				rCnt.AddValue(NAME_PARAM, m_ci.m_conf_id);
				rCnt.AddValue(TYPE_PARAM, CT_MULTISTREAM);
				rCnt.AddValue(TOPIC_PARAM, m_ci.m_topic);

				VS_TransportRouterServiceHelper::PostRequest(i.m_serverID.c_str(), i.m_realID.c_str(), rCnt, 0, CONFERENCE_SRV);
			}
		}

	}
};
//////////////////// END TASKS ////////////////////
VS_BaseConferenceService::VS_BaseConferenceService(boost::asio::io_service& ios)
	: m_strand(ios)
	, INVITATION_PERIOD(5000)
	, m_invitation_tick(0)
{
	m_TimeInterval = std::chrono::seconds(5);

	VS_AutoLock lock(&m_confs_lock);

	m_confs.SetPredicate(vs_conf_id::Predicate);
	m_confs.SetKeyFactory(vs_conf_id::Factory, vs_conf_id::Destructor);
	m_confs.SetDataFactory(ConferenceInvitation::Factory, ConferenceInvitation::Destructor);
}
bool VS_BaseConferenceService::Init( const char *our_endpoint, const char *our_service, const bool permittedAll )
{
	std::vector<std::string> v;
	auto dbStorage = g_dbStorage;
	if(!dbStorage)
		return false;
	dbStorage->InitNamedConfInvitaions(v);
	for (std::vector<std::string>::iterator it = v.begin(); it!=v.end(); it++)
		ScheduleInvitaion(it->data());

	std::string SM;
	char buff[256] = { 0 };
	VS_RegistryKey cfg(false, CONFIGURATION_KEY, false, true);
	if (cfg.GetValue(buff, 256, VS_REG_STRING_VT, SERVER_MANAGER_TAG) > 0 && *buff) {
		SM = buff;
	}
	else {
		dprint0("invalid server manager specified \n");
		return false;
	}
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, GETASOFMYDOMAIN_METHOD);
	PostRequest(SM.c_str(), nullptr, cnt, nullptr, MANAGER_SRV);

	VS_InstallConnectionManager("checksrv");
	return true;
}
VS_BaseConferenceService::~VS_BaseConferenceService(void)
{
	VS_AutoLock lock(&m_confs_lock);
	m_confs.Clear();
}

bool VS_BaseConferenceService::Processing( std::unique_ptr<VS_RouterMessage>&&recvMess)
{
	if (recvMess == 0)  return true;
	VS_Container cnt;

	switch (recvMess->Type())
	{
	case transport::MessageType::Invalid:
		break;
	case transport::MessageType::Request:
	case transport::MessageType::Reply:
		m_recvMess = recvMess.get();
		if (cnt.Deserialize(recvMess->Body(), recvMess->BodySize())) {
			const char* method = cnt.GetStrValueRef(METHOD_PARAM);
			if (method && _stricmp(method, JOIN_METHOD) == 0) {
				Join_Method(cnt);
			}else if (_stricmp(method, INVITATIONUPDATE_METHOD) == 0) {
				InvitationUpdate_Method(cnt);
			}else if (_stricmp(method, CONFERENCECREATED_METHOD) == 0) {
				ConferenceCreated_Method(cnt.GetStrValueRef(NAME_PARAM),
										 cnt.GetStrValueRef(CONFERENCE_PARAM));
			}else if (_stricmp(method, GETASOFMYDOMAIN_METHOD) == 0) {
				GetASOfMyDomain_Method(cnt);
			}
		}
		break;
	case transport::MessageType::Notify:
		break;
	}
	return true;
}

void VS_BaseConferenceService::Join_Method(VS_Container &cnt)
{
	const char *name = cnt.GetStrValueRef(NAME_PARAM);
	const char *from = cnt.GetStrValueRef(USERNAME_PARAM);
	const char *version = cnt.GetStrValueRef(FIELD1_PARAM);
	auto src_server = m_recvMess->SrcServer();
	if (!name || !*name || !from || !*from)
		return;

	dprint2("Join from %s(%s) to %s\n", m_recvMess->SrcUser(), src_server, name);

	VS_ConferenceDescription cd;
	ConferenceInvitation ci;
	VS_SimpleStr as_server_for_conference;
	long scope = GS_PUBLIC;

	auto dbStorage = g_dbStorage;
	if(!dbStorage)
		return;
	int error = dbStorage->GetNamedConfInfo(name, cd, ci, as_server_for_conference, scope);
	if (error)
	{
		VS_Container rCnt;
		rCnt.AddValue(METHOD_PARAM, JOIN_METHOD);
		rCnt.AddValueI32(RESULT_PARAM, error);
		PostRequest(src_server, from, rCnt);
		return;
	}

	const auto server_type = VS_GetServerType(m_recvMess->SrcServer_sv());
	if (!as_server_for_conference)
	{
		if (server_type == ST_VCS && !m_AS_online.empty()) {
			{
				auto ds = dstream4;
				ds << "available " << m_AS_online.size() << " AS:\n";
				for (auto const& p : m_AS_online)
					ds << p.first << ", ping=" << std::to_string(p.second) << "ms\n";
			}

			auto best_ping_my = -1;
			auto best_ping_all = -1;
			VS_SimpleStr as_my;
			VS_SimpleStr as_all;
			for (auto const& p : m_AS_online)
			{
				if (m_AS_my_domain.find(p.first) != m_AS_my_domain.end())
				{
					if (p.second < best_ping_my || as_my.IsEmpty())
					{
						as_my = p.first.c_str();
						best_ping_my = p.second;
					}
				}

				if (p.second < best_ping_all || as_all.IsEmpty())
				{
					as_all = p.first.c_str();
					best_ping_all = p.second;
				}
			}
			if (!as_my.IsEmpty())
				as_server_for_conference = as_my;
			else if (!as_all.IsEmpty())
				as_server_for_conference = as_all;
			else
				as_server_for_conference = m_AS_online.begin()->first.c_str();
			dstream4 << "choose best AS for NamedConf " << as_server_for_conference.m_str << ", as_my=" << as_my.m_str << ", as_all=" << as_all.m_str;

			VS_RegistryKey key(false, CONFIGURATION_KEY);
			const unsigned long buff_sz(512);
			char buff[buff_sz] = { 0 };
			if (key.IsValid() && key.GetValue(buff, buff_sz, VS_REG_STRING_VT, "default_as_server_for_conference") && buff && *buff)
			{
				as_server_for_conference = buff;
				dstream4 << "use as_server_for_conference from Registry = " << as_server_for_conference.m_str;
			}
		}
		if(server_type != ST_AS && as_server_for_conference.IsEmpty()) {
			dstream4 << "reject join to NamedConf from TCS " << src_server;
			VS_Container rCnt;
			rCnt.AddValue(METHOD_PARAM, JOIN_METHOD);
			rCnt.AddValueI32(RESULT_PARAM, VSS_CONF_NOT_STARTED);
			PostRequest(src_server, from, rCnt);
			return ;
		}

		if (!as_server_for_conference)
			as_server_for_conference = src_server;
		if (!as_server_for_conference)
		{
			VS_Container rCnt;
			rCnt.AddValue(METHOD_PARAM, JOIN_METHOD);
			rCnt.AddValueI32(RESULT_PARAM, VSS_CONF_NOT_VALID);
			PostRequest(src_server, from, rCnt);
			return;
		}
	}

	cnt.AddValueI32(TYPE_PARAM, cd.m_type);
	cnt.AddValueI32(SUBTYPE_PARAM, cd.m_SubType);
	cnt.AddValueI32(SCOPE_PARAM, scope);
	cnt.AddValue(TOPIC_PARAM, cd.m_topic);

	cnt.AddValue(OWNER_PARAM, cd.m_owner);
	cnt.AddValueI32(MAXPARTISIPANTS_PARAM, cd.m_MaxParticipants);
	cnt.AddValueI32(MAXCAST_PARAM, cd.m_MaxCast);
	cnt.AddValue(TIME_PARAM,cd.m_timeExp);

	cnt.AddValue(SERVER_PARAM, src_server);			// home AS of user
	PostRequest(as_server_for_conference, 0, cnt, version ? version : "40");

	if (cd.m_owner == from && as_server_for_conference != src_server && server_type == ST_AS)
	{
		VS_Container cnt;
		cnt.AddValue(METHOD_PARAM, UPDATE_PEERCFG_METHOD);
		cnt.AddValue(CALLID_PARAM, from);
		cnt.AddValue(SERVER_PARAM, as_server_for_conference);
		PostRequest(OurEndpoint(), 0, cnt, AUTH_SRV, LOCATE_SRV);
	}
}

void VS_BaseConferenceService::InvitationUpdate_Method(VS_Container &cnt)
{
	ScheduleInvitaion(cnt.GetStrValueRef(CONFERENCE_PARAM));
}

bool VS_BaseConferenceService::Timer(unsigned long tickcount)
{
	if (tickcount - m_invitation_tick > INVITATION_PERIOD) {
		m_invitation_tick = tickcount;

		VS_AutoLock lock(&m_confs_lock);
		VS_Map::Iterator it = m_confs.Begin();
		while (it != m_confs.End())
		{
			CheckConfInviteTime(*((ConferenceInvitation*)it->data));
			++it;
		}
	}

	auto now = std::chrono::steady_clock::now();
	if (now - m_last_as_check > std::chrono::minutes(5))
	{
		VS_SCOPE_EXIT{ m_last_as_check = now; };
		m_strand.post([this, self = shared_from_this(), as = m_AS_online] () mutable {
			for (auto& p : as)
			{
				VS_LocatorCheck check(p.first.c_str());
				check.Run(2000);
				p.second = check.m_res;
			}
			{
				auto ds = dstream4;
				ds << "checked " << as.size() << " AS:\n";
				for (auto const& p : as)
					ds << p.first << ", ping=" << std::to_string(p.second) << "ms\n";
			}
			CallInProcessingThread([this, as]() {
				for (auto const& p : as)
				{
					auto it = m_AS_online.find(p.first);
					if (it != m_AS_online.end())
						it->second = p.second;
				}
			});
		});
	}
	return true;
}

void VS_BaseConferenceService::StartInviteParticipants(ConferenceInvitation& cd)
{
	VS_PoolThreadsTask* task = new InviteParticipants_Task(cd);
	if (task->IsValid())
		PutTask(task, "InviteParticipants", 30);
	else
		delete task;
}

void VS_BaseConferenceService::CheckConfInviteTime( ConferenceInvitation& cd )
{
	time_t curt;
	time(&curt);
	tm curt_tm;
	localtime_r(&curt, &curt_tm);
	long time_now = curt_tm.tm_hour * 60 + curt_tm.tm_min;
	long wday = 1 << curt_tm.tm_wday;

	auto now = std::chrono::system_clock::now();
	auto interval = cd.m_invited_time;	interval += std::chrono::minutes(1);		// interval = 1 min
	bool is_done = (now >= cd.m_invited_time) && (now < interval);

	bool is_day = false;
	bool is_time = false;

	if (cd.IsOneDayInvitation()) {
		interval = cd.m_invitation_start_time;	interval += std::chrono::minutes(1);
		is_day = is_time = ((now >= cd.m_invitation_start_time) && (now < interval));
	} else {
		is_day = (wday & cd.m_invitation_day) > 0;
		is_time = (time_now == cd.m_invitation_time);
	}

	if (is_day && is_time && !is_done)
	{
		dprint3("INV: Try Send Invites for conf: %s\n", cd.m_conf_id.c_str());
		cd.m_invited_time = std::chrono::system_clock::now();
		StartInviteParticipants(cd);
	}

	bool process_emails = (cd.m_email_minutes>0);
	if (cd.IsInvitaion())
		process_emails = (cd.m_email_minutes < cd.m_invitation_time);

	if (process_emails)
	{
		interval = cd.m_email_sent_time;	interval += std::chrono::minutes(1);		// interval = 1 min
		is_done = (now >= cd.m_email_sent_time) && (now < interval);
		if (cd.IsOneDayInvitation()) {
			auto email_start_time = cd.m_invitation_start_time;
			email_start_time += -(std::chrono::minutes(cd.m_email_minutes));		// minus N minutes
			interval = email_start_time;	interval += std::chrono::minutes(1);
			is_day = is_time = ((now >= email_start_time) && (now < interval));
		} else {
			is_time = (time_now == (cd.m_invitation_time - cd.m_email_minutes));
		}

		if (is_day && is_time && !is_done)
		{
			dprint3("INV: Try send e-mails for conf: %s (%ld minutes to start)\n", cd.m_conf_id.c_str(), cd.m_email_minutes);
			cd.m_email_sent_time = std::chrono::system_clock::now();
			StartSendEmail(cd);
		}
	}
}

void VS_BaseConferenceService::StartSendEmail( ConferenceInvitation& cd )
{
	InvPartsMap::iterator it = cd.m_invitation_parts.begin();
	while (it != cd.m_invitation_parts.end())
	{
		VS_SimpleStr user = it->first.GetID();
		if (!!user)
		{
			VS_Container cnt;
			cnt.AddValue(METHOD_PARAM, SENDMAIL_METHOD);
			cnt.AddValue(CALLID_PARAM, "admin@pca.ru");
			cnt.AddValue(DISPLAYNAME_PARAM, "Administrator");
			cnt.AddValue(CALLID2_PARAM, user);
			cnt.AddValue("app_name", "BS");
			cnt.AddValue(TIME_PARAM, std::chrono::system_clock::now());
			cnt.AddValue(CONFERENCE_PARAM, cd.m_conf_id); // CID
			cnt.AddValue(NAME_PARAM, cd.m_conf_id); // CID
			cnt.AddValue(TOPIC_PARAM, cd.m_topic);
			cnt.AddValue(PASSWORD_PARAM, cd.m_password);

			VS_TransportRouterServiceHelper::PostRequest(OurEndpoint(), 0, cnt, 0, LOG_SRV);
		}
		++it;
	}
}

void VS_BaseConferenceService::ScheduleInvitaion(const char* conf_id)
{
	if (!conf_id)
		return ;

	ConferenceInvitation inv;

	VS_ConferenceDescription cd;
	VS_SimpleStr as_server;
	long scope = 0;
	auto dbStorage = g_dbStorage;
	if(!dbStorage)
		return;
	dbStorage->GetNamedConfInfo(conf_id, cd, inv, as_server, scope);
	if (cd.m_name.IsEmpty())
		return;

	auto error = dbStorage->GetNamedConfParticipants(conf_id, inv);
	if (error || inv.m_invitation_parts.size()<=0)
		return;

	if (!inv.IsInvitaion() && !inv.IsOneDayInvitation())
		return;

	VS_AutoLock lock(&m_confs_lock);
	VS_Map::Iterator it = m_confs.Find(conf_id);
	if (it!=m_confs.End())
		*((ConferenceInvitation*)it->data) = inv;
	else
		m_confs.Insert(conf_id, &inv);
}

void VS_BaseConferenceService::ConferenceCreated_Method(const char* named_conf_id, const char* stream_conf_id)
{
	if (!named_conf_id || !*named_conf_id || !stream_conf_id || !*stream_conf_id)
		return ;
	auto dbStorage = g_dbStorage;
	if(!dbStorage)
		return;
	dbStorage->SetNamedConfServer(named_conf_id, stream_conf_id);
}

void VS_BaseConferenceService::GetASOfMyDomain_Method(VS_Container& cnt)
{
	dstream4 << "GetASOfMyDomain_Method: " << cnt;
	cnt.Reset();
	while (cnt.Next())
	{
		if (strcmp(cnt.GetName(), SERVER_PARAM) == 0 && cnt.GetStrValueRef() != nullptr)
			m_AS_my_domain.emplace(cnt.GetStrValueRef());
	}
}

bool VS_BaseConferenceService::OnPointConnected_Event(const VS_PointParams* prm)
{
	if (prm->reazon <= 0) {
		dprint1("Error {%d} while Connect to (%s)\n", prm->reazon, prm->uid);
	}
	else if (prm->type == VS_PointParams::PT_SERVER) {
		dprint2("Server Connect: uid=%s, reason: %2d\n", prm->uid, prm->reazon);
		if (prm->uid && VS_GetServerType(prm->uid) == ST_AS)
			CallInProcessingThread([this, as = std::string(prm->uid)](){
				m_AS_online.emplace(as, -1);
				m_last_as_check = {};
			});
	}
	else {
		dprint1("NOT SERVER Connect!: uid=%s, reason: %2d\n", prm->uid, prm->reazon);
	}
	return true;
}

bool VS_BaseConferenceService::OnPointDisconnected_Event(const VS_PointParams* prm)
{
	if (prm->type == VS_PointParams::PT_SERVER) {
		dprint2("Server DisConnect: uid=%s, reason: %2d\n", prm->uid, prm->reazon);
		if (prm->uid && VS_GetServerType(prm->uid) == ST_AS)
			CallInProcessingThread([this, as = std::string(prm->uid)]() {
				m_AS_online.erase(as);
			});
	}
	return true;
}

#endif