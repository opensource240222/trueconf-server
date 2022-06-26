#include "VS_ConfLog.h"
#include "AppServer/Services/VS_Storage.h"
#include "std/debuglog/VS_Debug.h"

#define DEBUG_CURRENT_MODULE VS_DM_LOGSERVICE

vs::fast_recursive_mutex VS_ConfLog::m_lock;
VS_ConfLog::storage_t VS_ConfLog::s_subBS;

void VS_ConfLog::SubscribeBS(const vs_conf_id& conf_id, const char *bs, const char* user)
{
	if (!conf_id || !bs || !*bs)
		return ;
	std::lock_guard<decltype(m_lock)> _(m_lock);
	auto it = s_subBS.find(string_view(conf_id.m_str));
	if (it == s_subBS.end())
		it = s_subBS.emplace(std::piecewise_construct, std::forward_as_tuple(conf_id.m_str), std::forward_as_tuple()).first;
	if (user && *user && it->second.count(string_view(user)) == 0)
	{
		VS_ConferenceDescription	cd;
		if(!g_storage->FindConference(SimpleStrToStringView(conf_id), cd))
		{
			s_subBS.erase(it);
			return;
		}
		bool exist_before = !strcasecmp(bs, OurEndpoint());
		if (!exist_before)
			for (auto const& p : it->second)
				if (p.second == bs)
					exist_before = true;
		dprint3("add user:%s bs:%s to bs map\n", user, bs);
		it->second[user] = bs;
		if (!exist_before)
			LogConferenceStartToBS(cd,bs);
	}
}
void VS_ConfLog::LogConferenceStart(const VS_ConferenceDescription &cd)
{
	dprint3("LogConfStart(%s)\n", cd.m_name.m_str);
	std::lock_guard<decltype(m_lock)> _(m_lock);
	VS_UserData	ud;
	if (g_storage->FindUser(SimpleStrToStringView(cd.m_owner), ud))
		SubscribeBS(cd.m_name, ud.m_homeServer, cd.m_owner);
	std::set<string_view> servers{ OurEndpoint() };
	auto it = s_subBS.find(string_view(cd.m_name.m_str));
	if (it == s_subBS.end())
		it = s_subBS.emplace(std::piecewise_construct, std::forward_as_tuple(cd.m_name.m_str), std::forward_as_tuple()).first;
	if (it != s_subBS.end())
		for (auto const& p : it->second)
			servers.emplace(p.second);
	for (auto const& server : servers)
		LogConferenceStartToBS(cd, server);
}

void VS_ConfLog::LogParticipantInvite(const vs_conf_id &conf_id, const VS_SimpleStr &call_id1, const VS_SimpleStr &app_id, const VS_SimpleStr &call_id2, const std::chrono::system_clock::time_point time, VS_ParticipantDescription::Type type, const char* other_server)
{
	dprint3("LogParticipantInvite(%s->%s, %s)\n", call_id1.m_str, call_id2.m_str, conf_id.m_str);
	std::lock_guard<decltype(m_lock)> _(m_lock);
/**
подписать BS'ы пользователей call_id1 и call_id2
*/
	VS_UserData ud1, ud2;
	if(g_storage->FindUser(SimpleStrToStringView(call_id1), ud1))
		SubscribeBS(conf_id, ud1.m_homeServer, call_id1);

	if(g_storage->FindUser(SimpleStrToStringView(call_id2), ud2))
		SubscribeBS(conf_id, ud2.m_homeServer, call_id2);
	else if (other_server && *other_server)
		SubscribeBS(conf_id, other_server, call_id2);
	auto it = s_subBS.find(string_view(conf_id.m_str));
	if (it != s_subBS.end())
	{
		std::set<string_view> servers{ OurEndpoint() };
		if (!!call_id1)
		{
			auto server = it->second.find(string_view(call_id1.m_str));
			if (server != it->second.end())
				servers.emplace(server->second);
		}
		if (!!call_id2)
		{
			auto server = it->second.find(string_view(call_id2.m_str));
			if (server != it->second.end())
				servers.emplace(server->second);
		}

		for(auto const& server: servers)
			LogParticipantInviteToBS(conf_id, server, call_id1, app_id, call_id2, time, type);
	}
}

void VS_ConfLog::LogParticipantJoin(const VS_ParticipantDescription &pd, const VS_SimpleStr &call_id2, const char* bs_user, const char* bs_named_conf)
{
	dprint3("LogParticipantJoin(%s:%s, %s)\n", pd.m_user_id.m_str, pd.m_server_id.m_str, pd.m_conf_id.m_str);
	std::lock_guard<decltype(m_lock)> _(m_lock);
	VS_UserData	ud;
	if(g_storage->FindUser(SimpleStrToStringView(pd.m_user_id), ud))
		SubscribeBS(pd.m_conf_id, ud.m_homeServer, pd.m_user_id);
	if (bs_user && *bs_user)
		SubscribeBS(pd.m_conf_id, bs_user, pd.m_user_id);
	if (bs_named_conf && *bs_named_conf)
		SubscribeBS(pd.m_conf_id, bs_named_conf, bs_named_conf);

	auto it = s_subBS.find(string_view(pd.m_conf_id.m_str));
	if (it != s_subBS.end())
	{
		std::set<string_view> servers{ OurEndpoint() };
		if (is_p2p_conf(pd.m_conf_id)) {
			for (auto const& p : it->second)
				servers.emplace(p.second);
		} else {
			if (!!pd.m_user_id)
			{
				auto server = it->second.find(string_view(pd.m_user_id.m_str));
				if (server != it->second.end())
					servers.emplace(server->second);
			}
			if (!!call_id2)
			{
				auto server = it->second.find(string_view(call_id2.m_str));
				if (server != it->second.end())
					servers.emplace(server->second);
			}
		}

		for (auto const& server : servers)
			LogParticipantJoinToBS(pd, server);
	}
}

void VS_ConfLog::LogParticipantReject(const vs_conf_id conf_id, const vs_user_id user, const vs_user_id& invited_from, const VS_Reject_Cause cause)
{
	dprint3("LogParticipantReject(%s, %s, cause=%d)\n", conf_id.m_str, user.m_str, cause);
	std::lock_guard<decltype(m_lock)> _(m_lock);
	auto it = s_subBS.find(string_view(conf_id.m_str));
	if (it != s_subBS.end())
	{
		std::set<string_view> servers{ OurEndpoint() };
		if (!!invited_from)
		{
			auto server = it->second.find(string_view(invited_from.m_str));
			if (server != it->second.end())
				servers.emplace(server->second);
		}
		if (!!user)
		{
			auto server = it->second.find(string_view(user.m_str));
			if (server != it->second.end())
				servers.emplace(server->second);
		}

		for (auto const& server : servers)
			LogParticipantRejectToBS(conf_id, user, invited_from, cause, server);
	}
}

void VS_ConfLog::LogConferenceEnd(const VS_ConferenceDescription& cd)
{
	dprint3("LogConferenceEnd(%s)\n", cd.m_name.m_str);
	std::lock_guard<decltype(m_lock)> _(m_lock);
	auto it = s_subBS.find(string_view(cd.m_name.m_str));
	if (it != s_subBS.end())
	{
		std::set<string_view> servers{ OurEndpoint() };
		for (auto const& p : it->second)
			servers.emplace(p.second);

		for (auto const& server : servers)
			LogConferenceEndToBS(cd, server);
		s_subBS.erase(it);
	}
}

void VS_ConfLog::LogParticipantLeave(const VS_ParticipantDescription& pd)
{
	dprint3("LogParticipantLeave(%s:%s, %s)\n", pd.m_user_id.m_str, pd.m_server_id.m_str, pd.m_conf_id.m_str);
	std::lock_guard<decltype(m_lock)> _(m_lock);
	auto it = s_subBS.find(string_view(pd.m_conf_id.m_str));
	if (it != s_subBS.end())
	{
		std::set<string_view> servers{ OurEndpoint() };
		if (is_p2p_conf(pd.m_conf_id)) {
			for (auto const& p : it->second)
				servers.emplace(p.second);
		} else {
			if (!!pd.m_user_id)
			{
				auto server = it->second.find(string_view(pd.m_user_id.m_str));
				if (server != it->second.end())
					servers.emplace(server->second);
			}
		}

		for (auto const& server : servers)
				LogParticipantLeaveToBS(pd, server);
	}
}

void VS_ConfLog::LogConferenceStartToBS(const VS_ConferenceDescription &cd, string_view bs)
{
	dstream4 << "Send " << bs << " ConfStart " << cd.m_name.m_str;
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM,LOGCONFSTART_METHOD);
	cnt.AddValue(CONFERENCE_PARAM,(const char*)cd.m_name);
	cnt.AddValue(CALLID_PARAM, (const char*)cd.m_call_id);
	cnt.AddValue(OWNER_PARAM,(const char*)cd.m_owner);
	cnt.AddValue(TIME_PARAM,cd.m_logStarted);
	cnt.AddValueI32(TYPE_PARAM, cd.m_type);
	cnt.AddValueI32(SUBTYPE_PARAM, cd.m_SubType);
	cnt.AddValue(APPID_PARAM,(const char*)cd.m_appID);
	cnt.AddValueI32(MAXPARTISIPANTS_PARAM, cd.m_MaxParticipants);
	cnt.AddValue(PUBLIC_PARAM,cd.m_public);
	cnt.AddValue(TOPIC_PARAM, cd.m_topic);
	cnt.AddValue(LANG_PARAM,(const char*)cd.m_lang);
	LogPostMess(cnt,bs);
}

void VS_ConfLog::LogConferenceEndToBS(const VS_ConferenceDescription& cd, string_view bs)
{
	dstream4 << "Send " << bs << " ConfEnd " << cd.m_name.m_str;
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM,LOGCONFEND_METHOD);
	cnt.AddValue(CONFERENCE_PARAM,(const char*)cd.m_name);
	cnt.AddValueI32(TYPE_PARAM, cd.m_type);
	cnt.AddValueI32(SUBTYPE_PARAM, cd.m_SubType);
	cnt.AddValue(TIME_PARAM,cd.m_logEnded);
	cnt.AddValue(OWNER_PARAM,(const char*)cd.m_owner);
	cnt.AddValueI32(CAUSE_PARAM, cd.m_logCause);
	LogPostMess(cnt,bs);
}

void VS_ConfLog::LogParticipantInviteToBS(const vs_conf_id& conf_id, string_view bs, const VS_SimpleStr& call_id1, const VS_SimpleStr& app_id, const VS_SimpleStr& call_id2,
		const std::chrono::system_clock::time_point time, VS_ParticipantDescription::Type type)
{
	dstream4 << "Send " << bs << " PartInvite " << call_id1.m_str << "->" << call_id2.m_str << " to conf " << conf_id.m_str;
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM,LOGPARTINVITE_METHOD);
	cnt.AddValue(CONFERENCE_PARAM,(const char*)conf_id);
	cnt.AddValue(CALLID_PARAM,(const char*)call_id1);
	cnt.AddValue(CALLID2_PARAM,(const char*)call_id2);
	cnt.AddValue(TIME_PARAM,time);
	cnt.AddValueI32(TYPE_PARAM, type);
	cnt.AddValue(APPID_PARAM,app_id);
	LogPostMess(cnt,bs);
}

void VS_ConfLog::LogParticipantJoinToBS(const VS_ParticipantDescription& pd, string_view bs, const VS_SimpleStr& call_id2)
{
	dstream4 << "Send " << bs << " PartJoin " << pd.m_user_id.m_str << " to conf " << pd.m_conf_id.m_str;
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM,LOGPARTJOIN_METHOD);
	cnt.AddValue(CONFERENCE_PARAM,(const char*)pd.m_conf_id);
	cnt.AddValue(CALLID_PARAM,(const char*)pd.m_user_id);
	cnt.AddValue(SERVER_PARAM,(const char*)pd.m_server_id);
	cnt.AddValueI32(TYPE_PARAM, pd.m_type);

	cnt.AddValueI32(PRICE_PARAM, pd.m_decLimit);
	cnt.AddValue(CHARGE1_PARAM,pd.m_charge1);
	cnt.AddValue(CHARGE2_PARAM,pd.m_charge2);
	cnt.AddValue(CHARGE3_PARAM,pd.m_charge3);
	cnt.AddValue(TIME_PARAM, pd.m_joinTime);
	cnt.AddValue(APPID_PARAM,pd.m_appID);
	if (!pd.m_displayName.empty())
		cnt.AddValue(DISPLAYNAME_PARAM, pd.m_displayName);
	if(call_id2)
		cnt.AddValue(CALLID2_PARAM,(const char*)call_id2);
	LogPostMess(cnt,bs);
}
void VS_ConfLog::LogParticipantRejectToBS(const vs_conf_id& conf_id, const vs_user_id& user, const vs_user_id& invited_from, const VS_Reject_Cause cause, string_view bs)
{
	dstream4 << "Send " << bs << " PartReject " << user.m_str << " from conf " << conf_id.m_str << ", cause=" << cause;
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, LOGPARTREJECT_METHOD);
	cnt.AddValue(CONFERENCE_PARAM, (const char*)conf_id);
	cnt.AddValue(CALLID_PARAM, (const char*)user);
	cnt.AddValue(FROM_PARAM, (const char*)invited_from);
	cnt.AddValueI32(CAUSE_PARAM, cause);
	cnt.AddValue(TIME_PARAM, std::chrono::system_clock::now());
	LogPostMess(cnt, bs);
}
void VS_ConfLog::LogParticipantLeaveToBS(const VS_ParticipantDescription& pd, string_view bs)
{
	dstream4 << "Send " << bs << " PartLeave " << pd.m_user_id.m_str << " from conf" << pd.m_conf_id.m_str;
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM,LOGPARTLEAVE_METHOD);
	cnt.AddValue(CONFERENCE_PARAM,(const char*)pd.m_conf_id);
	cnt.AddValue(CALLID_PARAM,(const char*)pd.m_user_id);
	cnt.AddValueI32(TYPE_PARAM, pd.m_type);
	cnt.AddValue(SERVER_PARAM,(const char*)pd.m_server_id);
	cnt.AddValue(TIME_PARAM, std::chrono::system_clock::now());
	cnt.AddValue(JOIN_TIME_PARAM, pd.m_joinTime);
	cnt.AddValueI32(CAUSE_PARAM, pd.m_cause);

	cnt.AddValueI32(BYTES_SENT_PARAM, pd.m_bytesSnd);
	cnt.AddValueI32(BYTES_RECEIVED_PARAM, pd.m_bytesRcv);
	cnt.AddValueI32(RECON_SND_PARAM, pd.m_reconSnd);
	cnt.AddValueI32(RECON_RCV_PARAM, pd.m_reconRcv);
	LogPostMess(cnt, bs);
}

void VS_ConfLog::LogPostMess(VS_Container &cnt, string_view bs)
{
	PostRequest(((std::string)bs).c_str(), 0, cnt, 0, LOG_SRV, 300000, CONFERENCE_SRV);
}

bool VS_ConfLog::is_p2p_conf(const vs_conf_id& stream_id)
{
	VS_ConferenceDescription cd;
	g_storage->FindConference(SimpleStrToStringView(stream_id), cd);
	return !(cd.m_type == CT_MULTISTREAM || cd.m_type == CT_INTERCOM);
}