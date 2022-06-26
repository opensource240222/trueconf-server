/*****************************************************************************
 * (c) 2007 Visicron Systems, Inc.  http://www.visicron.net/
 *
 * Project: Broker Services - Media streaming
 *
 * $Revision: 23 $
 * $History: VS_MultiConfService.h $
 *
 * *****************  Version 23  *****************
 * User: Ktrushnikov  Date: 25.04.12   Time: 16:17
 * Updated in $/VSNA/Servers/AppServer/Services
 * #9715
 * - MaxParts fix at Join with conf_id at short and full formats
 * #7086
 * - Join to SpecConf at remote server is forbidden
 * - Join from remote server to SpecConf is declined
 *
 * *****************  Version 22  *****************
 * User: Sanufriev    Date: 16.02.12   Time: 19:06
 * Updated in $/VSNA/Servers/AppServer/Services
 * - key frame request for SVC server
 *
 * *****************  Version 21  *****************
 * User: Ktrushnikov  Date: 5.02.12    Time: 17:33
 * Updated in $/VSNA/Servers/AppServer/Services
 * - CreateStreamConference(): CheckTarifRestrictions default = true
 *
 * *****************  Version 20  *****************
 * User: Smirnov      Date: 26.12.11   Time: 19:24
 * Updated in $/VSNA/Servers/AppServer/Services
 * - added switch for SVC buffers. default = false
 *
 * *****************  Version 19  *****************
 * User: Ktrushnikov  Date: 3.10.11    Time: 15:36
 * Updated in $/VSNA/Servers/AppServer/Services
 * - lstatus: send only changed status (not all statuses)
 * - RemovePart_event: clean lstatus of disconnected user
 *
 * *****************  Version 18  *****************
 * User: Ktrushnikov  Date: 20.09.11   Time: 14:03
 * Updated in $/VSNA/Servers/AppServer/Services
 * [#9803]
 * - send lstatuses for joining participant on Join()
 * - dprint
 *
 * *****************  Version 17  *****************
 * User: Ktrushnikov  Date: 19.09.11   Time: 21:23
 * Updated in $/VSNA/Servers/AppServer/Services
 * #9802,#9803
 * - lerya statuses
 *
 * *****************  Version 16  *****************
 * User: Smirnov      Date: 30.08.10   Time: 20:48
 * Updated in $/VSNA/Servers/AppServer/Services
 * - bugfix#7653
 *
 * *****************  Version 15  *****************
 * User: Ktrushnikov  Date: 12.08.10   Time: 18:37
 * Updated in $/VSNA/Servers/AppServer/Services
 * [#7592]
 * - VS_ConfKick interface for store kicked users and deny them to connect
 * to conference in server
 *
 * *****************  Version 14  *****************
 * User: Smirnov      Date: 2.07.10    Time: 18:57
 * Updated in $/VSNA/Servers/AppServer/Services
 * - commands for bug#7496
 *
 * *****************  Version 13  *****************
 * User: Ktrushnikov  Date: 22.06.10   Time: 23:41
 * Updated in $/VSNA/Servers/AppServer/Services
 * Arch 3.1: NamedConfs added
 * - BS has Conference service
 * - Join_Method can create/join conf (if came from BS) or post request to
 * BS (if came from user)
 *
 * *****************  Version 12  *****************
 * User: Smirnov      Date: 15.02.10   Time: 18:50
 * Updated in $/VSNA/Servers/AppServer/Services
 * - licence restrictions reorganization
 * - SECUREBEGIN_A temporally removed
 *
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 12.02.10   Time: 13:13
 * Updated in $/VSNA/Servers/AppServer/Services
 * - named group conf alfa
 *
 * *****************  Version 10  *****************
 * User: Ktrushnikov  Date: 28.01.10   Time: 17:40
 * Updated in $/VSNA/Servers/AppServer/Services
 * vs_bc.dll:
 * - send to conf_srv (not to multiconf_srv)
 * - InviteUsers method
 *
 * *****************  Version 9  *****************
 * User: Ktrushnikov  Date: 13.01.10   Time: 21:43
 * Updated in $/VSNA/Servers/AppServer/Services
 * SS & DS support from old arch:
 * - ConnectServices added
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 13.10.08   Time: 21:37
 * Updated in $/VSNA/Servers/AppServer/Services
 * - some cleaning
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 4.07.08    Time: 20:00
 * Updated in $/VSNA/Servers/AppServer/Services
 * - rights depend on client protocol version
 * - don't alow users with protocol version less then 31
 *   participate in group conference
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 12.02.08   Time: 18:39
 * Updated in $/VSNA/Servers/AppServer/Services
 * - update config in ConfigSrv
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 5.02.08    Time: 16:42
 * Updated in $/VSNA/Servers/AppServer/Services
 * - direct conf half-repaired
 *
 * *****************  Version 4  *****************
 * User: Ktrushnikov  Date: 5.12.07    Time: 13:13
 * Updated in $/VSNA/Servers/AppServer/Services
 * - valid overload of Init()-method
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 26.11.07   Time: 22:02
 * Updated in $/VSNA/Servers/AppServer/Services
 * app server replacements done
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 26.11.07   Time: 17:51
 * Updated in $/VSNA/Servers/AppServer/Services

 ****************************************************************************/
/*****************************************************************************
 * \file	VS_MultiConfService.h
 * \brief	Process only multi conference requsts. The rest are in the
			conference servise file
 ****************************************************************************/

#ifndef VS_MULTI_CONF_SERVICE_H
#define VS_MULTI_CONF_SERVICE_H

#include "../../ServerServices/VS_SmtpMailerService.h"

#include "AppServer/Services/VS_Storage.h"
#include "VS_ConferenceService.h"
#include "std-generic/asio_fwd.h"

#include <boost/signals2/connection.hpp>
#include <boost/asio/ip/address.hpp>

#include "std-generic/compat/map.h"
#include <set>
#include <string>

namespace tc3_test { class MultiConfServiceTest; }

class VS_MulticastManager
{
	unsigned short	m_StartPort;
	unsigned short	m_EndPort;
	bool			m_IsValid;
	std::string			m_ip_str;
	boost::asio::ip::address	m_ip;

	typedef std::set<unsigned short> Ports;
	typedef std::map<boost::asio::ip::address, Ports> AddrPool;
	typedef std::pair<boost::asio::ip::address, Ports> AddrPair;

	static AddrPool m_AddrPool;
public:
	VS_MulticastManager();
	~VS_MulticastManager();
	bool Init(const char* initval, boost::asio::io_service* pios);
	void Release();
	bool IsValid();
	const char* GetIp();
	unsigned short AllocPort();
	void FreePort(const char *ip, unsigned short port);
};

struct ConferenceLayouts {
	std::string							common;
	std::string							mixer;
	vs::map<std::string, std::string>	parts;
};

struct VS_DeviceInfo {
	std::string							current;
	int32_t								volume;
	bool								mute;
	std::map<std::string, std::string>	device_list;
	VS_DeviceInfo() : volume(0), mute(false) {}
};

class VS_MultiConfService : public VS_ConferenceService
{
	std::chrono::steady_clock::time_point m_now;
	std::chrono::steady_clock::time_point Now();

	VS_Map				m_parts;
	VS_MulticastManager	m_mcast;
	bool				m_useDefaultStreamBuffer;
	unsigned			m_invite_check_time;
	uint32_t			m_last_sr_sysload_update_time;
	std::chrono::steady_clock::time_point		m_last_fecc_pending_time;
	// map[to, map[users,pair<req_type,time>]]
	vs::map<std::string, vs::map<std::string, std::pair<eFeccRequestType, std::chrono::steady_clock::time_point>>> m_fecc_pending;
	vs::map<std::string, ConferenceLayouts>		m_layouts;
	vs::map<std::string, std::map<std::string, VS_DeviceInfo>>	m_part_devices;
	std::vector<stream::ParticipantLoadInfo> m_load;

	boost::signals2::connection m_rtspAnnounceStatusReportConn;

	enum ModeratorAssymetricAlgo_t
	{
		e_invalid,
		e_one_leader,		// allow only one leader/owner; parts will disconnect from old owner when changing to new leader
		e_many_leaders		// allow many leaders; parts will disconnect only if dismiss from moderator/leader/owner
	};
	ModeratorAssymetricAlgo_t m_moderator_asymmetric_algo;

	void OnRTSPAnnounceStatusReport(string_view conf_name, string_view announce_id, bool is_active, string_view reason);
	void CheckPodiumsBeforeCast(long role, long fltr, vs_conf_id& conf_id);
public:
	VS_MultiConfService();
	~VS_MultiConfService(){}

	bool Init(const char* our_endpoint, const char* our_service, const bool permittedAll = false) override;
	void AsyncDestroy() override;
	bool Timer(unsigned long tickcount) override;
	bool Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;

	void CreateConference_Method(VS_Container &cnt); // not derived from VS_ConferenceService !!!
	long CreateStreamConference(long tarif_opt, const char *host, const char *app_id, VS_ConferenceDescription &cd, bool FromBS=false, const std::string& transcReserveToken = "");
	void Invite_Method(VS_Container &cnt);
	void InviteMulti_Method(VS_Container &cnt);
	void Join_Method(VS_Container &cnt);
	void SendPartsList(VS_ParticipantDescription &pd, VS_ParticipantListType action);
	void ReqInvite_Method(const char *host, const char* user_id, const char* dn);
	void InviteReply_Method(VS_Container &cnt);
	void InviteUsers_Method(VS_Container &cnt);
	void ConnectReciever_Method(const char* party, long fltr, const char *from);
	void ConnectSender_Method(long fltr, const char *from);
	void ConnectServices_Method(long fltr, const char *from);
	void PingParticipant_Method(const char *conf, const char *part, int32_t *sysLoad);
	void RoleEvent_Method(VS_Container &cnt);
	void FECC_Method(VS_Container &cnt);
	void QueryParticipants_Method(VS_Container &cnt);

	void CheckMaxCast(unsigned long NumOfCast, VS_ConferenceDescription &cd, unsigned long NumOfReport, VS_ParticipantDescription ** Reporters, unsigned long NumOfLeaders_Cast, VS_ParticipantDescription ** Leaders_Cast);

	void DeviceStatus_Method(VS_Container &cnt);
	void SetMyLStatus_Method(VS_Container &cnt);
	void ClearAllLStatuses_Method(VS_Container &cnt);
	void UpdateFrameMBpsParticipants_Method(VS_Container &cnt);
	void UpdateAllLStatuses(const char* conf_name, bool reset=false, const char* only_for_call_id=0);
	void UpdateLStatus(const char* conf_name, const char* call_id, unsigned long lstatus);
	void ReceiverConnected(VS_StreamPartDesc &spd, long result, const char* dst_server, const char* dst_user);
	void SenderConnected(VS_StreamPartDesc &spd, long result, const char* dst_server, const char* dst_user);
	void UpdateMediaFiltr(VS_ParticipantDescription &pd, long fltr);
	void LogConnectFailure_Method(VS_Container &cnt);

	void ManageConference(VS_Container &cnt);
	void MC_SetLayout(string_view stream_id, const VS_ParticipantDescription& pd, layout_for f, string_view new_layout);
	void MC_ChangeDevice(VS_ParticipantDescription &pd, const char* type, const char* id, const char* name);
	void MC_SetDeviceVol(VS_ParticipantDescription &pd, const char* type, const char* id, int32_t volume);
	void MC_SetDeviceMute(VS_ParticipantDescription &pd, const char* type, const char* id, bool mute);
	void LogParticipantDevice(const char* conf, const char* call_id, const char* action, const char* device_id, const char* device_type,
		const char* device_name, bool is_active = false, bool* pmute = nullptr, int32_t* pvolume = nullptr);
	void LogParticipantRole(const VS_ParticipantDescription& pd);
	void ResendSlidesCommand(string_view conf, string_view part);
	void DeviceList_Method(VS_Container &cnt);
	void DeviceChanged_Method(VS_Container &cnt);
	void DeviceState_Method(VS_Container &cnt);
	void QueryDevices_Method(VS_Container &cnt);

	void RemoveParticipant_Event(const char* user, long cause, VS_ConfPartStat* ps = NULL);
	void RemoveConference_Event(VS_ConferenceDescription& cd, long cause = 0);

	int RestrictBitrate(const char* conf_name, const char* part_name, int bitrate) override;
	int RestrictBitrateSVC(const char* conf_name, const char* part_name, int v_bitrate, int bitrate, int old_bitrate) override;
	void RequestKeyFrame(const char* conf_name, const char* part_name) override;
	void SetParticipantCaps(const char* conf_name, const char* part_name, const void* caps_buf, size_t buf_sz) override;

	void SendCommand_Method(VS_Container &cnt);
	void TurnOffReceiverVideo(VS_StreamPartDesc* spd, const char* dst_server, const char* dst_user);
	void SetRole(VS_ParticipantDescription &pd, long role);
	void AnswerRole(VS_ParticipantDescription &pd, long role, long result, const char* from = 0);
	void NotifyRole(VS_ParticipantDescription &pd);
	void ParticipantConnectsAtJoin(VS_ConferenceDescription& cd, VS_ParticipantDescription &pd, long fltr);
	void SetOrInviteOnPodiumByLayout(VS_ConferenceDescription& cd, const std::string& layout, const char* part, bool invite);
	void RequestReporterRole(VS_ParticipantDescription &pd);
	void TakeSlot(const VS_ConferenceDescription& cd, string_view part_instance);
	void FreeSlot(const VS_ConferenceDescription& cd, string_view part_instance);
	long CheckParticipantsConnects(VS_ConferenceDescription& cd,
		VS_ParticipantDescription &to, VS_ParticipantDescription &from, long fltr);

	enum VS_MultiRcv_Direction{
		MRVD_FORWARD = 1,
		MRVD_BACKWARD = 2
	};

	bool ConnectServices(VS_ParticipantDescription &pd,  long fltr);
	bool ConnectPart(VS_ParticipantDescription &pd, const char *party, long fltr, int dir);
	int  NewPartConnect(
		VS_StreamPartDesc &spd,
		VS_ParticipantDescription &pd1,
		VS_ParticipantDescription &pd2,
		long type, long fltr);
	bool DisconnectPart(const char *part, int type);
	void StreamPartRemoved(const char* conf, const char* part, VS_SimpleStr &Snd, VS_SimpleStr &Rcv);

	void OnTimer_CheckInvites();

	VS_SimpleStr m_fake_name;
	virtual char* OurService()
	{	return  m_fake_name; };

	void UserRegistrationInfo_Method(VS_Container &cnt);

	friend tc3_test::MultiConfServiceTest;
};


#endif // VS_MULTI_CONF_SERVICE_H
