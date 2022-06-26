#pragma once
#include "ServerServices/VS_ConfRestrictInterface.h"

extern const char MMC_PASS_TAG[];
extern const char MMC_OWNER_TAG[];
extern const char MMC_TYPE_TAG[];
extern const char MMC_SUBTYPE_TAG[];
extern const char MMC_TOPIC_TAG[];
extern const char MMC_CONFID_TAG[];
extern const char MMC_CASTER_TAG[];
extern const char MMC_PODIUMS_TAG[];
extern const char MMC_MAXPARTS_TAG[];
extern const char MMC_INVITEDTIME[];
extern const char MMC_INVITATION_DATE[];
extern const char MMC_INVITATION_DAY[];
extern const char MMC_INVITATION_TIME[];
extern const char MMC_INVITATION_TYPE[];
extern const char MMC_AUTO_INVITE[];
extern const char MMC_MULTICAST_IP_TAG[];
extern const char MMC_BROADCAST_TAG[];
extern const char MMC_CONFERENCE_RECORDING_TAG[];
extern const char MMC_CODECS_TAG[];
extern const char MMC_RTSP_ANNOUNCE_KEY[];
extern const char MMC_RTSP_ANNOUNCE_URL_TAG[];
extern const char MMC_RTSP_ANNOUNCE_USERNAME_TAG[];
extern const char MMC_RTSP_ANNOUNCE_PASSWORD_TAG[];
extern const char MMC_RTSP_ANNOUNCE_RTP_OVER_TCP_TAG[];
extern const char MMC_RTSP_ANNOUNCE_KEEPALIVE_TIMEOUT_TAG[];
extern const char MMC_RTSP_ANNOUNCE_RETRIES_TAG[];
extern const char MMC_RTSP_ANNOUNCE_RETRY_DELAY_TAG[];
extern const char MMC_RTSP_ANNOUNCE_CODECS_TAG[];
extern const char MMC_RTSP_ANNOUNCE_ACTIVE_TAG[];
extern const char MMC_RTSP_ANNOUNCE_REASON_TAG[];
extern const char MMC_RTSP_HELPER_PROGRAM_TAG[];
extern const char MMC_WEBPARAMS_KEY[];
extern const char MMC_WEBPARAMS_TYPE[];
extern const char MMC_CLIENTRIGHTS_KEY[];
extern const char MMC_CLIENTRIGHTS_GUEST[];
extern const char MMC_CLIENTRIGHTS_USER[];
extern const char MMC_DURATION_TAG[];
extern const char MMC_PLANNEDPARTICIPANTSONLY_TAG[];

class VS_RoamingSettings;

class VS_VCSConfRestrict : public VS_ConfRestrictInterface
{
	int InsertSpecialConference(const char* name, const vs_user_id& from_user, const char* topic, bool is_public);
public:
	VS_VCSConfRestrict();
	virtual ~VS_VCSConfRestrict(){}

	virtual long CheckInsertMultiConf(long tarif_opt, VS_ConferenceDescription &cd, const char *host, bool FromBS = false) override;
	virtual void VCS_SetUserStatus(const VS_SimpleStr& call_id, int status, const VS_ExtendedStatusStorage &extStatus, bool set_server = false, const VS_SimpleStr& server = VS_SimpleStr()) override;
	bool FindUser(const vs_user_id& /*id*/, VS_StorageUserData& /*user*/) const override;
	int FindMultiConference(const char* name, VS_ConferenceDescription& conf, VS_Container& cnt, const vs_user_id& from_user, bool FromBS = false) override;
	bool UpdateMultiConference(VS_ConferenceDescription &cd, bool curr) override;
	bool UpdateConference_RTSPAnnounce(VS_ConferenceDescription& cd, string_view announce_id) override;
	unsigned UpdateConfDuration(VS_ConferenceDescription& conf) override;
	bool IsPlannedParticipant(VS_ConferenceDescription &cd, const char *user) override;
	bool GetSSL(VS_SimpleStr& key) override;
	VS_SimpleStr GetAnyBSbyDomain(string_view call_id) override;

	// Tarif
	bool Tarif_CreateConf(VS_Container& cnt, VS_ConferenceDescription& cd, VS_UserData& ud, VS_TransportRouterServiceReplyHelper* caller = 0) override;
	bool OnJoinConf(VS_Container& cnt, VS_ConferenceDescription& cd, vs_user_id user_id, bool FromBS = false, VS_TransportRouterServiceReplyHelper* caller = 0) override;
	void OnRemovePartEvent(const VS_ParticipantDescription& pd, VS_ConferenceDescription& cd, VS_TransportRouterServiceReplyHelper* caller) override;
	bool CheckInviteMulti_Roaming(vs_user_id user_id) override;

	void GetFirstBS(const char* dst_user, const char* our_endpoint, VS_SimpleStr &server) override;
	void SetOfflineChatMessage(VS_Container &cnt) override;
	void DeleteOfflineChatMessage(VS_Container& cnt) override;
	void GetRoamingOfflineMessages(const char* our_sid, VS_ChatMsgs& v) override;

	virtual bool DoWriteConference(const VS_ConferenceDescription& cd) override;
	virtual bool DoBroadcastConference(const VS_ConferenceDescription& cd) override;

	virtual bool GetRecordState(const VS_ConferenceDescription& cd, VS_ConfRecordingState &state) override;
	virtual bool SetRecordState(const VS_ConferenceDescription& cd, VS_ConfRecordingState state) override;

	std::string ReadLayout(string_view cid, layout_for f, string_view part) override;
	bool UpdateLayout(string_view cid, string_view new_layout, layout_for f, string_view part) override;
	std::vector<std::string> GetUsersWithIndividualLayouts(string_view cid) override;

	virtual bool LogSlideShow(VS_Container &cnt) override;
	virtual bool LogGroupChat(VS_Container &cnt) override;
	virtual void LogRecordStart(const vs_conf_id& conf_id, const std::string& filename, std::chrono::system_clock::time_point started_at, VS_TransportRouterServiceReplyHelper* caller) override;
	virtual void LogRecordStop(const vs_conf_id& conf_id, std::chrono::system_clock::time_point stopped_at, uint64_t file_size, VS_TransportRouterServiceReplyHelper* caller) override;

	bool IsVCS() const override;
	virtual VS_UserPresence_Status CheckOfflineStatus(const VS_SimpleStr& call_id, VS_ExtendedStatusStorage & extStatus) override;
	virtual bool IsRoamingAllowed(const char *for_server_name) override;
	virtual bool GetAppProp(const char* prop_name, VS_SimpleStr& value) override;
	VS_SimpleStr GetLocalMultiConfID(const char* conf_id) const override;
	bool CheckSessionID(const char *pass) const override;
	virtual boost::signals2::connection Connect_AliasesChanged(const AliasesChangedSlot &slot) override;
	virtual void UpdateAliasList() override;

	virtual void SetRoamingSettings(const eRoamingMode_t mode, const std::string& params) override;
	virtual bool IsOperator(const vs_user_id& user) override ;

	CMRFlags GetCMRFlagsByLicense() override;

	void SetRoamingSettings(std::shared_ptr<VS_RoamingSettings> rs)
	{
		m_roaming_settings = std::move(rs);
	}

	vs_conf_id NewConfID(VS_SimpleStr) override;

private:
	std::shared_ptr<VS_RoamingSettings> m_roaming_settings;
};
