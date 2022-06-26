#pragma once

#include "statuslib/status_types.h"
#include "std/CallLog/VS_ConferenceDescription.h"
#include "std/CallLog/VS_ParticipantDescription.h"
#include "std/cpplib/VS_UserData.h"
#include "transport/typedefs.h"
#include "transport/VS_ChatMsg.h"

#include <boost/signals2.hpp>

class VS_TorrentStarterBase;
class VS_Connection;
class VS_TransportRouterServiceReplyHelper;
class VS_ExtendedStatusStorage;

enum VS_ConfRecordingState : int32_t
{
	RS_NO_RECORDING = 0,
	RS_RECORDING = 1,
	RS_PAUSED = 2
};

class VS_ConfRestrictInterface
{
protected:
	std::weak_ptr<VS_TorrentStarterBase> m_torrent_starter;
public:
	virtual ~VS_ConfRestrictInterface() { /*stub*/ }
	virtual long CheckInsertMultiConf(long tarif_opt, VS_ConferenceDescription &cd, const char *host, bool FromBS=false) = 0;
	virtual void VCS_SetUserStatus(const VS_SimpleStr& call_id,int status, const VS_ExtendedStatusStorage &extStatus, bool set_server = false,const VS_SimpleStr& server = VS_SimpleStr()) = 0;
	virtual bool FindUser(const vs_user_id& /*id*/, VS_StorageUserData& /*user*/) const = 0;
	virtual int FindMultiConference(const char* name, VS_ConferenceDescription& conf, VS_Container& cnt, const vs_user_id& from_user, bool FromBS = false)=0;
	virtual bool UpdateMultiConference(VS_ConferenceDescription &cd, bool curr)=0;
	virtual bool UpdateConference_RTSPAnnounce(VS_ConferenceDescription& cd, string_view announce_id = {}) = 0;
	virtual unsigned UpdateConfDuration(VS_ConferenceDescription& conf) = 0;
	virtual bool IsPlannedParticipant(VS_ConferenceDescription& /*cd*/, const char* /*user*/) { return true; }
	virtual bool GetSSL(VS_SimpleStr& key) = 0;
	virtual VS_SimpleStr GetAnyBSbyDomain(string_view call_id) = 0;

	// Tarif Restrictions (for AS&BS 3.1 only)
	virtual bool Tarif_CreateConf(VS_Container& cnt, VS_ConferenceDescription& cd, VS_UserData& ud, VS_TransportRouterServiceReplyHelper* caller = 0) = 0;
	virtual bool OnJoinConf(VS_Container& cnt, VS_ConferenceDescription& cd, vs_user_id user_id, bool FromBS = false, VS_TransportRouterServiceReplyHelper* caller = 0) = 0;
	virtual void OnRemovePartEvent(const VS_ParticipantDescription& pd, VS_ConferenceDescription& cd, VS_TransportRouterServiceReplyHelper* caller) = 0;
	virtual bool CheckInviteMulti_Roaming(vs_user_id user_id) = 0;

	virtual void GetFirstBS(const char* dst_user, const char* our_endpoint, VS_SimpleStr &server) = 0;
	virtual void SetOfflineChatMessage(VS_Container &cnt) = 0;
	virtual void DeleteOfflineChatMessage(VS_Container& cnt) = 0;
	virtual void GetRoamingOfflineMessages(const char* our_sid, VS_ChatMsgs& v) = 0;
	virtual bool DoWriteConference(const VS_ConferenceDescription& cd) = 0;
	virtual bool DoBroadcastConference(const VS_ConferenceDescription& cd) = 0;
	virtual bool GetRecordState(const VS_ConferenceDescription& /*cd*/, VS_ConfRecordingState& /*state*/) { return false; }
	virtual bool SetRecordState(const VS_ConferenceDescription& /*cd*/, VS_ConfRecordingState /*state*/) { return false; }

	virtual std::string ReadLayout(string_view /*cid*/, layout_for /*f*/, string_view /*part*/ = {}) { return {}; }
	virtual bool UpdateLayout(string_view /*cid*/, string_view /*new_layout*/, layout_for /*f*/, string_view /*part*/) { return false; }
	virtual std::vector<std::string> GetUsersWithIndividualLayouts(string_view /*cid*/) { return {}; };

	virtual bool LogSlideShow(VS_Container &cnt) = 0;
	virtual bool LogGroupChat(VS_Container &cnt) = 0;
	virtual void LogRecordStart(const vs_conf_id& /*conf_id*/, const std::string& /*filename*/, std::chrono::system_clock::time_point /*started_at*/, VS_TransportRouterServiceReplyHelper* /*caller*/)
	{}
	virtual void LogRecordStop(const vs_conf_id& /*conf_id*/, std::chrono::system_clock::time_point /*stopped_at*/, uint64_t /*file_size*/, VS_TransportRouterServiceReplyHelper* /*caller*/)
	{}

	virtual bool IsVCS() const = 0;
	virtual VS_UserPresence_Status CheckOfflineStatus ( const VS_SimpleStr& /*call_id*/, VS_ExtendedStatusStorage & /*extStatus*/ )
	{
		return USER_LOGOFF;
	}
	virtual bool IsUserAdmin(const char* /*user_name*/)
	{
		return false;
	}
	virtual bool IsRoamingAllowed(const char* /*for_server_name*/)
	{
		return true;
	}
	virtual bool GetAppProp(const char* /*prop_name*/, VS_SimpleStr& /*value*/)
	{
		return false;
	}
	bool IsSpecialConf(const char* name)
	{
		if (!name || !*name)
			return false;
		return strstr(name, SPECIAL_CONF_PREFIX)==name;
	}

	// for VCS local_conf_id = my_conf (no server name)
	// for AS local_conf_id = my_conf@server.name#as;
	virtual VS_SimpleStr GetLocalMultiConfID(const char* conf_id) const = 0;
	virtual bool CheckSessionID(const char* /*pass*/) const
	{
		return false;
	}

	virtual boost::signals2::connection Connect_AliasesChanged(const AliasesChangedSlot &slot) = 0;
	virtual void UpdateAliasList() = 0;

	virtual void SetRoamingSettings(const eRoamingMode_t mode, const std::string& params) = 0;

	virtual bool IsOperator(const vs_user_id& /*user*/)
	{
		return false;
	}
	virtual CMRFlags GetCMRFlagsByLicense();

	void SetTorrentStarter(const std::weak_ptr<VS_TorrentStarterBase>& torrent_start)
	{
		m_torrent_starter = torrent_start;
	}
	virtual vs_conf_id NewConfID(VS_SimpleStr) = 0;
};
