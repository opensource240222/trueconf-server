#pragma once

#include "transport/Router/VS_TransportRouterServiceHelper.h"
#include "ServerServices/Common.h"
#include "std/cpplib/VS_Lock.h"

#include "std-generic/cpplib/StrCompare.h"
#include "std-generic/compat/map.h"
#include "std-generic/compat/set.h"
#include <string>
#include <map>

class VS_ConfLog : public virtual VS_TransportRouterServiceHelper

{
private:
	static vs::fast_recursive_mutex	m_lock;
	using storage_t = vs::map<std::string/*conf_id*/, vs::map<std::string/*trueconf_id*/, std::string/*server_name*/, vs::str_less>, vs::str_less>; // server_name = BS | AS
	static storage_t s_subBS;

	void LogConferenceStartToBS(const VS_ConferenceDescription& cd, string_view bs);
	void LogConferenceEndToBS(const VS_ConferenceDescription& cd, string_view bs);
	void LogParticipantInviteToBS(const vs_conf_id& conf_id, string_view bs, const VS_SimpleStr& call_id1, const VS_SimpleStr& app_id, const VS_SimpleStr& call_id2,
		const std::chrono::system_clock::time_point time, VS_ParticipantDescription::Type type=VS_ParticipantDescription::PRIVATE_HOST);
	void LogParticipantJoinToBS(const VS_ParticipantDescription& pd, string_view bs, const VS_SimpleStr& call_id2=NULL);
	void LogParticipantRejectToBS(const vs_conf_id& conf_id, const vs_user_id& user, const vs_user_id& invited_from, const VS_Reject_Cause cause, string_view bs);
	void LogParticipantLeaveToBS(const VS_ParticipantDescription& pd, string_view bs);

	static bool is_p2p_conf(const vs_conf_id& stream_id);
public:
	VS_ConfLog(){}
	virtual ~VS_ConfLog(){}
	void SubscribeBS(const vs_conf_id& conf_id, const char *bs, const char* user = 0);

	virtual void LogConferenceStart(const VS_ConferenceDescription& cd);
	virtual void LogConferenceEnd(const VS_ConferenceDescription& cd);
	virtual void LogParticipantInvite(const vs_conf_id& conf_id, const VS_SimpleStr& call_id1, const VS_SimpleStr& app_id, const VS_SimpleStr& call_id2,
		const std::chrono::system_clock::time_point time, VS_ParticipantDescription::Type type=VS_ParticipantDescription::PRIVATE_HOST, const char* other_server = 0);
	virtual void LogParticipantJoin(const VS_ParticipantDescription& pd, const VS_SimpleStr& call_id2=NULL, const char* bs_user = 0, const char* bs_named_conf = 0);
	virtual void LogParticipantReject(const vs_conf_id conf_id, const vs_user_id user, const vs_user_id& invited_from, const VS_Reject_Cause cause);
	virtual void LogParticipantLeave(const VS_ParticipantDescription& pd);

	void LogPostMess(VS_Container &cnt, string_view bs);
};