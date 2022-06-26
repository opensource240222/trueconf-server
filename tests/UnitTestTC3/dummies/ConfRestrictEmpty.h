#pragma once

#include "ServerServices/VS_ConfRestrictInterface.h"
#include "std/CallLog/VS_ParticipantDescription.h"

namespace tc3_test
{
class ConfRestrictEmpty : public VS_ConfRestrictInterface
{
	long CheckInsertMultiConf(long tarif_opt, VS_ConferenceDescription &cd, const char *host, bool FromBS = false) override;
	void VCS_SetUserStatus(const VS_SimpleStr& call_id, int status, const VS_ExtendedStatusStorage &extStatus, bool set_server = false, const VS_SimpleStr& server = VS_SimpleStr()) override;
	bool FindUser(const vs_user_id&, VS_StorageUserData&) const override;
	int FindMultiConference(const char* name, VS_ConferenceDescription& conf, VS_Container& cnt, const vs_user_id& from_user, bool FromBS = false) override;
	bool UpdateMultiConference(VS_ConferenceDescription &cd, bool curr) override;
	bool UpdateConference_RTSPAnnounce(VS_ConferenceDescription& cd, string_view announce_id) override;
	unsigned UpdateConfDuration(VS_ConferenceDescription& conf) override;
	bool GetSSL(VS_SimpleStr& key) override;
	VS_SimpleStr GetAnyBSbyDomain(string_view call_id) override;

	// Tarif Restrictions (for AS&BS 3.1 only)
	bool Tarif_CreateConf(VS_Container& cnt, VS_ConferenceDescription& cd, VS_UserData& ud, VS_TransportRouterServiceReplyHelper* caller = 0) override;
	bool OnJoinConf(VS_Container& cnt, VS_ConferenceDescription& cd, vs_user_id user_id, bool FromBS = false, VS_TransportRouterServiceReplyHelper* caller = 0) override;
	void OnRemovePartEvent(const VS_ParticipantDescription& pd, VS_ConferenceDescription& cd, VS_TransportRouterServiceReplyHelper* caller) override;
	bool CheckInviteMulti_Roaming(vs_user_id user_id) override;

	void GetFirstBS(const char* dst_user, const char* our_endpoint, VS_SimpleStr &server) override;
	void SetOfflineChatMessage(VS_Container &cnt) override;
	void DeleteOfflineChatMessage(VS_Container& cnt) override;
	void GetRoamingOfflineMessages(const char* our_sid, VS_ChatMsgs& v) override;
	bool DoWriteConference(const VS_ConferenceDescription& cd) override;
	bool DoBroadcastConference(const VS_ConferenceDescription& cd) override;

	bool LogSlideShow(VS_Container &cnt) override;
	bool LogGroupChat(VS_Container &cnt) override;

	bool IsVCS() const override;
	VS_SimpleStr GetLocalMultiConfID(const char* conf_id) const override;
	boost::signals2::connection Connect_AliasesChanged(const AliasesChangedSlot &slot) override;
	void UpdateAliasList() override;

	void SetRoamingSettings(const eRoamingMode_t mode, const std::string& params) override;

	vs_conf_id NewConfID(VS_SimpleStr) override;

	bool m_isVCS;
public:
	ConfRestrictEmpty(bool isVCS)
		: m_isVCS(isVCS)
	{}
};
}
