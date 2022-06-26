#include "ConfRestrictEmpty.h"
namespace tc3_test
{
long ConfRestrictEmpty::CheckInsertMultiConf(long tarif_opt, VS_ConferenceDescription &cd, const char *host, bool FromBS )
{
	return 0;
}
void ConfRestrictEmpty::VCS_SetUserStatus(const VS_SimpleStr& call_id, int status, const VS_ExtendedStatusStorage &extStatus, bool set_server, const VS_SimpleStr& server)
{}

bool ConfRestrictEmpty::FindUser(const vs_user_id&, VS_StorageUserData&) const
{
	return false;
}

int ConfRestrictEmpty::FindMultiConference(const char* name, VS_ConferenceDescription& conf, VS_Container& cnt, const vs_user_id& from_user, bool FromBS)
{
	return VSS_CONF_NOT_FOUND;
}
bool ConfRestrictEmpty::UpdateMultiConference(VS_ConferenceDescription &cd, bool curr)
{
	return false;
}
bool ConfRestrictEmpty::UpdateConference_RTSPAnnounce(VS_ConferenceDescription& cd, string_view announce_id)
{
	return false;
}
unsigned ConfRestrictEmpty::UpdateConfDuration(VS_ConferenceDescription& conf)
{
	return 0;

}
bool ConfRestrictEmpty::GetSSL(VS_SimpleStr& key)
{
	return false;
}
VS_SimpleStr ConfRestrictEmpty::GetAnyBSbyDomain(string_view call_id)
{
	return VS_SimpleStr();
}

// Tarif Restrictions (for AS&BS 3.1 only)
bool ConfRestrictEmpty::Tarif_CreateConf(VS_Container& cnt, VS_ConferenceDescription& cd, VS_UserData& ud, VS_TransportRouterServiceReplyHelper* caller)
{
	return false;
}
bool ConfRestrictEmpty::OnJoinConf(VS_Container& cnt, VS_ConferenceDescription& cd, vs_user_id user_id, bool FromBS, VS_TransportRouterServiceReplyHelper* caller)
{
	return false;
}
void ConfRestrictEmpty::OnRemovePartEvent(const VS_ParticipantDescription& pd, VS_ConferenceDescription& cd, VS_TransportRouterServiceReplyHelper* caller)
{
}
bool ConfRestrictEmpty::CheckInviteMulti_Roaming(vs_user_id user_id)
{
	return false;
}

void ConfRestrictEmpty::GetFirstBS(const char* dst_user, const char* our_endpoint, VS_SimpleStr &server)
{}
void ConfRestrictEmpty::SetOfflineChatMessage(VS_Container &cnt)
{}
void ConfRestrictEmpty::DeleteOfflineChatMessage(VS_Container& cnt)
{}
void ConfRestrictEmpty::GetRoamingOfflineMessages(const char* our_sid, VS_ChatMsgs& v)
{}
bool ConfRestrictEmpty::DoWriteConference(const VS_ConferenceDescription& cd)
{
	return false;
}
bool ConfRestrictEmpty::DoBroadcastConference(const VS_ConferenceDescription& cd)
{
	return false;
}

bool ConfRestrictEmpty::LogSlideShow(VS_Container &cnt)
{
	return false;
}
bool ConfRestrictEmpty::LogGroupChat(VS_Container &cnt)
{
	return false;
}

bool ConfRestrictEmpty::IsVCS() const
{
	return m_isVCS;
}
VS_SimpleStr ConfRestrictEmpty::GetLocalMultiConfID(const char* conf_id) const
{
	return VS_SimpleStr();
}
boost::signals2::connection ConfRestrictEmpty::Connect_AliasesChanged(const AliasesChangedSlot &slot)
{
	return boost::signals2::connection();
}
void ConfRestrictEmpty::UpdateAliasList()
{}

void ConfRestrictEmpty::SetRoamingSettings(const eRoamingMode_t mode, const std::string& params) {}

vs_conf_id ConfRestrictEmpty::NewConfID(VS_SimpleStr _serverID)
{
	static uint32_t m_lastConferenceName(0);
	m_lastConferenceName++;
	vs_conf_id newConf;
	newConf.Resize(_serverID.Length() + 10);
	sprintf(newConf.m_str, "%08x@%s", m_lastConferenceName, _serverID.m_str);
	return newConf;
}
}