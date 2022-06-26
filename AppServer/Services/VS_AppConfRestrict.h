#pragma once
#include "ServerServices/VS_ConfRestrictInterface.h"
#include "std/CallLog/VS_ParticipantDescription.h"

class VS_AppConfRestrict : public VS_ConfRestrictInterface
{
	VS_SimpleStr m_bs_com;
	VS_SimpleStr m_bs_ru;
	uint32_t     m_lastConferenceName;
public:
	VS_AppConfRestrict();
	virtual ~VS_AppConfRestrict() {}

	virtual long CheckInsertMultiConf(long tarif_opt, VS_ConferenceDescription &cd, const char *host, bool FromBS=false) override;
	virtual void VCS_SetUserStatus(const VS_SimpleStr& call_id, int status, const VS_ExtendedStatusStorage &extStatus, bool set_server = false, const VS_SimpleStr& server = VS_SimpleStr()) override;
	bool FindUser(const vs_user_id&, VS_StorageUserData&) const override;
	virtual int FindMultiConference(const char* name, VS_ConferenceDescription& conf, VS_Container& cnt, const vs_user_id& from_user, bool FromBS = false) override;
	virtual bool UpdateMultiConference(VS_ConferenceDescription &cd, bool curr) override;
	virtual bool UpdateConference_RTSPAnnounce(VS_ConferenceDescription& cd, string_view announce_id) override;
	virtual unsigned UpdateConfDuration(VS_ConferenceDescription& conf) override;
	virtual bool GetSSL(VS_SimpleStr& key) override;
	VS_SimpleStr GetAnyBSbyDomain(string_view call_id) override;

	// Tarif
	virtual bool Tarif_CreateConf(VS_Container& cnt, VS_ConferenceDescription& cd, VS_UserData& ud, VS_TransportRouterServiceReplyHelper* caller = 0) override;
	virtual bool OnJoinConf(VS_Container& cnt, VS_ConferenceDescription& cd, vs_user_id user_id, bool FromBS = false, VS_TransportRouterServiceReplyHelper* caller = 0) override;
	virtual void OnRemovePartEvent(const VS_ParticipantDescription& pd, VS_ConferenceDescription& cd, VS_TransportRouterServiceReplyHelper* caller) override;
	virtual bool CheckInviteMulti_Roaming(vs_user_id user_id) override;

	void GetFirstBS(const char* dst_user, const char* our_endpoint, VS_SimpleStr &server) override;
	void SetOfflineChatMessage(VS_Container &cnt) override;
	void DeleteOfflineChatMessage(VS_Container& cnt) override;
	void GetRoamingOfflineMessages(const char* our_sid, VS_ChatMsgs& v) override;
	bool DoWriteConference(const VS_ConferenceDescription &cd) override;
	bool DoBroadcastConference(const VS_ConferenceDescription& cd) override;

	virtual bool LogSlideShow(VS_Container &cnt) override;
	virtual bool LogGroupChat(VS_Container &cnt) override;

	/////VS_SimpleStr GetUplink(const char* call_id, const char* uplink, bool uplinkOn);
	bool IsVCS() const override;
	virtual bool GetAppProp(const char* prop_name, VS_SimpleStr& value) override;
	VS_SimpleStr GetLocalMultiConfID(const char* conf_id) const override;

	virtual boost::signals2::connection Connect_AliasesChanged(const AliasesChangedSlot &slot) override;
	virtual void UpdateAliasList() override;

	virtual void SetRoamingSettings(const eRoamingMode_t mode, const std::string& params) override;

	vs_conf_id NewConfID(VS_SimpleStr) override;
};
