/*****************************************************************************
 *
 * Project: Services - Media streaming
 *
 ****************************************************************************/
/*****************************************************************************
   \file	VS_MultiConfService.cpp
   \brief	Process only multi conference requsts. The rest are in the
			conference servise file
 ****************************************************************************/


#include "VS_MultiConfService.h"

#include "../../common/streams/Router/Router.h"
#include "../../common/streams/Router/DefaultBuffer.h"
#include "../../common/streams/Router/SVCBuffer.h"

#include "../../ServerServices/Common.h"
#include "../../common/std/cpplib/VS_MemoryLeak.h"

#include "../../common/std/cpplib/VS_RegistryKey.h"
#include "../../common/std/cpplib/VS_RegistryConst.h"
#include "../../common/std/cpplib/VS_RcvFunc.h"
#include "../../common/std/cpplib/VS_ClientCaps.h"
#include "../../common/std/cpplib/VS_IntConv.h"
#include "../../common/std/cpplib/VS_Utils.h"
#include "../../common/std/cpplib/VS_CallIDUtils.h"
#include "std/cpplib/VS_Replace.h"
#include "std/cpplib/json/elements.h"
#include "std/cpplib/json/reader.h"
#include "std/cpplib/json/writer.h"

#include "VS_AppServerData.h"
#include "ProtectionLib/Protection.h"
#include "../../common/TransceiverLib/VS_ConfRecorderModuleCtrl.h"
#include "../../common/TransceiverLib/VS_RTSPBroadcastModuleCtrl.h"
#include "../../common/TransceiverLib/VS_ConfControlModule.h"
#include "../../common/transport/Router/VS_TransportRouterServiceHelper.h"
#include "../../common/tools/Server/VS_Server.h"
#include "../../common/tools/Server/VS_ServerComponentsInterface.h"
#include "../../common/std/cpplib/curl_deleters.h"
#include "std-generic/cpplib/ignore.h"
#include "std-generic/clib/strcasecmp.h"
#include "net/Lib.h"
#include "TransceiverLib/TransceiversPool.h"
#include "TransceiverLib/VS_ConfControlModule.h"
#include "TrueGateway/TransportTools.h"
#include "std/cpplib/layout_json.h"

#include <rapidjson/document.h>
#include <rapidjson/reader.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <curl/curl.h>
#include <boost/algorithm/string/trim.hpp>

#include <functional>
#include <stdio.h>

#define DEBUG_CURRENT_MODULE VS_DM_MCS

#define DEFAULT_MULTICAST_IP	"224.0.1.224:4000-6000"

extern std::string g_tr_endpoint_name;

namespace rj = rapidjson;

boost::asio::ip::address GetIpFromStr(const char *str) {
	boost::system::error_code ec;
	boost::asio::ip::address ip = boost::asio::ip::address_v4::from_string(str, ec);

	if (!ec && !ip.is_unspecified()) return ip;
	return boost::asio::ip::address_v6::from_string(str, ec);
}

VS_MulticastManager::AddrPool VS_MulticastManager::m_AddrPool;

////////////////////// class VS_MulticastManager ////////////////////////
VS_MulticastManager::VS_MulticastManager() : m_StartPort(0), m_EndPort(0), m_IsValid(false)
{

}

VS_MulticastManager::~VS_MulticastManager() {
	Release();
}

#include "ProtectionLib/OptimizeDisable.h"
bool VS_MulticastManager::Init(const char* initval, boost::asio::io_service* pios) {
	assert(pios != nullptr);
	if (!initval) return false;
	bool result(false);
NANOBEGIN2;
	Release();

	std::string ip_port(initval);
	size_t port_begin = ip_port.find_last_of(":");
	bool target_ipv6 = false;
	std::string ip = ip_port.substr(0, port_begin);

	boost::asio::ip::address addr;
	boost::system::error_code ec;

	if (ip[0] == '[') { // remove brackets from ipv6 address
		boost::trim_if(ip, [](char c) {return c == '[' || c == ']'; });
		target_ipv6 = true;
		addr = boost::asio::ip::address_v6::from_string(ip, ec);
	}
	else
		addr  = GetIpFromStr(ip.c_str());

	std::string ports = ip_port.substr(port_begin + 1);

	if (!ports.empty()) {
		if (!ip.empty() && !addr.is_unspecified() && !ec) {
			m_ip = addr;
			m_ip_str = ip;
		}
		else {
			auto ipaddr = net::MakeA_lookup(ip, *pios, target_ipv6);
			if (ipaddr.is_unspecified()) result = false;
			else {
				m_ip = ipaddr;
			}
		}

		m_StartPort = ::atoi(ports.c_str());
		ports = ports.substr(ports.find_first_of("-") + 1);
	}
	if (!ports.empty() && !m_ip.is_unspecified()) {
		m_EndPort = ::atoi(ports.c_str());
		if (m_StartPort > 0 && m_EndPort > 0) {
			if (m_EndPort < m_StartPort) {
				std::swap(m_StartPort, m_EndPort);
			}
			m_IsValid = true;
			result = true;
		}
	}
NANOEND2;
	return result;
}
#include "ProtectionLib/OptimizeEnable.h"

void VS_MulticastManager::Release() {
	m_StartPort = m_EndPort = 0;
	m_ip = boost::asio::ip::address_v4::from_string("255.255.255.255");
	m_ip_str.clear();
	m_IsValid = false;
}

bool VS_MulticastManager::IsValid() {
	return m_IsValid;
}

const char* VS_MulticastManager::GetIp() {
	return m_ip_str.data();
}

#include "ProtectionLib/OptimizeDisable.h"
unsigned short VS_MulticastManager::AllocPort() {
	unsigned short result(0);
NANOBEGIN2;
	AddrPool::iterator it = m_AddrPool.find(m_ip);
	if (it == m_AddrPool.end())
	{
		auto it1 = m_AddrPool.emplace(m_ip, Ports());
		it = it1.first;
		(*it).second.insert(m_StartPort);
		result = m_StartPort;
	}
	else
	{
		Ports &ports = (*it).second;
		for (unsigned short i = m_StartPort; i<=m_EndPort; i++)
		{
			if (ports.find(i) == ports.end())
			{
				ports.insert(i);
				result = i;
				break;
			}
		}
	}
NANOEND2;
	return result;
}

void VS_MulticastManager::FreePort(const char *ip, unsigned short port) {
	if (!ip || !*ip || !port)
		return;
NANOBEGIN2;
	auto addr = GetIpFromStr(ip);
	if (!addr.is_unspecified()) {
		AddrPool::iterator it;
		if ((it = m_AddrPool.find(addr)) != m_AddrPool.end())
		{
			Ports &ports = (*it).second;
			ports.erase(port);
			if (ports.empty())
				m_AddrPool.erase(it);
		}
	}
NANOEND2;
}
#include "ProtectionLib/OptimizeEnable.h"

////////////////////// END class VS_MulticastManager ////////////////////////

extern int VS_CheckFmt(VS_BinBuff &to, VS_BinBuff &from, int fltr);
extern int VS_SetRcvDevStatus(long &dvs, int s, int d, int type);
extern bool VS_CheckDynChange(VS_BinBuff &to);
extern unsigned char VS_GetLevelPerformance(VS_BinBuff &to);
extern int VS_GetSndMBps(VS_BinBuff &from);
extern int VS_GetSndFrameSizeMB(VS_BinBuff &from);
// class defifnition

////////////////////// class VS_MultiConfService ////////////////////////
extern int g_rating_th;

VS_MultiConfService::VS_MultiConfService()
	: VS_ConferenceService()
	, m_invite_check_time(0)
	, m_last_sr_sysload_update_time(0)
	, m_fake_name(CONFERENCE_SRV)
{
	m_TimeInterval = std::chrono::seconds(1);
	m_parts.SetPredicate(VS_Map::StrIPredicate);
	m_parts.SetKeyFactory(vs_user_id::Factory, vs_user_id::Destructor);
	m_parts.SetDataFactory(VS_StreamPartMap::Factory, VS_StreamPartMap::Destructor);
	VS_RegistryKey cfg(false, CONFIGURATION_KEY);
	int val = 1;
	cfg.GetValue(&val, 4, VS_REG_INTEGER_VT, "UseSVC");
	m_useDefaultStreamBuffer = val==0;
}

bool VS_MultiConfService::Init(const char *our_endpoint, const char *our_service, const bool permittedAll)
{
	VS_ConferenceService::Init(our_endpoint, our_service, permittedAll);
	m_rtspAnnounceStatusReportConn = m_RTSPBroadcastModule->ConnectToAnnounceStatusReport(
		std::bind(&VS_MultiConfService::OnRTSPAnnounceStatusReport, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)
	);

	m_moderator_asymmetric_algo = e_many_leaders;
	VS_RegistryKey cfg(false, CONFIGURATION_KEY);
	if (cfg.IsValid())
	{
		cfg.GetValue(&m_moderator_asymmetric_algo, 4, VS_REG_INTEGER_VT, "Moderator Asymmetric Algo");
		if (m_moderator_asymmetric_algo <= e_invalid || m_moderator_asymmetric_algo > e_many_leaders)
			m_moderator_asymmetric_algo = e_many_leaders;
	}

	return true;
}

void VS_MultiConfService::AsyncDestroy()
{
	m_rtspAnnounceStatusReportConn.disconnect();
	VS_ConferenceService::AsyncDestroy();
}

////////////////////////////////////////////////////////////////////////////////
// Main service
////////////////////////////////////////////////////////////////////////////////
bool VS_MultiConfService::Processing( std::unique_ptr<VS_RouterMessage>&&recvMess)
{
	VS_AutoLock lock(this);
	if (recvMess == 0)		return true;
	VS_Container cnt;
	switch (recvMess->Type()) {
	case transport::MessageType::Invalid:
		break;
	case transport::MessageType::Request:
		m_recvMess = recvMess.get();
		if (cnt.Deserialize(recvMess->Body(), recvMess->BodySize())) {
			// Search method name
			const char* method = 0;
			if ((method = cnt.GetStrValueRef(METHOD_PARAM)) != 0) {
				dprint3("Method %20s| srv %20s | user %20s\n", method, recvMess->SrcServer(), recvMess->SrcUser());
				// Process methods
				if (strcasecmp(method, CREATECONFERENCE_METHOD) == 0) {
					CreateConference_Method(cnt);
				} else if (strcasecmp(method, DELETECONFERENCE_METHOD) == 0) {
					int32_t cause = 0;
					cnt.GetValue(CAUSE_PARAM, cause);
					DeleteConference_Method(cnt.GetStrValueRef(NAME_PARAM), cause,cnt.GetStrValueRef(SESSION_PARAM));
				} else if (strcasecmp(method, INVITE_METHOD) == 0) {
					Invite_Method(cnt);
				} else if (strcasecmp(method, INVITETOMULTI_METHOD) == 0) {
					InviteMulti_Method(cnt);
				} else if (strcasecmp(method, ACCEPT_METHOD) == 0) {
					Accept_Method(cnt);
				} else if (strcasecmp(method, REJECT_METHOD) == 0) {
					Reject_Method(cnt);
				} else if (strcasecmp(method, HANGUP_METHOD) == 0) {
					int32_t result = 0;
					HangupFlags hflags(HangupFlags::NONE);
					cnt.GetValue(RESULT_PARAM, result);
					cnt.GetValueI32(HANGUP_FLAGS_PARAM, hflags);
					Hangup_Method(cnt.GetStrValueRef(CONFERENCE_PARAM), cnt.GetStrValueRef(NAME_PARAM), result, hflags);
				} else if (strcasecmp(method, JOIN_METHOD) == 0) {
					if (!IsKickedUser(cnt.GetStrValueView(NAME_PARAM), cnt.GetStrValueView(USERNAME_PARAM), m_confRestriction.get()))
						Join_Method(cnt);
					else {
						VS_Container rCnt;
						rCnt.AddValue(METHOD_PARAM, JOIN_METHOD);
						rCnt.AddValueI32(RESULT_PARAM, REJECTED_BY_ACCESS_DENIED);
						bool FromBS(IsBS(m_recvMess->SrcServer()));
						if (FromBS) {
							vs_user_id user_id = m_recvMess->SrcUser();
							if (!user_id) user_id = cnt.GetStrValueRef(USERNAME_PARAM);
							if (!!user_id)
								PostRequest(cnt.GetStrValueRef(SERVER_PARAM), user_id, rCnt);
						} else
							PostReply(rCnt);
					}
				} else if (strcasecmp(method, KICK_METHOD) == 0) {
					Kick_Method(cnt);
				} else if (strcasecmp(method, IGNORE_METHOD) == 0) {
					Ignore_Method(cnt.GetStrValueRef(CONFERENCE_PARAM), cnt.GetStrValueRef(NAME_PARAM));
				} else if (strcasecmp(method, CONNECTRECEIVER_METHOD) == 0) {
					int32_t fltr = 0;
					cnt.GetValue(MEDIAFILTR_PARAM, fltr);
					ConnectReciever_Method(cnt.GetStrValueRef(NAME_PARAM), fltr, cnt.GetStrValueRef(USERNAME_PARAM));
				} else if (strcasecmp(method, CONNECTSENDER_METHOD) == 0) {
					int32_t fltr = 0;
					cnt.GetValue(MEDIAFILTR_PARAM, fltr);
					ConnectSender_Method(fltr, cnt.GetStrValueRef(USERNAME_PARAM));
				} else if (strcasecmp(method, CONNECTSERVICES_METHOD) == 0) {
					int32_t fltr = 0;
					cnt.GetValue(SERVICES_PARAM, fltr);
					ConnectServices_Method(fltr, cnt.GetStrValueRef(USERNAME_PARAM));

				} else if (strcasecmp(method, PING_METHOD) == 0) {
					PingParticipant_Method(cnt.GetStrValueRef(CONFERENCE_PARAM), cnt.GetStrValueRef(USERNAME_PARAM), cnt.GetLongValueRef(SYSLOAD_PARAM));
				} else if (strcasecmp(method, MBPSLIST_METHOD) == 0) {
					UpdateFrameMBpsParticipants_Method(cnt);
				} else if (strcasecmp(method, REQINVITE_METHOD) == 0) {
					const char* src_user = cnt.GetStrValueRef(TRANSPORT_SRCUSER_PARAM);
					const char* from_user = src_user? cnt.GetStrValueRef(CALLID_PARAM): m_recvMess->SrcUser();
					const char* host = (src_user)? src_user: cnt.GetStrValueRef(CALLID_PARAM);
					VS_ConferenceDescription cd;
					bool local = (host && *host)? g_storage->FindConferenceByUser(host, cd): false;
					if (!local || !IsKickedUser(SimpleStrToStringView(cd.m_name), from_user ? string_view(from_user) : string_view(), m_confRestriction.get()))
					{
						ReqInvite_Method(host, from_user, cnt.GetStrValueRef(DISPLAYNAME_PARAM));
					}else {
						VS_Container rCnt;
						rCnt.AddValue(METHOD_PARAM, REJECT_METHOD);
						rCnt.AddValue(CALLID_PARAM, host);
						rCnt.AddValue(CONFERENCE_PARAM, "");
						rCnt.AddValueI32(CAUSE_PARAM, REJECTED_BY_ACCESS_DENIED);
						PostRequest(m_recvMess->SrcServer(), from_user, rCnt);
					}
				} else if (strcasecmp(method, DELETEPARTICIPANT_METHOD) == 0) {
					int32_t cause = VS_ParticipantDescription::DISCONNECT_BYSTREAM;
					cnt.GetValue(CAUSE_PARAM, cause);
					VS_ConfPartStat stat;
					stat.part = cnt.GetStrValueRef(USERNAME_PARAM);
					stat.conf = cnt.GetStrValueRef(CONFERENCE_PARAM);
					size_t size = 0;
					const void * data = cnt.GetBinValueRef(DATA_PARAM, size);
					if (data && size == sizeof(stream::ParticipantStatistics)) {
						stat.stat = *static_cast<const stream::ParticipantStatistics*>(data);
						RemoveParticipant_Event(stat.part, cause, &stat);
					}
					else {
						RemoveParticipant_Event(stat.part, cause);
					}
				}
				else if (strcasecmp(method, UPDATEPARTICIPANT_METHOD) == 0) {
					VS_ParticipantDescription pd;
					if (g_storage->FindParticipant(cnt.GetStrValueView(USERNAME_PARAM), pd)) {
						SendPartsList(pd, PLT_UPD);
					}
				} else if (strcasecmp(method, SENDCOMMAND_METHOD) == 0) {
					SendCommand_Method(cnt);
				}
				else if (strcasecmp(method, ROLEEVENT_METHOD) == 0) {
					RoleEvent_Method(cnt);
				}
				else if (strcasecmp(method, DEVICESTATUS_METHOD) == 0) {
					DeviceStatus_Method(cnt);
				} else if (strcasecmp(method, INVITEREPLY_METHOD) == 0) {
					InviteReply_Method(cnt);
				} else if (strcasecmp(method, INVITEUSERS_METHOD) == 0) {
					InviteUsers_Method(cnt);
				} else if (strcasecmp(method, SETMYLSTATUS_METHOD) == 0) {
					VS_SimpleStr value;
					if (m_confRestriction->GetAppProp("lstatus_set", value) && !!value)
						SetMyLStatus_Method(cnt);
				} else if (strcasecmp(method, CLEARALLLSTATUSES_METHOD) == 0) {
					VS_SimpleStr value;
					if (m_confRestriction->GetAppProp("lstatus_set", value) && !!value)
						ClearAllLStatuses_Method(cnt);
				} else if (strcasecmp(method, CONNECTFAILURE_METHOD) == 0) {
					LogConnectFailure_Method(cnt);
				} else if (strcasecmp(method, USERREGISTRATIONINFO_METHOD) == 0) {
					UserRegistrationInfo_Method(cnt);
				} else if (strcasecmp(method, FECC_METHOD) == 0) {
					FECC_Method(cnt);
				} else if (strcasecmp(method, SEARCHADDRESSBOOK_METHOD) == 0) {
					QueryParticipants_Method(cnt);
				} else if (strcasecmp(method, MANAGECONEFRENCE_METHOD) == 0) {
					ManageConference(cnt);
				} else if (strcasecmp(method, DEVICESLIST_METHOD) == 0) {
					DeviceList_Method(cnt);
				} else if (strcasecmp(method, DEVICECHANGED_METHOD) == 0) {
					DeviceChanged_Method(cnt);
				} else if (strcasecmp(method, DEVICESTATE_METHOD) == 0) {
					DeviceState_Method(cnt);
				} else if (strcasecmp(method, QUERYDEVICES_METHOD) == 0) {
					QueryDevices_Method(cnt);
				}
			}
		}
		break;
	case transport::MessageType::Reply:
		break;
	case transport::MessageType::Notify:
		break;
	}
	m_recvMess = nullptr;
	return true;
}

void VS_MultiConfService::CreateConference_Method(VS_Container &cnt)
{
	const auto user_id = m_recvMess->SrcUser_sv();
	VS_UserData ud;
	if (!g_storage->FindUser(user_id, ud))
		return;
	VS_SimpleStr	homeBS = ud.m_homeServer;

	std::string transceiverReserveToken;
	int32_t duration = ~0, maxParticipants = 1, type = CT_PRIVATE, subType = GCST_FULL, scope = GS_PERSONAL, maxcasts = 4;
	cnt.GetValue(DURATION_PARAM, duration);
	cnt.GetValue(MAXPARTISIPANTS_PARAM, maxParticipants);
	cnt.GetValue(MAXCAST_PARAM, maxcasts);
	cnt.GetValue(TYPE_PARAM, type);
	cnt.GetValue(SUBTYPE_PARAM, subType);
	cnt.GetValue(SCOPE_PARAM, scope);
	cnt.GetValue(RESERVATION_TOKEN, transceiverReserveToken);
	const char* pass = cnt.GetStrValueRef(PASSWORD_PARAM);
	const char* name = cnt.GetStrValueRef(NAME_PARAM);
	const char* lang = cnt.GetStrValueRef(LANG_PARAM);
	const char* topic = cnt.GetStrValueRef(TOPIC_PARAM);
	size_t size;
	const void* buff = cnt.GetBinValueRef(CLIENTCAPS_PARAM, size);
	VS_BinBuff caps(buff, size);

	if (type!= CT_MULTISTREAM && type!= CT_INTERCOM)
		return VS_ConferenceService::CreateConference_Method(duration, maxParticipants, type, caps, pass, transceiverReserveToken);

	VS_ConferenceDescription cd;

	// find any conf with the same owner
	int32_t result = g_storage->FindConferenceByName(user_id, cd);
	if (result != 0) {
		// not found, create
		result = CREATE_ACCESS_DENIED;
		if (ud.m_rights & VS_UserData::UR_COMM_CREATEMULTI)
		{
			cd.m_owner = StringViewToSimpleStr(user_id);
			cd.m_type = type;
			cd.m_SubType = subType;
			if(topic) cd.m_topic = topic;
			if (name && cd.m_topic.empty()) {
				if (atou_s(m_recvMess->AddString()) < 40) {
					if (strlen(name) > 3)		// avoid "@c_" prefix
						cd.m_topic = name+3;
				}
				else {
					cd.m_topic = name;
				}
			}
			cd.m_public = scope==GS_PUBLIC;
			cd.m_lang = lang;
			cd.m_MaxParticipants = maxParticipants;
			cd.m_MaxCast = maxcasts;
			cd.SetTimeExp(TIME_EXP_ACCEPT);

			if (m_confRestriction->Tarif_CreateConf(cnt, cd, ud, this))
				result = CreateStreamConference(ud.m_tarif_restrictions, user_id.c_str(), ud.m_appID, cd, false, transceiverReserveToken);
			else
				result = CREATE_ACCESS_DENIED;
		}
	}
	else {
		dstream2 << "Conf with name exists, join to it  <" << cd.m_name.m_str << ">" <<
			", maxp=" << cd.m_MaxParticipants << ", maxcast=" << cd.m_MaxCast <<
			", t=" << cd.m_type <<
			", st=" << cd.m_SubType <<
			", is_pub=" << cd.m_public <<
			", res=" << result <<
			", topic= " << cd.m_topic << "\n";
	}

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, CONFERENCECREATED_METHOD);
	if (result == 0) {
		rCnt.AddValueI32(RESULT_PARAM, CONFERENCE_CREATED_OK);
		rCnt.AddValue(NAME_PARAM, cd.m_name);
		PostReply(rCnt);

		rCnt.Clear();
		rCnt.AddValue(TYPE_PARAM, type);
		rCnt.AddValue(PASSWORD_PARAM, pass);
		rCnt.AddValue(NAME_PARAM, cd.m_name);
		rCnt.AddValue(USERNAME_PARAM, user_id);
		rCnt.AddValue(CLIENTCAPS_PARAM, buff, size);
		rCnt.AddValue(APPID_PARAM,ud.m_appID);
		Join_Method(rCnt);
	}
	else {
		rCnt.AddValueI32(RESULT_PARAM, result);
		PostReply(rCnt);
	}
}

long VS_MultiConfService::CreateStreamConference(long tarif_opt, const char *host, const char *app_id, VS_ConferenceDescription &cd, bool FromBS, const std::string& transcReserveToken)
{
	cd.m_appID = app_id;
	if (!cd.m_owner)
	{
		cd.m_owner = host;
		// todo(kt): make role of host = PR_LEADER
	}
	VS_UserData	ud;

	if (!FromBS && !m_confRestriction->IsVCS()) {
		if (!host || !g_storage->FindUser(host,ud))
			return VSS_CONF_NOT_STARTED;
		tarif_opt = ud.m_tarif_restrictions;
	}

	long result = m_confRestriction->CheckInsertMultiConf(tarif_opt, cd, host, FromBS);
	cd.m_need_record = m_confRestriction->DoWriteConference(cd);
	cd.m_isBroadcastEnabled = m_confRestriction->DoBroadcastConference(cd);

	if (result == 0) {
		result = g_storage->InsertConference(cd);
		if (result == 0) {
			if (!transcReserveToken.empty()) {
				assert(cd.m_name.m_str != nullptr);
				gw::ConnectReservedProxyToConf(m_transceiversPool, transcReserveToken, cd.m_name.m_str);
			}
			if (cd.m_type==CT_MULTISTREAM) {
				unsigned long MaxParticipants = 0;
				if (cd.m_SubType == GCST_FULL || cd.m_SubType == GCST_ALL_TO_OWNER || cd.m_SubType == GCST_PENDING_ALL_TO_OWNER)
					MaxParticipants = cd.m_MaxParticipants*cd.m_MaxParticipants;
				else if (cd.m_SubType == GCST_ROLE)
					MaxParticipants = cd.m_MaxParticipants + (cd.m_MaxCast+1)*cd.m_MaxParticipants;	// N + Cast*N + 1N
				if (!m_sr->CreateConference(cd.m_name.m_str, (VS_Conference_Type)cd.m_type, cd.m_symKey, MaxParticipants, false, std::chrono::seconds(TIME_EXP_INVITE))) {
					result = VSS_CONF_ROUTER_DENIED;
				}
			}
			else if (cd.m_type!=CT_INTERCOM)
				result = VSS_CONF_NOT_VALID;
			if (result!= 0)
				g_storage->DeleteConference(cd);
		}
	}

	dstream2 << "Create MC <" << cd.m_name.m_str << ">" <<
		", maxp=" << cd.m_MaxParticipants << ", maxcast=" << cd.m_MaxCast <<
		", t=" << cd.m_type <<
		", st=" << cd.m_SubType <<
		", is_pub=" << cd.m_public <<
		", res=" << result <<
		", topic= " << cd.m_topic << "\n";
	if (result == 0) {
		m_confRestriction->UpdateMultiConference(cd, true);
		///LOG start multiConf
		LogConferenceStart(cd);
		if (cd.m_need_record)
		{
			m_confWriterModule->StartRecordConference(cd.m_name);
			m_confRestriction->SetRecordState(cd, RS_RECORDING);
		}

		auto layout = m_confRestriction->ReadLayout(SimpleStrToStringView(cd.m_call_id), layout_for::mixer);
		auto transceiversPool = m_transceiversPool.lock();
		if (!transceiversPool) {
			dstream1 << "Error\n TransceiversPool not found!\n";
			return result;
		}
		auto streamsCircuit = transceiversPool->GetTransceiverProxy(cd.m_name.m_str);
		if (streamsCircuit) {
			if (auto confCtrl = streamsCircuit->ConfControl()) confCtrl->UpdateLayout(cd.m_name, layout.c_str());
		}

		if (cd.m_isBroadcastEnabled)
		{
			std::string url("c/");
			url += cd.m_call_id.m_str;
			m_RTSPBroadcastModule->StartBroadcast(cd.m_name.m_str, url, cd.m_topic, cd.m_rtspEnabledCodecs, cd.m_rtspHelperProgram);
			for (const auto& announce : cd.m_rtspAnnounces)
				m_RTSPBroadcastModule->AnnounceBroadcast(
					cd.m_name.m_str,
					announce.first,
					announce.second.url,
					announce.second.username,
					announce.second.password,
					announce.second.rtp_over_tcp,
					announce.second.enabled_codecs,
					announce.second.keepalive_timeout,
					announce.second.retries,
					announce.second.retry_delay
				);
		}

	}else if (m_confRestriction->IsSpecialConf(cd.m_call_id)){
		m_confRestriction->UpdateMultiConference(cd, false);
	}

	return result;
}


// join part: create new conf if not exist
// create new with default filt param
// create all to all connects
void VS_MultiConfService::Join_Method(VS_Container &cnt)
{
	const char *name = cnt.GetStrValueRef(NAME_PARAM);
	const char *pass = cnt.GetStrValueRef(PASSWORD_PARAM);
	const auto from = cnt.GetStrValueView(USERNAME_PARAM);
	const char *app_id = cnt.GetStrValueRef(APPID_PARAM);
	const char *display_name = cnt.GetStrValueRef(DISPLAYNAME_PARAM);

	int32_t type = -1; cnt.GetValue(TYPE_PARAM, type);
	size_t size = 0;
	const void *buff = cnt.GetBinValueRef(CLIENTCAPS_PARAM, size);
	VS_ClientCaps cc(buff, size);

	dprint4("Join_Method <%s> <%s> %d \n", name? name:"", pass?pass:"", type);

	if (!name || !*name)	return;
	if (!pass || type==CT_BROADCAST || type==CT_PUBLIC || type==CT_VIPPUBLIC || type==CT_HALF_PRIVATE)
		return VS_ConferenceService::Join_Method(cnt);

	VS_ConferenceDescription	cd;
	VS_ParticipantDescription	pd;
	VS_Container rCnt;
	int32_t result = 0;
	int32_t fltr = 0;

	vs_user_id user_id = m_recvMess->SrcUser();
	if (!user_id)
		user_id = StringViewToSimpleStr(from);
	if (!user_id)
		return;

	bool FromBS(IsBS(m_recvMess->SrcServer()));
	VS_SimpleStr server = VS_GetConfEndpoint(name);
	auto server_type = server ? VS_GetServerType(server.m_str) : ST_UNKNOWN;
	bool ConfAtRemoteVCS((server_type==ST_VCS || server_type==ST_AS) && server!=OurEndpoint());
	bool ConfAtOurVCS = !ConfAtRemoteVCS;

	bool found = g_storage->FindConference(name, cd);

	if (m_confRestriction->IsSpecialConf(name))							// special_conf ($c50asd)
	{
		if (!found && (ConfAtRemoteVCS ||								// our user tries to join special conf at other server (it is forbidden)
			(!VS_IsNotTrueConfCallID(user_id.m_str) && (VS_RealUserLogin(SimpleStrToStringView(user_id)).IsOurSID() != true))))		// deny remote participant (but allow unath sip #20053)
		{
			rCnt.AddValue(METHOD_PARAM, JOIN_METHOD);
			rCnt.AddValueI32(RESULT_PARAM, INVALID_CONFERENCE);
			if (FromBS)
				PostRequest(cnt.GetStrValueRef(SERVER_PARAM), user_id, rCnt);
			else
				PostReply(rCnt);
			return ;
		}
	}

	bool stream_conf_created(false);
	if (!found) {
		result = m_confRestriction->FindMultiConference(name, cd, cnt, user_id, FromBS);
		if (result == VSS_CONF_NOT_STARTED) {
			if (!!cd.m_name || cd.m_owner == user_id) // allow for: already going on, owner of conference
				result = 0;
		}
		if (result == 0) {
			if (!!cd.m_password || cd.m_PlannedPartsOnly) {
				if (!m_confRestriction->IsPlannedParticipant(cd, user_id)) {
					if (!!cd.m_password) {
						bool has_conf_pass = (cc.GetClientFlags() & VS_ClientCaps::ClientFlags::CONFERECNCE_PASS) != VS_ClientCaps::ClientFlags::NONE;
						if (!pass || !*pass)
							result = has_conf_pass ? CONFERENCE_PASSWORD_REQUIRED : REJECTED_BY_ACCESS_DENIED;
						else if (cd.m_password != pass)
							result = has_conf_pass ? REJECTED_BY_WRONG_PASSWORD : REJECTED_BY_ACCESS_DENIED;
					}
					else
						result = REJECTED_BY_ACCESS_DENIED;
				}
				else {
					// allow for planned participate without password checking
				}
			}
		}
		if (result == 0) {
			if (!!cd.m_name) {
				found = g_storage->FindConference(SimpleStrToStringView(cd.m_name), cd);
				if (!found)
					result = VSS_CONF_NOT_FOUND;
			}
			else {
				std::string transceiverReserveToken;
				cnt.GetValue(RESERVATION_TOKEN, transceiverReserveToken);
				//TODO: place real max paticipants restrictions
				result = CreateStreamConference(0x04781009, user_id, app_id, cd, FromBS, transceiverReserveToken);
				found = stream_conf_created = result == 0;

				VS_Container aiCnt;
				aiCnt.AddValue(METHOD_PARAM, AUTOINVITE_METHOD);
				aiCnt.AddValue(CONFERENCE_PARAM, cd.m_call_id);
				aiCnt.AddValue(CALLID_PARAM, user_id);
				PostRequest(OurEndpoint(), 0, aiCnt, 0, AUTH_SRV);
			}
		}
	}

	if (!found)		// No conference found: Send to BS
	{
		VS_UserData ud;
		if (g_storage->FindUser(SimpleStrToStringView(user_id), ud)) {
			if (IsBS(server)) { // named conf at BS
				cnt.AddValue(DISPLAYNAME_PARAM, ud.m_displayName);
				cnt.AddValue(LOCATORBS_PARAM, ud.m_homeServer);
				cnt.AddValue(FIELD1_PARAM, m_recvMess->AddString());	// send version param to save for reply
				PostRequest(server, 0, cnt);				// send to BS
				return;
			}

			if (ConfAtRemoteVCS)
			{
				if (m_confRestriction->IsRoamingAllowed(server)) {
					dprint4("PostRequest Join to VCS %s\n", server.m_str);
					cnt.AddValue(DISPLAYNAME_PARAM, ud.m_displayName);
					PostRequest(server, 0, cnt, m_recvMess->AddString(), CONFERENCE_SRV, default_timeout, CONFERENCE_SRV, user_id);
					return;
				}
				else
				{
					dprint4("Roaming to \"%s\" is not allowed. Request will not be posted.\n", server.m_str);
					result = VSS_CONF_NOT_FOUND;
				}
			}
		}
	}

	if (found) {
		pd.m_conf_id = cd.m_name;
		pd.m_user_id = user_id;
		pd.m_server_id = (FromBS)? cnt.GetStrValueRef(SERVER_PARAM): m_recvMess->SrcServer();

		pd.m_confKey = VS_GenKeyByMD5();
		pd.m_version = atou_s(m_recvMess->AddString());
		pd.m_caps.Set(buff, size);
		pd.m_appID = app_id;
		pd.m_IsOperatorByGroups = m_confRestriction->IsOperator(pd.m_user_id);

		// has hungup
		VS_AutoInviteService::SetSupport(pd.m_conf_id, pd.m_user_id, bool(cc.GetClientFlags() & VS_ClientCaps::ClientFlags::HAS_HANGUP_FLAGS));
		// ssl stream
		if (!!cd.m_symKey) {
			if (cc.GetStreamsDC()&VSCC_STREAM_CAN_DECODE_SSL)
				pd.m_symKey = cd.m_symKey;
		}
		// svc stream
		if (!m_useDefaultStreamBuffer && cd.m_svc_mode != 0) {
			if (cc.GetStreamsDC()&VSCC_STREAM_CAN_USE_SVC)
				pd.m_svc_mode = cd.m_svc_mode;
		}

		assert(m_pios != nullptr);
		if (cd.m_type==CT_INTERCOM) {
			pd.m_type = pd.INTERCOM_MEMBER;
			fltr = VS_RcvFunc::FLTR_DEFAULT_MULTIS;
			if (!m_mcast.Init(cd.m_multicast_ip, m_pios))
				m_mcast.Init(DEFAULT_MULTICAST_IP, m_pios);
			pd.m_mcast_ip = m_mcast.GetIp();
			pd.m_port = m_mcast.AllocPort();
			if (!pd.m_port)
				result = VSS_CONF_MAX_PART_NUMBER;
			dprint3("UDP Params for %s: m=%s, p=%d\n", pd.m_user_id.m_str, pd.m_mcast_ip.m_str, pd.m_port);
		}
		else if (cd.m_type==CT_MULTISTREAM) {
			pd.m_type = pd.MULTIS_MEMBER;
			fltr = VS_RcvFunc::FLTR_DEFAULT_MULTIS;
		}
		VS_UserData ud;
		if (g_storage->FindUser(SimpleStrToStringView(user_id), ud)) { // local
			pd.m_ClientType = ud.m_client_type;
			pd.m_displayName = ud.m_displayName;
		}
		else if(display_name!=nullptr){
			pd.m_displayName = display_name;
		}
		else {
			InviteInfo info;
			if (g_storage->FindInvitation(pd.m_conf_id, pd.m_user_id, info) && !info.user_display_name.empty())
				pd.m_displayName = info.user_display_name;
		}
		// CMRFlags
		CMRFlags lic_flags = m_confRestriction->GetCMRFlagsByLicense();
		if (VS_RealUserLogin::IsGuest(SimpleStrToStringView(pd.m_user_id)))
			pd.m_CMRFlags = lic_flags & cd.m_CMR_guest;
		else
			pd.m_CMRFlags = lic_flags & cd.m_CMR_user;

	}

	if (result == 0) {
		int error_code = g_storage->AddParticipant(pd);
		if (error_code==0) {
			result = JOIN_OK;
		}
		else {
			result = error_code;
			if (pd.m_port) m_mcast.FreePort(pd.m_mcast_ip, pd.m_port);
		}
	}

	rCnt.AddValue(METHOD_PARAM, JOIN_METHOD);
	rCnt.AddValueI32(RESULT_PARAM, result);
	if (result==JOIN_OK) {
		if (!m_confRestriction->OnJoinConf(cnt, cd, user_id, FromBS, this))
			return ;
		VS_MultiLoginCapability ml_cap = VS_MultiLoginCapability::UNKNOWN;
		cnt.GetValueI32(MULTI_LOGIN_CAPABILITY_PARAM, ml_cap);
		const char* to_user = (ml_cap == VS_MultiLoginCapability::UNKNOWN ||
			ml_cap == VS_MultiLoginCapability::SINGLE_USER)? user_id.m_str: nullptr;
		if (FromBS)
		{
			VS_UserData ud;
			if (!g_storage->FindUser(from, ud))		// not local, send connect info
			{
				VS_Container rCnt;
				if (g_appServer->GetNetInfo(rCnt))
					PostRequest(cnt.GetStrValueRef(SERVER_PARAM), to_user, rCnt, 0, CONFIGURATION_SRV);
			}
			if (stream_conf_created)				// tell BS current stream_conf_id of named_conf (only once, when created)
			{
				VS_Container for_bs_cnt;
				for_bs_cnt.AddValue(METHOD_PARAM, CONFERENCECREATED_METHOD);
				for_bs_cnt.AddValue(CONFERENCE_PARAM, cd.m_name);
				for_bs_cnt.AddValue(NAME_PARAM, name);
				PostReply(for_bs_cnt);
			}
		}
		if (ConfAtOurVCS)		// send to user at VCS1, that wants to Join our registry conference
		{
			// send ConnectInfo
			VS_Container rCnt;
			if (g_appServer->GetNetInfo(rCnt))
				PostRequest(m_recvMess->SrcServer(), to_user, rCnt, 0, CONFIGURATION_SRV);
		}

		// LOG
		const char* bs_named_conf = (FromBS) ? m_recvMess->SrcServer() : 0;
		if (!bs_named_conf)
			bs_named_conf = VS_GetConfEndpoint(pd.m_conf_id.m_str);
		auto bs_user = cnt.GetStrValueRef(LOCATORBS_PARAM);
		if ((!bs_user || !*bs_user) && (VS_GetServerType(m_recvMess->SrcServer()) == ST_VCS))
			bs_user = m_recvMess->SrcServer();
		LogParticipantJoin(pd, name, bs_user, bs_named_conf);

		string_view conf_owner = cd.m_owner ? cd.m_owner.m_str : "";
		const auto trans_id_pos = conf_owner.find('/');
		const bool from_fake_client = string_view(user_id.m_str).find('/') != string_view::npos;
		if (!from_fake_client) {
			conf_owner = conf_owner.substr(0, trans_id_pos);
		}

		ConnectPart(pd, pd.m_user_id, fltr, MRVD_FORWARD);	// add myself sender

		rCnt.AddValue(CONFERENCE_PARAM, cd.m_name);			// if all ok
		rCnt.AddValue(NAME_PARAM, cd.m_topic);			// send real conference name
		rCnt.AddValue(TOPIC_PARAM, cd.m_topic);			// send real conference name
		std::string l_callid = cd.m_call_id.m_str;
		if (l_callid.find('@') == std::string::npos) {
			l_callid += "@";
			l_callid += OurEndpoint();
		}
		rCnt.AddValue(CALLID_PARAM, l_callid.c_str());		// full conference call id
		rCnt.AddValue(USERNAME_PARAM, conf_owner);
		rCnt.AddValueI32(TYPE_PARAM, cd.m_type);
		rCnt.AddValueI32(SUBTYPE_PARAM, cd.m_SubType);

		if (cd.m_public)
			rCnt.AddValueI32(SCOPE_PARAM, GS_PUBLIC);
		rCnt.AddValueI32(MAXCAST_PARAM, cd.m_MaxCast);
		rCnt.AddValueI32(MAXPARTISIPANTS_PARAM, cd.m_MaxParticipants);
		if (!!pd.m_symKey)
			rCnt.AddValue(SYMKEY_PARAM, pd.m_symKey);
		if (pd.m_svc_mode != 0)
			rCnt.AddValueI32(SVCMODE_PARAM, pd.m_svc_mode);
		rCnt.AddValueI32(LSTATUS_PARAM, cd.m_LstatusFlag);
		rCnt.AddValueI32(CMR_FLAGS_PARAM, pd.m_CMRFlags);

		if (FromBS)
			PostRequest(cnt.GetStrValueRef(SERVER_PARAM), user_id, rCnt);
		else
			PostReply(rCnt);

		ParticipantConnectsAtJoin(cd, pd, fltr);
		SendPartsList(pd, PLT_ADD);
		TakeSlot(cd, SimpleStrToStringView(pd.m_user_id));

		// send layout
		auto layout = m_confRestriction->ReadLayout(SimpleStrToStringView(cd.m_call_id), layout_for::individual, SimpleStrToStringView(pd.m_user_id));
		if (!layout.empty())
		{
			VS_Container cnt;
			cnt.AddValue(METHOD_PARAM, MANAGELAYOUT_METHOD);
			cnt.AddValue(CONFERENCE_PARAM, pd.m_conf_id);
			cnt.AddValue(TYPE_PARAM, CLT_FIXED);
			cnt.AddValue(LAYOUT_PARAM, layout);

			PostRequest(pd.m_server_id, pd.m_user_id, cnt, 0, CONFERENCE_SRV);
		}
		layout = m_confRestriction->ReadLayout(SimpleStrToStringView(cd.m_call_id), layout_for::all);
		if (!layout.empty())
		{
			SetOrInviteOnPodiumByLayout(cd, layout, pd.m_user_id, false);
		}

		auto transceiversPool = m_transceiversPool.lock();
		if (!transceiversPool) {
			dstream1 << "Error\n TransceiversPool not found!\n";
			return;
		}
		auto streamsCircuit = transceiversPool->GetTransceiverProxy(pd.m_conf_id.m_str);
		if (streamsCircuit) {
			if (auto confCtrl = streamsCircuit->ConfControl()) confCtrl->SetDisplayName(cd.m_name, user_id, pd.m_displayName.c_str());
		}

		VS_SimpleStr value;
		if (m_confRestriction->GetAppProp("lstatus_set", value) && !!value)
		{
			UpdateAllLStatuses(pd.m_conf_id, false, pd.m_user_id);		// send LStatus of all users to Joining part
		}

		ResendSlidesCommand(cd.m_name.m_str, pd.m_user_id.m_str);

		g_storage->EndInvitationProcess(cd.m_name, user_id);
	}
	else
	{
		rCnt.AddValue(CALLID_PARAM, name);				// call id sent in join
		dprint2("\t ERROR of %s Join - result = %d \n", user_id.m_str, result);
		if (FromBS)
			PostRequest(cnt.GetStrValueRef(SERVER_PARAM), user_id, rCnt);
		else
			PostReply(rCnt);
	}
}


void VS_MultiConfService::SendPartsList(VS_ParticipantDescription &pd, VS_ParticipantListType action)
{
	VS_ParticipantDescription *parts;
	int32_t NumOfPart = 0, j, id = -1;
	VS_BinBuff allparts, addpart, delpart, oldlist;

	uint32_t seq_id = g_storage->GetSeqId(pd.m_conf_id, action == PLT_UPD);

	if ((NumOfPart = g_storage->GetParticipants(pd.m_conf_id, parts)) > 0) {
		if (action == PLT_OLD || action == PLT_ADD || action == PLT_DEL) {
			// do not change this container to keep compatibility
			VS_Container rCnt;
			rCnt.AddValue(METHOD_PARAM, SENDPARTSLIST_METHOD);
			rCnt.AddValue(CONFERENCE_PARAM, pd.m_conf_id);

			for (j = 0; j < NumOfPart; j++) {
				rCnt.AddValue(USERNAME_PARAM, parts[j].m_user_id);
				rCnt.AddValueI32(ROLE_PARAM, (parts[j].m_role & 0xff) | (parts[j].m_brStatus << 8));
				rCnt.AddValue(IS_OPERATOR_PARAM, parts[j].m_IsOperatorByGroups);
				rCnt.AddValueI32(DEVICESTATUS_PARAM, parts[j].m_devStatus);
				if (id == -1 && parts[j].m_user_id == pd.m_user_id) {
					id = j;
				}
			}
			oldlist.Set(rCnt);
		}

		if (action == PLT_ADD || action == PLT_ALL) {
			VS_Container rCnt;
			rCnt.AddValue(METHOD_PARAM, SENDPARTSLIST_METHOD);
			rCnt.AddValue(TYPE_PARAM, PLT_ALL);
			rCnt.AddValue(CONFERENCE_PARAM, pd.m_conf_id);
			rCnt.AddValueI32(RESULT_PARAM, NumOfPart);
			rCnt.AddValueI32(SEQUENCE_ID_PARAM, seq_id);

			for (j = 0; j < NumOfPart; j++) {
				rCnt.AddValue(USERNAME_PARAM, parts[j].m_user_id);
				rCnt.AddValue(DISPLAYNAME_PARAM, parts[j].m_displayName);
				rCnt.AddValueI32(ROLE_PARAM, (parts[j].m_role & 0xff) | (parts[j].m_brStatus << 8));
				rCnt.AddValue(IS_OPERATOR_PARAM, parts[j].m_IsOperatorByGroups);
				rCnt.AddValueI32(DEVICESTATUS_PARAM, parts[j].m_devStatus);
				rCnt.AddValueI32(VIDEOTYPE_PARAM, parts[j].m_videoType);
				if (id == -1 && parts[j].m_user_id == pd.m_user_id) {
					id = j;
				}
			}
			allparts.Set(rCnt);
		}

		if (action == PLT_ADD || action == PLT_UPD) {
			VS_Container rCnt;
			rCnt.AddValue(METHOD_PARAM, SENDPARTSLIST_METHOD);
			rCnt.AddValue(TYPE_PARAM, action);
			rCnt.AddValue(CONFERENCE_PARAM, pd.m_conf_id);
			rCnt.AddValueI32(RESULT_PARAM, 1);
			rCnt.AddValueI32(SEQUENCE_ID_PARAM, seq_id);

			rCnt.AddValue(USERNAME_PARAM, pd.m_user_id);
			rCnt.AddValue(DISPLAYNAME_PARAM, pd.m_displayName);
			rCnt.AddValueI32(ROLE_PARAM, (pd.m_role & 0xff) | (pd.m_brStatus << 8));
			rCnt.AddValue(IS_OPERATOR_PARAM, pd.m_IsOperatorByGroups);
			rCnt.AddValueI32(DEVICESTATUS_PARAM, pd.m_devStatus);
			rCnt.AddValueI32(VIDEOTYPE_PARAM, pd.m_videoType);
			addpart.Set(rCnt);
		}

		if (action == PLT_DEL) {
			VS_Container rCnt;
			rCnt.AddValue(METHOD_PARAM, SENDPARTSLIST_METHOD);
			rCnt.AddValue(TYPE_PARAM, PLT_DEL);
			rCnt.AddValue(CONFERENCE_PARAM, pd.m_conf_id);
			rCnt.AddValueI32(RESULT_PARAM, 1);
			rCnt.AddValueI32(SEQUENCE_ID_PARAM, seq_id);

			rCnt.AddValue(USERNAME_PARAM, pd.m_user_id);
			delpart.Set(rCnt);
		}

		auto sndf = [this](VS_BinBuff *sndb, VS_ParticipantDescription & pd) {
			if (sndb && sndb->IsValid())
				PostRequest(pd.m_server_id, pd.m_user_id, sndb->Buffer(), sndb->Size(), 0, PRESENCE_SRV, 20000);
		};

		if (action == PLT_ADD) {
			for (j = 0; j < NumOfPart; j++) {
				VS_BinBuff *sndb = 0;
				if (parts[j].m_rights&parts->RIGHTS_RCV_LIST) {
					if (parts[j].m_version >= 50) {
						if (j == id)  // all list
							sndb = &allparts;
						else  // new user
							sndb = &addpart;
					}
					else
						sndb = &oldlist;
				}
				sndf(sndb, parts[j]);
			}
		}
		else if (action == PLT_DEL) {
			for (j = 0; j < NumOfPart; j++) {
				VS_BinBuff *sndb = 0;
				if (parts[j].m_rights&parts->RIGHTS_RCV_LIST)
					sndb = parts[j].m_version >= 50 ? &delpart : &oldlist;
				sndf(sndb, parts[j]);
			}
		}
		else if (action == PLT_UPD) {
			for (j = 0; j < NumOfPart; j++) {
				if (parts[j].m_rights&parts->RIGHTS_RCV_LIST)
					if (parts[j].m_version >= 50)
						sndf(&addpart, parts[j]);
			}
		}
		else if (action == PLT_ALL) {
			sndf(&allparts, pd);
		}
		else if (action == PLT_OLD) {
			sndf(&oldlist, pd);
		}

		delete[] parts;
	}
}


// set fltr for one rcv connection
void VS_MultiConfService::ConnectReciever_Method(const char* party, long fltr, const char *from)
{
	dprint4("ConnectReciever_Method %s %lx \n", party? party:"", fltr);

	if (!party || !*party)	return;

	auto user_id = m_recvMess->SrcUser_sv();
	if (user_id.empty())
	{
		if (from && *from)
			user_id = from;
		else
			return;
	}

	VS_ParticipantDescription pd;
	if (g_storage->FindParticipant(user_id, pd)) {
		ConnectPart(pd, party, fltr, MRVD_FORWARD);
	}
}

void VS_MultiConfService::ConnectServices_Method(long fltr, const char *from)
{
	dprint3("ConnectService_Method %lx \n", fltr);

	auto user_id = m_recvMess->SrcUser_sv();
	if (user_id.empty())
	{
		if (from && *from)
			user_id = from;
		else
			return;
	}

	VS_ParticipantDescription pd;

	if (g_storage->FindParticipant(user_id, pd)) {
		ConnectServices(pd, fltr);
		NotifyRole(pd);
	}
}

// set fltr for snd connections
void VS_MultiConfService::ConnectSender_Method(long fltr, const char *from)
{
	dprint3("ConnectSender_Method %lx \n", fltr);

	auto user_id = m_recvMess->SrcUser_sv();
	if (user_id.empty())
	{
		if (from && *from)
			user_id = from;
		else
			return;
	}

	VS_ParticipantDescription pd;

	if (g_storage->FindParticipant(user_id, pd)) {
		// if (RoleConf) and NumOfCast >= MaxCast
		//		reject new reporter or leader
		CheckPodiumsBeforeCast(pd.m_role, fltr, pd.m_conf_id);

		ConnectPart(pd, 0, fltr, MRVD_BACKWARD);
		UpdateMediaFiltr(pd, fltr);
		NotifyRole(pd);
	}
}

void VS_MultiConfService::CheckPodiumsBeforeCast(long role, long fltr, vs_conf_id& conf_id)
{
	if ((role != PR_LEADER && role != PR_REPORTER) || fltr == 0)
		return;
	VS_ConferenceDescription cd;
	if (!g_storage->FindConference(SimpleStrToStringView(conf_id), cd) || cd.m_SubType != GCST_ROLE)
		return;

	VS_ParticipantDescription *parts = 0;
	int NumOfPart = 0;
	unsigned long NumOfLeaders = 0;
	unsigned long NumOfLeaders_Cast = 0;
	unsigned long NumOfReport = 0;
	if ((NumOfPart = g_storage->GetParticipants(conf_id, parts)) > 0) {
		VS_ParticipantDescription **Leaders = new VS_ParticipantDescription*[NumOfPart];
		VS_ParticipantDescription **Leaders_Cast = new VS_ParticipantDescription*[NumOfPart];
		VS_ParticipantDescription **Reporters = new VS_ParticipantDescription*[NumOfPart];
		for (int i = 0; i < NumOfPart; i++) {
			if (parts[i].m_role == PR_LEADER) {
				Leaders[NumOfLeaders++] = &parts[i];
				if (parts[i].m_brStatus & ~BS_SND_PAUSED)
				{
					Leaders_Cast[NumOfLeaders_Cast++] = &parts[i];
				}
			}
			//if (parts[i].m_role == PR_PODIUM)
			//	Podium = &parts[i];
			if (parts[i].m_role == PR_REPORTER) {
				Reporters[NumOfReport++] = &parts[i];
			}
		}
		unsigned long NumOfCast = NumOfReport + NumOfLeaders_Cast;
		CheckMaxCast(NumOfCast, cd, NumOfReport, Reporters, NumOfLeaders_Cast, Leaders_Cast);
		delete[] Leaders;
		delete[] Leaders_Cast;
		delete[] Reporters;
	}
	delete[] parts;
}

void VS_MultiConfService::SetRole(VS_ParticipantDescription &pd, long role)
{
	VS_ConferenceDescription cd;
	if (!g_storage->FindConference(SimpleStrToStringView(pd.m_conf_id), cd))
		return;

	auto old_role = pd.m_role;
	pd.m_role = role;

SECUREBEGIN_D_ROLE;
	long fltr = 0;
	if (role == PR_LEADER) {
		if (pd.m_user_id == cd.m_owner || cd.m_SubType != GCST_ROLE) {// owner OR any other conf type
			fltr = VS_RcvFunc::FLTR_DEFAULT_MULTIS;
			CheckPodiumsBeforeCast(role, fltr, cd.m_name);		// check, then kick someone from podium if needed
		}
		else if (old_role == PR_REPORTER || old_role == PR_PODIUM) // OR user that casted before
			fltr = VS_RcvFunc::FLTR_DEFAULT_MULTIS;
	}
	else if (role == PR_PODIUM || role == PR_REPORTER || role == PR_EQUAL)
		fltr = VS_RcvFunc::FLTR_DEFAULT_MULTIS;
	else if (role == PR_REMARK)
		fltr = VS_RcvFunc::FLTR_RCV_STAT|VS_RcvFunc::FLTR_RCV_AUDIO;
	else if (role == PR_COMMON)
		fltr = 0;
	ConnectPart(pd, 0, fltr, MRVD_BACKWARD);
	UpdateMediaFiltr(pd, fltr); // also it calls g_storage->UpdateParticipant(pd)

	if (role == PR_LEADER || role == PR_EQUAL || role == PR_COMMON) {
		cd.m_last_roles[pd.m_user_id] = (VS_Participant_Role)role;
		g_storage->UpdateConference(cd);
	}

	// send confirmition
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, ROLEEVENT_METHOD);
	cnt.AddValue(CONFERENCE_PARAM, pd.m_conf_id);
	cnt.AddValueI32(TYPE_PARAM, RET_CONFIRM);
	cnt.AddValue(USERNAME_PARAM, pd.m_user_id);
	cnt.AddValueI32(ROLE_PARAM, role);
	PostRequest(pd.m_server_id, pd.m_user_id, cnt);

	NotifyRole(pd);
SECUREEND_D_ROLE;
}

void VS_MultiConfService::AnswerRole(VS_ParticipantDescription &pd, long role, long result, const char* from)
{
SECUREBEGIN_D_ROLE;
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, ROLEEVENT_METHOD);
	cnt.AddValue(CONFERENCE_PARAM, pd.m_conf_id);
	cnt.AddValueI32(TYPE_PARAM, RET_ANSWER);
	cnt.AddValue(USERNAME_PARAM, from ? from : pd.m_user_id.m_str);
	cnt.AddValueI32(ROLE_PARAM, role);
	cnt.AddValueI32(RESULT_PARAM, result);
	PostRequest(pd.m_server_id, pd.m_user_id, cnt);
SECUREEND_D_ROLE;
}

void VS_MultiConfService::NotifyRole(VS_ParticipantDescription &pd)
{
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, ROLEEVENT_METHOD);
	cnt.AddValue(CONFERENCE_PARAM, pd.m_conf_id);
	cnt.AddValueI32(TYPE_PARAM, RET_NOTIFY);
	cnt.AddValue(USERNAME_PARAM, pd.m_user_id);

	cnt.AddValueI32(ROLE_PARAM, (pd.m_role&0xff)|(pd.m_brStatus<<8));
	cnt.AddValue(IS_OPERATOR_PARAM, pd.m_IsOperatorByGroups);

	VS_BinBuff buff; buff.Set(cnt);
	if (!buff.Buffer())
		return;

	std::vector<tPartServer> part_serv;
	g_storage->GetParticipants(pd.m_conf_id, part_serv);
	for (auto &i : part_serv)
		PostRequest(i.second, i.first, buff.Buffer(), buff.Size());
	LogParticipantRole(pd);
}


void VS_MultiConfService::RoleEvent_Method(VS_Container &cnt)
{
	VS_RoleEvent_Type EventType = RET_INQUIRY;
	cnt.GetValueI32(TYPE_PARAM, EventType);
	VS_Participant_Role Role = PR_COMMON;
	cnt.GetValueI32(ROLE_PARAM, Role);
	VS_RoleInqury_Answer Result = RIA_POSITIVE;
	cnt.GetValueI32(RESULT_PARAM, Result);
	const auto name = cnt.GetStrValueView(USERNAME_PARAM);
	const auto conf = cnt.GetStrValueView(CONFERENCE_PARAM);
	dstream3 << "\t type=" << EventType << ", Role=" << Role << ", name=" << name << ", r=" << Result << ", conf=" << conf;

	// todo(kt): TEST!!!
//	Role = PR_LEADER;

	VS_ConferenceDescription cd;
	if (!g_storage->FindConference(conf, cd)) {
		// TODO trace
		return;
	}

	VS_ParticipantDescription pd;
	if (!g_storage->FindParticipant(name, pd)) {
		// TODO trace
		return;
	}

	unsigned long NumOfPart = 0;
	unsigned long NumOfLeaders = 0;
	unsigned long NumOfLeaders_Cast = 0;
	unsigned long NumOfReport = 0;


	VS_ParticipantDescription *parts = 0;
	VS_ParticipantDescription * Podium = 0;
	VS_ParticipantDescription * Remark = 0;

	if ((NumOfPart = g_storage->GetParticipants(cd.m_name, parts)) < 0)
		return;

	VS_ParticipantDescription **Leaders = new VS_ParticipantDescription*[NumOfPart];
	VS_ParticipantDescription **Leaders_Cast = new VS_ParticipantDescription*[NumOfPart];
	VS_ParticipantDescription **Reporters = new VS_ParticipantDescription*[NumOfPart];

	for (auto i = 0u; i < NumOfPart; i++) {
		if (parts[i].m_role == PR_LEADER) {
			Leaders[NumOfLeaders++] = &parts[i];
			if (parts[i].m_brStatus & ~BS_SND_PAUSED)
			{
				Leaders_Cast[NumOfLeaders_Cast++] = &parts[i];
			}
		}
		if (parts[i].m_role == PR_REPORTER) {
			Reporters[NumOfReport++] = &parts[i];
		}
		if (parts[i].m_role == PR_REMARK)
			Remark = &parts[i];
		if (parts[i].m_role == PR_PODIUM)
			Podium = &parts[i];
	}
	unsigned long NumOfCast = NumOfReport + NumOfLeaders_Cast;


SECUREBEGIN_D_ROLE;

if (EventType==RET_INQUIRY) {
		if (Role==PR_COMMON || Role==PR_EQUAL) { // release role
			const char * srcuser = m_recvMess->SrcUser();
			if (srcuser) {
				bool SetOldLeaderAsNormal(false);
				if ((pd.m_user_id == srcuser) &&
					!pd.m_IsOperatorByGroups)
				{
					AnswerRole(pd, Role, RIA_POSITIVE);
					SetRole(pd, Role);
					SetOldLeaderAsNormal = true;
				}
				else {
					// query from owner
					VS_ParticipantDescription spd;
					if ((g_storage->FindParticipant(srcuser, spd) && spd.m_role==PR_LEADER) &&
						!pd.m_IsOperatorByGroups)
					{
						AnswerRole(spd, Role, RIA_POSITIVE, pd.m_user_id);
						SetRole(pd, Role);
						SetOldLeaderAsNormal = true;
					}
				}
				if (SetOldLeaderAsNormal && (cd.m_SubType == GCST_ALL_TO_OWNER))
				{
					if (m_moderator_asymmetric_algo == e_many_leaders) {
						VS_ParticipantDescription& old_leader = pd;
						for (unsigned int j = 0; j < NumOfPart; ++j)
						{
							// disconnect old leader's receiver from everyone, except new leader
							char p[512] = { 0 };
							VS_SimpleStr tmp1, tmp2;
							if (parts[j].m_user_id != old_leader.m_user_id && 		// exclude me
								parts[j].m_role != PR_LEADER)						// exclude current leaders
							{
								VS_RcvFunc::SetName(p, old_leader.m_user_id, parts[j].m_user_id);
								m_sr->RemoveParticipant(cd.m_name.m_str, p);
								StreamPartRemoved(cd.m_name, p, tmp1, tmp2);

								VS_RcvFunc::SetName(p, parts[j].m_user_id, old_leader.m_user_id);
								m_sr->RemoveParticipant(cd.m_name.m_str, p);
								StreamPartRemoved(cd.m_name, p, tmp1, tmp2);
							}
						}
					}
				}
			}
		}
		else if (Role == PR_PODIUM) {
			// request from part to go on podium
			if (NumOfLeaders) {
				// todo(kt): add id for future cancel pending requests
				for (auto j = 0u; j < NumOfLeaders; ++j)
					PostRequest(Leaders[j]->m_server_id, Leaders[j]->m_user_id, cnt);
			}
			else {
				AnswerRole(pd, Role, RIA_PARTICIPANT_ABSENT, cd.m_owner);
			}
		}
		else if (Role == PR_REPORTER) {
			// leader send query to part
			if (cd.m_MaxCast > 0) // always send request
				PostRequest(pd.m_server_id, pd.m_user_id, cnt);
			else {
				if (NumOfLeaders)
					AnswerRole(*(Leaders[0]), Role, RIA_COMMON, 0); //TODO:  or from=user ?		// todo(kt): set first leader as common?
			}
		}
		else if (Role == PR_REMARK) {
			// request from part to go to "replica"
			if (!Remark) {
				AnswerRole(pd, Role, RIA_POSITIVE);
				SetRole(pd, Role);
			}
			else {
				AnswerRole(pd, Role, RIA_ROLE_BUSY);
			}
		}
		else if (Role == PR_LEADER) {
			// if src is leader, then send to dst, else send to leaders
			bool i_am_leader(false);
			for (auto j = 0u; j < NumOfLeaders; ++j)
			{
				if (Leaders[j]->m_user_id == m_recvMess->SrcUser())
					i_am_leader = true;
			}

			if (i_am_leader) {
				// leader send query to part
				PostRequest(pd.m_server_id, pd.m_user_id, cnt);
			}

			// todo(kt): this part doesn't work correctly
			//else {
			//	// request from part to go on podium
			//	if (NumOfLeaders) {
			//		if (cd.m_MaxCast > 1)
			//			for (int j = 0; j < NumOfLeaders; ++j)
			//				PostRequest(Leaders[j]->m_server_id, Leaders[j]->m_user_id, cnt);
			//		else
			//			AnswerRole(pd, Role, RIA_ROLE_BUSY, m_recvMess->SrcUser());
			//	}
			//	else {
			//		AnswerRole(pd, Role, RIA_PARTICIPANT_ABSENT, cd.m_owner);
			//	}
			//}
		}
	}
	else if (EventType==RET_ANSWER) {
		if (Role == PR_REPORTER) {
			// answer from part
			if (NumOfLeaders) {
				// reply to leader
				for (unsigned int j = 0; j < NumOfLeaders; ++j)
					PostRequest(Leaders[j]->m_server_id, Leaders[j]->m_user_id, cnt);
				if (Result == RIA_POSITIVE) {
					CheckMaxCast(NumOfCast, cd, NumOfReport, Reporters, NumOfLeaders_Cast, Leaders_Cast);
					SetRole(pd, Role);
				}
			}
			else {
				// without leader dont allow set REPORTER role
			}
		}
		else if (Role == PR_PODIUM) {
			// answer from leader
			VS_ParticipantDescription* i_am_leader(0);
			for (auto j = 0u; j < NumOfLeaders; ++j)
			{
				if (Leaders[j]->m_user_id == m_recvMess->SrcUser())
					i_am_leader = Leaders[j];
			}

			if (i_am_leader) {

				// todo(kt): cancel all pending requests with id

				AnswerRole(pd, Role, Result, i_am_leader->m_user_id);
				if (Result == RIA_POSITIVE) {
					CheckMaxCast(NumOfCast, cd, NumOfReport, Reporters, NumOfLeaders_Cast, Leaders_Cast);
					SetRole(pd, PR_REPORTER); //hack
				} else {
					NotifyRole(pd);
				}
			}
			else {
				// who is there?
			}
		}
		else if (Role == PR_LEADER) {
			// answer from part
			if (NumOfLeaders) {
				// reply to leaders
				for (unsigned int j = 0; j < NumOfLeaders; ++j)
					PostRequest(Leaders[j]->m_server_id, Leaders[j]->m_user_id, cnt);
				if (Result == RIA_POSITIVE) {
					SetRole(pd, Role);

					// kt: set old leader as normal if asymmetric conf
					if (cd.m_SubType == GCST_ALL_TO_OWNER)
					{
						if (m_moderator_asymmetric_algo == e_one_leader) {
							VS_ParticipantDescription& old_leader = *(Leaders[0]);
							SetRole(old_leader, PR_EQUAL);

							for (unsigned int j = 0; j < NumOfPart; ++j)
							{
								// disconnect old leader's receiver from everyone, except new leader
								char p[512] = { 0 };
								VS_SimpleStr tmp1, tmp2;
								if ((parts[j].m_user_id != old_leader.m_user_id) &&		// exclude me
									(parts[j].m_user_id != pd.m_user_id))				// exclude new leader
								{
									VS_RcvFunc::SetName(p, old_leader.m_user_id, parts[j].m_user_id);
									m_sr->RemoveParticipant(cd.m_name.m_str, p);
									StreamPartRemoved(cd.m_name, p, tmp1, tmp2);

									VS_RcvFunc::SetName(p, parts[j].m_user_id, old_leader.m_user_id);
									m_sr->RemoveParticipant(cd.m_name.m_str, p);
									StreamPartRemoved(cd.m_name, p, tmp1, tmp2);
								}
							}

							for (unsigned int j = 0; j < NumOfPart; ++j)
							{
								ConnectPart(parts[j], pd.m_user_id, VS_RcvFunc::FLTR_DEFAULT_MULTIS, MRVD_FORWARD);
								ConnectPart(parts[j], pd.m_user_id, VS_RcvFunc::FLTR_DEFAULT_MULTIS, MRVD_BACKWARD);
							}

							// save as owner of conf (for KICK)
							//cd.m_owner = name;
							//g_storage->UpdateConference(cd);
						}
						else if (m_moderator_asymmetric_algo == e_many_leaders) {
							for (unsigned int j = 0; j < NumOfPart; ++j)
							{
								ConnectPart(parts[j], pd.m_user_id, VS_RcvFunc::FLTR_DEFAULT_MULTIS, MRVD_FORWARD);
								ConnectPart(parts[j], pd.m_user_id, VS_RcvFunc::FLTR_DEFAULT_MULTIS, MRVD_BACKWARD);
							}
						}
					}
				}
			}
			else {
				// without leader dont allow set REPORTER role
			}
		} // end of leader
	} // end of inquery/answer
SECUREEND_D_ROLE;
	delete[] Leaders;
	delete[] Leaders_Cast;
	delete[] Reporters;
	delete[] parts;
}

void VS_MultiConfService::FECC_Method(VS_Container &cnt)
{
	int32_t type;		cnt.GetValue(TYPE_PARAM, type);
	const char *pfrom = cnt.GetStrValueRef(FROM_PARAM);
	const char *pto = cnt.GetStrValueRef(TO_PARAM);
	if (!pfrom || !*pfrom || !pto || !*pto)
		return;

	std::string from = pfrom, to = pto;

	dprint4("FECC_Method: type=%d, from=%s, to=%s", type, from.c_str(), to.c_str());

	switch (eFeccRequestType(type)) {
		case eFeccRequestType::GET_STATE: {
			VS_ParticipantDescription pd;
			if (g_storage->FindParticipant(from, pd))
			{
				VS_ConferenceDescription cd;
				if ((g_storage->FindConference(SimpleStrToStringView(pd.m_conf_id), cd) && cd.m_type != CT_MULTISTREAM && cd.m_type != CT_INTERCOM) ||	// p2p
					(pd.m_role == PR_LEADER))
				cnt.AddValue(IS_OPERATOR_PARAM, (bool)true);
			}
			m_fecc_pending[to].emplace(from, std::make_pair(eFeccRequestType::GET_STATE, std::chrono::steady_clock::now()));
		} break;
		case eFeccRequestType::SET_STATE: {
			if (strcasecmp(from.c_str(), to.c_str()) != 0)
				return;

			for (auto& p : m_fecc_pending[from]) {
				if (p.second.first == eFeccRequestType::GET_STATE)
					PostRequest(nullptr, p.first.c_str(), cnt);
			}
		} break;
		case eFeccRequestType::MY_STATE:

		case eFeccRequestType::REQUEST_ACCESS:
			m_fecc_pending[to].emplace(from, std::make_pair(eFeccRequestType::REQUEST_ACCESS, std::chrono::steady_clock::now()));
			break;
		case eFeccRequestType::ALLOW_ACCESS:
		case eFeccRequestType::DENY_ACCESS:
		case eFeccRequestType::DENY_BY_TIMEOUT_ACCESS:
		case eFeccRequestType::UP:
		case eFeccRequestType::DOWN:
		case eFeccRequestType::LEFT:
		case eFeccRequestType::RIGHT:
		case eFeccRequestType::ZOOM_IN:
		case eFeccRequestType::ZOOM_OUT:
		case eFeccRequestType::FOCUS_IN:
		case eFeccRequestType::FOCUS_OUT:
		case eFeccRequestType::USE_PRESET:
		case eFeccRequestType::SAVE_PRESET:

		case eFeccRequestType::HOME:
		case eFeccRequestType::POSITION:
			break;
	}

	if (eFeccRequestType(type) != eFeccRequestType::SET_STATE)
		PostRequest(nullptr, to.c_str(), cnt);
}

void VS_MultiConfService::QueryParticipants_Method(VS_Container & cnt)
{
	VS_ParticipantDescription pd;
	if (!g_storage->FindParticipant(m_recvMess->SrcUser_sv(), pd))
		return;

	VS_AddressBook abtype = AB_NONE;
	cnt.GetValueI32(ADDRESSBOOK_PARAM,  abtype);
	const char *conf = cnt.GetStrValueRef(CONFERENCE_PARAM);
	if (abtype == AB_PUBCONF && pd.m_conf_id == conf)
		SendPartsList(pd, PLT_ALL);
}

void VS_MultiConfService::CheckMaxCast(unsigned long NumOfCast, VS_ConferenceDescription &cd, unsigned long NumOfReport, VS_ParticipantDescription ** Reporters, unsigned long NumOfLeaders_Cast, VS_ParticipantDescription ** Leaders_Cast)
{
	if (cd.m_SubType != GCST_ROLE)
		return;
	if (NumOfCast >= cd.m_MaxCast) {
		// all slots are busy - reject one slot
		/*
		// reject one old slot
		const FILETIME ftt = {~0, ~0};
		VS_FileTime ltime(&ftt);
		unsigned long j = 0, lowest = ~0;
		for (; j<NumOfReport; j++) {
		if (Reporters[j]->m_roleTime < ltime) {
		ltime = Reporters[j]->m_roleTime;
		lowest = j;
		}
		}*/
		// reject one new slot
		if (NumOfReport)
		{
			std::chrono::system_clock::time_point ltime;
			unsigned long j = 0, lowest = ~0;
			for (; j < NumOfReport; j++) {
				if (Reporters[j]->m_podiumTime > ltime) {
					ltime = Reporters[j]->m_podiumTime;
					lowest = j;
				}
			}
			if (lowest != ~0) {
				dprint3("MaxCast: free podium of %s", Reporters[lowest]->m_user_id.m_str);
				SetRole(*Reporters[lowest], PR_COMMON);
			}
		}
		else if (NumOfLeaders_Cast) {		// reject one new slot from Leaders
			std::chrono::system_clock::time_point ltime;
			unsigned long j = 0, lowest = ~0;
			for (; j < NumOfLeaders_Cast; j++) {
				if (Leaders_Cast[j]->m_podiumTime > ltime) {
					ltime = Leaders_Cast[j]->m_podiumTime;
					lowest = j;
				}
			}
			if (lowest != ~0) {	// remove user from casting
				VS_ParticipantDescription& pd = *Leaders_Cast[lowest];
				if (cd.m_owner == pd.m_user_id || pd.m_role == PR_LEADER) {		// owner/moderator just do not cast, not change role to PR_EQUAL
					dprint3("MaxCast: change casting state of leader %s", pd.m_user_id.m_str);
					ConnectPart(pd, 0, 0, MRVD_BACKWARD);
					UpdateMediaFiltr(pd, 0);
					NotifyRole(pd);
				}
				else
					SetRole(pd, PR_COMMON);
			}
		}
	}
}

void VS_MultiConfService::ParticipantConnectsAtJoin(VS_ConferenceDescription& cd, VS_ParticipantDescription &pd, long fltr)
{
	bool IsOwnerOrOperator = ((pd.m_user_id == cd.m_owner) || pd.m_IsOperatorByGroups) ? true : false;
	boost::optional<VS_Participant_Role> last_role;
	if (!IsOwnerOrOperator)
	{
		if (cd.m_moderators.find(pd.m_user_id) != cd.m_moderators.end())
			last_role = PR_LEADER;
		auto it = cd.m_last_roles.find(pd.m_user_id);
		if (it != cd.m_last_roles.end())
			last_role = it->second;
	}

	bool do_notify(false);
	if (cd.m_SubType != GCST_ROLE && (IsOwnerOrOperator || last_role.is_initialized())) {
		pd.m_role = (IsOwnerOrOperator) ? PR_LEADER : (last_role.is_initialized()) ? *last_role : PR_LEADER;
		g_storage->UpdateParticipant(pd);
		do_notify = true;
	}

	if (cd.m_SubType==GCST_PENDING_ALL_TO_OWNER) {
		if (IsOwnerOrOperator) {
			ConnectPart(pd, NULL, fltr, MRVD_BACKWARD);				// connect all parts with owner
			UpdateMediaFiltr(pd, fltr);
		}
		else
			ConnectPart(pd, cd.m_owner, fltr, MRVD_FORWARD);		// connect part only with owner
	}
	else if (cd.m_SubType==GCST_ROLE) {
		ConnectPart(pd, 0, fltr, MRVD_FORWARD);
		auto role = IsOwnerOrOperator ? PR_LEADER : (last_role.is_initialized()) ? *last_role : PR_COMMON;
		SetRole(pd, role);
		return;
	}
	else {
		ConnectPart(pd, 0, fltr, MRVD_FORWARD|MRVD_BACKWARD);	// cross-connection
		UpdateMediaFiltr(pd, fltr);
	}

	if (do_notify)
		NotifyRole(pd);
}

void VS_MultiConfService::SetOrInviteOnPodiumByLayout(VS_ConferenceDescription& cd, const std::string& layout, const char* part, bool invite)
{
	if (cd.m_SubType != GCST_ROLE)
		return;

	// call on podium. Find userid in json layout. if part is empty apply podium for all
	json::Object obj;
	std::stringstream ss(layout);
	try {
		json::Reader::Read(obj, ss);
		json::Array slots = obj[layout_json::slots];
		for (auto it = slots.Begin(); it != slots.End(); ++it) {
			json::Object slot = *it;
			std::string id = json::String(slot[layout_json::instance]);
			if (!part || !*part || id == part) {
				VS_ParticipantDescription pd;
				if (g_storage->FindParticipant(id, pd)) {
					auto fltr = VS_RcvFunc::FLTR_DEFAULT_MULTIS;
					if (pd.m_role == PR_COMMON || pd.m_role == PR_REMARK) {
						if (invite) {
							RequestReporterRole(pd);
						}
						else {
							CheckPodiumsBeforeCast(PR_REPORTER, fltr, cd.m_name);
							SetRole(pd, PR_REPORTER);
						}
					}
					else if (pd.m_role == PR_LEADER && (pd.m_brStatus & BS_SND_PAUSED)) {
						CheckPodiumsBeforeCast(pd.m_role, fltr, cd.m_name);
						ConnectPart(pd, 0, fltr, MRVD_BACKWARD);
						UpdateMediaFiltr(pd, fltr);
						NotifyRole(pd);
					}
				}
			}
		}
	}
	catch (json::Exception &) {
	}
}

void VS_MultiConfService::RequestReporterRole(VS_ParticipantDescription & pd)
{
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, ROLEEVENT_METHOD);
	rCnt.AddValue(CONFERENCE_PARAM, pd.m_conf_id);
	rCnt.AddValueI32(TYPE_PARAM, RET_INQUIRY);
	rCnt.AddValue(USERNAME_PARAM, pd.m_user_id);
	rCnt.AddValueI32(ROLE_PARAM, PR_REPORTER);

	PostRequest(pd.m_server_id, pd.m_user_id, rCnt);
}

void VS_MultiConfService::TakeSlot(const VS_ConferenceDescription& cd, string_view part_instance)
{
	auto part_hunt = VS_RemoveTranscoderID_sv(part_instance);
	std::string part_hunt_unescaped;
	std::unique_ptr<CURL, CURL_deleter> curl(::curl_easy_init());
	if (curl) {
		int new_len = 0;
		std::unique_ptr<char, curl_free_deleter> unescaped(::curl_easy_unescape(curl.get(), part_hunt.data(), part_hunt.length(), &new_len));
		if (unescaped)
			part_hunt_unescaped = unescaped.get();
	}
	std::string part_hunt_unescaped_no_slashes;
	if (VS_IsRTSPCallID(part_hunt_unescaped) && part_hunt_unescaped.length() >= 8 && part_hunt_unescaped[6] == '/' && part_hunt_unescaped[7] == '/')
		part_hunt_unescaped_no_slashes.append("#rtsp:").append(part_hunt_unescaped, 8, std::string::npos);

	std::array<string_view, 3> ids{
		part_hunt,
		part_hunt_unescaped,
		part_hunt_unescaped_no_slashes,
	};

	auto take_slot = [&](layout_for f, const VS_ParticipantDescription& layout_for_user) {
		auto layout = m_confRestriction->ReadLayout(SimpleStrToStringView(cd.m_call_id), f, SimpleStrToStringView(layout_for_user.m_user_id));
		if (layout.empty())
			return;
		rj::Document doc;
		if (doc.Parse(layout.c_str()).GetParseError() != rj::kParseErrorNone || !doc.IsObject())
			return;
		auto slots = doc.FindMember(layout_json::slots);
		if (slots == doc.MemberEnd() || !slots->value.IsArray())
			return;
		bool changed = false;
		for (auto&& arr : slots->value.GetArray())
		{
			if (!arr.IsObject())
				continue;
			auto slot = arr.GetObject();
			string_view id;
			auto x = slot.FindMember(layout_json::id);
			if (x != slot.MemberEnd() && !x->value.IsNull())
				id = string_view(x->value.GetString(), x->value.GetStringLength());
			if (id.empty() || std::find(std::begin(ids), std::end(ids), id) == ids.end())
				continue;
			auto instance_it = slot.FindMember(layout_json::instance);
			if (instance_it != slot.MemberEnd() && instance_it->value.GetStringLength() > 0) // occupied slot
				continue;
			if (instance_it != slot.MemberEnd()) {
				instance_it->value.SetString(part_instance.data(), part_instance.length());
			}
			else {
				slot.AddMember(layout_json::instance,
					rj::Value().SetString(part_instance.data(), part_instance.length(), doc.GetAllocator()), doc.GetAllocator());
			}
			changed = true;
		}
		if (changed)
		{
			rj::StringBuffer buffer;
			rj::Writer<rj::StringBuffer> writer(buffer);
			doc.Accept(writer);
			std::string new_layout = buffer.GetString();

			MC_SetLayout(SimpleStrToStringView(cd.m_name), layout_for_user, f, new_layout);
		}
	};

	take_slot(layout_for::all, vs::ignore<VS_ParticipantDescription>());
	take_slot(layout_for::mixer, vs::ignore<VS_ParticipantDescription>());


	auto v = m_confRestriction->GetUsersWithIndividualLayouts(SimpleStrToStringView(cd.m_call_id));
	for (auto const& i : v)
	{
		VS_ParticipantDescription pd;
		if (!g_storage->FindParticipant(i, pd))
			pd.m_user_id = StringViewToSimpleStr(i);
		take_slot(layout_for::individual, pd);
	}
}

void VS_MultiConfService::FreeSlot(const VS_ConferenceDescription& cd, string_view part_instance)
{
	auto free_slot = [&](layout_for f, const VS_ParticipantDescription& layout_for_user) {
		auto layout = m_confRestriction->ReadLayout(SimpleStrToStringView(cd.m_call_id), f, SimpleStrToStringView(layout_for_user.m_user_id));
		if (layout.empty())
			return;
		rj::Document doc;
		if (doc.Parse(layout.c_str()).GetParseError() != rj::kParseErrorNone || !doc.IsObject())
			return;
		auto slots = doc.FindMember(layout_json::slots);
		if (slots == doc.MemberEnd() || !slots->value.IsArray())
			return;
		bool changed = false;
		for (auto&& arr : slots->value.GetArray())
		{
			if (!arr.IsObject())
				continue;
			auto slot = arr.GetObject();
			auto instance = slot.FindMember(layout_json::instance);
			if (instance == slot.MemberEnd())
				continue;
			string_view sv(instance->value.GetString(), instance->value.GetStringLength());
			if (sv == part_instance)
			{
				slot.RemoveMember(instance);
				changed = true;
			}
		}

		if (changed)
		{
			rj::StringBuffer buffer;
			rj::Writer<rj::StringBuffer> writer(buffer);
			doc.Accept(writer);
			std::string new_layout = buffer.GetString();

			MC_SetLayout(SimpleStrToStringView(cd.m_name), layout_for_user, f, new_layout);
		}
	};

	free_slot(layout_for::all, vs::ignore<VS_ParticipantDescription>());
	free_slot(layout_for::mixer, vs::ignore<VS_ParticipantDescription>());

	VS_ParticipantDescription* parts = nullptr;
	VS_SCOPE_EXIT{ delete[] parts; };
	auto NumOfPart = g_storage->GetParticipants(cd.m_name, parts);
	for (auto j = 0; j < NumOfPart; j++)
		free_slot(layout_for::individual, parts[j]);
}

long VS_MultiConfService::CheckParticipantsConnects(VS_ConferenceDescription& cd,
													VS_ParticipantDescription &to, VS_ParticipantDescription &from, long fltr)
{
	if (cd.m_SubType==GCST_FULL) {
	}
	else if (cd.m_SubType==GCST_ALL_TO_OWNER || cd.m_SubType==GCST_PENDING_ALL_TO_OWNER) {
		if (cd.m_owner != to.m_user_id && cd.m_owner != from.m_user_id &&
			to.m_role != PR_LEADER && from.m_role != PR_LEADER) {
			fltr = 0;
		}
	}
	else if (cd.m_SubType==GCST_ROLE) {
		if (from.m_role==PR_LEADER || from.m_role==PR_PODIUM || from.m_role==PR_REPORTER) {
		}
		else if (from.m_role==PR_REMARK) {
			fltr&=~VS_RcvFunc::FLTR_RCV_VIDEO;
		}
		else {
			fltr = 0;
		}
	}
	if (from.m_brStatus&BS_SND_PAUSED)
		fltr = 0;
	if (fltr!=0) {
		if (from.m_brStatus&BS_TRACK5_ON)
			fltr|= VS_RcvFunc::FLTR_RCV_DATA;
		else
			fltr&=~VS_RcvFunc::FLTR_RCV_DATA;
	}
	return fltr;
}


void VS_MultiConfService::DeviceStatus_Method(VS_Container &cnt)
{
	int32_t DeviceStatus = DVS_SND_UNKNOWN;

	if (cnt.GetValue(DEVICESTATUS_PARAM, DeviceStatus)) {
		VS_ParticipantDescription pd;
		if (g_storage->FindParticipant(cnt.GetStrValueView(USERNAME_PARAM), pd)) {
			VS_BinBuff buff; buff.Set(cnt);
			std::vector<tPartServer> part_serv;
			g_storage->GetParticipants(pd.m_conf_id, part_serv);
			if (buff.Buffer())
				for (auto &i : part_serv)
					PostRequest(i.second, i.first, buff.Buffer(), buff.Size());

			pd.m_devStatus = DeviceStatus;
			g_storage->UpdateParticipant(pd);
		}
	}
}


// send message to user
void VS_MultiConfService::ReceiverConnected(VS_StreamPartDesc &spd, long result, const char* server, const char* user)
{
	dprint3("RcvConn %s =%ld, par: %s, %s, %ld, %ld \n", spd.m_name.m_str, result, spd.m_mcast_ip.m_str, spd.m_host.m_str, spd.m_port, spd.m_confKey);

	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, RECEIVERCONNECTED_METHOD);
	cnt.AddValueI32(TYPE_PARAM, spd.m_type);
	cnt.AddValue(USERNAME_PARAM, spd.m_rcv);
	cnt.AddValue(CONFERENCE_PARAM, spd.m_conf);
	cnt.AddValueI32(RESULT_PARAM, result);
	cnt.AddValueI32(MEDIAFILTR_PARAM, spd.m_fltr);
	cnt.AddValueI32(DEVICESTATUS_PARAM, spd.m_dvs);
	if (spd.m_caps.IsValid())
		cnt.AddValue(CLIENTCAPS_PARAM, spd.m_caps.Buffer(), spd.m_caps.Size());
	// ssl streams
	if (!!spd.m_sym_key)
		cnt.AddValue(SYMKEY_PARAM, spd.m_sym_key);

	if		(spd.m_type == RCT_ROUTER) {
		cnt.AddValue(NAME_PARAM, spd.m_name);
	}
	else if (spd.m_type == RCT_INTERCOM) {
		cnt.AddValue(MULTICAST_IP_PARAM, spd.m_mcast_ip);
		cnt.AddValue(HOST_PARAM, spd.m_host);
		cnt.AddValueI32(PORT_PARAM, spd.m_port);
		cnt.AddValueI32(KEY_PARAM, spd.m_confKey);
	}
	PostRequest(server, user, cnt, 0, CONFERENCE_SRV);

#if 0 // old code, set media filtr to 0, if listeners num = 0;
	// notify reseiver about his listener
	VS_Map::Iterator i;
	i = m_parts.Find(spd.m_rcv);
	if (i!=m_parts.End()) {
		VS_ParticipantDescription pd;
		VS_Map* sPartMap = (VS_Map*)(*i).data;
		if (g_storage->FindParticipant(spd.m_rcv, pd)) {
			long fltr = 0;
			for (i = sPartMap->Begin(); i!=sPartMap->End(); ++i) {
				VS_StreamPartDesc* pspd = (VS_StreamPartDesc*)(*i).data;
				if (pspd->m_snd!=spd.m_rcv) {
					fltr|=pspd->m_fltr;
					dprint4("listener %-30s fltr = %x \n", pspd->m_snd, pspd->m_fltr);
				}
			}
			if (pd.m_lfltr!=fltr) {
				pd.m_lfltr = fltr;
				g_storage->UpdateParticipant(pd);
				dprint3("Listeners Fltr of %s is changed to %x\n", spd.m_rcv, fltr);
				cnt.Clear();
				cnt.AddValue(METHOD_PARAM, LISTENERSFLTR_METHOD);
				cnt.AddValue(CONFERENCE_PARAM, spd.m_conf);
				cnt.AddValue(MEDIAFILTR_PARAM, fltr);
				PostRequest(pd.m_server_id, pd.m_user_id, cnt, 0, CONFERENCE_SRV);
			}
		}
	}
#endif
}

void VS_MultiConfService::UpdateMediaFiltr(VS_ParticipantDescription &pd, long fltr)
{
	pd.m_lfltr = fltr;
	if (fltr & VS_RcvFunc::FLTR_ALL_MEDIA)
		pd.m_podiumTime = std::chrono::system_clock::now();
	g_storage->UpdateParticipant(pd);
	dprint3("Listeners Fltr of %s is changed to %lx\n", pd.m_user_id.m_str, fltr);

	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, LISTENERSFLTR_METHOD);
	cnt.AddValue(CONFERENCE_PARAM, pd.m_conf_id);
	cnt.AddValueI32(MEDIAFILTR_PARAM, fltr);
	PostRequest(pd.m_server_id, pd.m_user_id, cnt, 0, CONFERENCE_SRV);

	auto transceiversPool = m_transceiversPool.lock();
	if (!transceiversPool) {
		dstream1 << "Error\n TransceiversPool not found!\n";
		return;
	}
	auto streamsCircuit = transceiversPool->GetTransceiverProxy(pd.m_conf_id.m_str);
	if (!streamsCircuit) return;
	if (auto confCtrl = streamsCircuit->ConfControl()) confCtrl->UpdateFilter(pd.m_conf_id, pd.m_user_id, fltr, pd.m_role);
}


// send message to user
void VS_MultiConfService::SenderConnected(VS_StreamPartDesc &spd, long result, const char* server, const char* user)
{
	dprint3("SndConn %s =%ld, par: %s, %s, %ld, %ld \n", spd.m_name.m_str, result, spd.m_mcast_ip.m_str, spd.m_host.m_str, spd.m_port, spd.m_confKey);
	// only for new intercom part
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, SENDERCONNECTED_METHOD);
	cnt.AddValue(USERNAME_PARAM, spd.m_snd);
	cnt.AddValue(CONFERENCE_PARAM, spd.m_conf);
	cnt.AddValueI32(RESULT_PARAM, result);
	cnt.AddValue(MULTICAST_IP_PARAM, spd.m_mcast_ip);
	cnt.AddValue(HOST_PARAM, spd.m_host);
	cnt.AddValueI32(PORT_PARAM, spd.m_port);
	cnt.AddValueI32(KEY_PARAM, spd.m_confKey);
	PostRequest(server, user, cnt, 0, CONFERENCE_SRV);
}

// create new receiver coonect beetween two parts (p1<-p2)
int  VS_MultiConfService::NewPartConnect(
	VS_StreamPartDesc &spd,
	VS_ParticipantDescription &pd1,
	VS_ParticipantDescription &pd2,
	long type, long fltr)
{
	char name[512]; *name = 0;
	VS_RcvFunc::SetName(name, pd1.m_user_id, pd2.m_user_id);

	spd.m_name = name;
	spd.m_snd = pd1.m_user_id;
	spd.m_rcv = pd2.m_user_id;
	spd.m_conf = pd1.m_conf_id;
	spd.m_dvs = pd2.m_devStatus;
	spd.m_fltr = VS_CheckFmt(pd1.m_caps, pd2.m_caps, fltr);
	VS_SetRcvDevStatus(spd.m_dvs, fltr, spd.m_fltr, 1);
	spd.m_caps = pd2.m_caps;
	// ssl streams
	if (!!pd2.m_symKey)
		if (!pd1.m_symKey)
			fltr = 0;
		else
			spd.m_sym_key = pd2.m_symKey;

	bool dyn_change_cap = VS_CheckDynChange(pd1.m_caps);
	unsigned char level = VS_GetLevelPerformance(pd1.m_caps);
	int sndMBps = VS_GetSndMBps(pd2.m_caps);
	int sndFrameSizeMB = VS_GetSndFrameSizeMB(pd2.m_caps);

	if (fltr==0) return SCR_DISCONNECTED_OK;

	dprint4("NewPartConnect %s type %ld \n", spd.m_name.m_str, type);
	stream::Track tracks[4];
	unsigned nTracks;
	int result = SCR_FAILED;

	if (type == CT_MULTISTREAM){
		spd.m_type = RCT_ROUTER;
		stream::Buffer* strb;
		if (m_useDefaultStreamBuffer)
			strb = new stream::DefaultBuffer;
		else
			strb = new stream::SVCBuffer(pd2.m_svc_mode, dyn_change_cap, level, sndMBps, sndFrameSizeMB);
		VS_RcvFunc::SetTracks(spd.m_fltr, tracks, nTracks);
		if (m_sr->AddParticipant(spd.m_conf.m_str, spd.m_name.m_str, spd.m_rcv.m_str, strb, false, std::chrono::seconds(30), tracks, nTracks)) {
			result = SCR_CONNECTED_OK;
		}
		else {
			dprint1("AddParticipant (%s %s) FALSE \n", spd.m_conf.m_str, spd.m_name.m_str);
			result = SCR_FAILED;
		}
	}
	else {
		spd.m_type = RCT_INTERCOM;
		spd.m_port = pd2.m_port;
		spd.m_host = pd2.m_host;
		spd.m_mcast_ip = pd2.m_mcast_ip;
		spd.m_confKey = pd2.m_confKey;
		result = SCR_CONNECTED_OK;
	}
	if (result==SCR_CONNECTED_OK && fltr!=spd.m_fltr) {
		SendCommandMessage(CCMD_CONFEV_DATANOTAVALIBLE, pd1.m_user_id);
	}
	return result;
}

bool VS_MultiConfService::ConnectServices(VS_ParticipantDescription &pd,  long fltr){
	dprint4("ConnectServices %s to %s fltr = %lx \n", pd.m_user_id.m_str, "ALL", fltr);
	VS_ConferenceDescription cd;
	VS_ParticipantDescription *pD = NULL;
	int NumOfPart = 0;
	VS_Map::Iterator i, i2;
	VS_Map *sPartMap1, *sPartMap2 = 0;

	if (!g_storage->FindConference(SimpleStrToStringView(pd.m_conf_id), cd))
		return false;

	i = m_parts.Find(pd.m_user_id);
	if (i==m_parts.End()) { m_parts.Insert(pd.m_user_id, 0); i = m_parts.Find(pd.m_user_id); }
	sPartMap1 = (VS_Map*)(*i).data;

	NumOfPart = g_storage->GetParticipants(pd.m_conf_id, pD);
	if (NumOfPart <= 0) {
		return false;
	}
	// set broadcast status
	if ((fltr)&&(pd.m_brStatus &BS_TRACK5_ON)) {
		//pd.m_brStatus &=~BS_SND_PAUSED;
		if (fltr & BS_OWNER_SS) {
			// find any broadcaster on track 5
			int brStatus = 0;
			for (int i = 0; i<NumOfPart; i++)
				brStatus |=pD[i].m_brStatus;
			if ((brStatus&BS_OWNER_SS)==0)
				pd.m_brStatus |=BS_OWNER_SS;
		}
		else
			pd.m_brStatus &=~BS_OWNER_SS;
		if (fltr & BS_OWNER_DS) {
			// find any broadcaster on track 5
			int brStatus = 0;
			for (int i = 0; i<NumOfPart; i++)
				brStatus |=pD[i].m_brStatus;
			if ((brStatus&BS_OWNER_DS)==0)
				pd.m_brStatus |=BS_OWNER_DS;
		}
		else
			pd.m_brStatus &=~BS_OWNER_DS;

	}
	else {
		pd.m_brStatus &=~BS_OWNER_DS;
		pd.m_brStatus &=~BS_OWNER_SS;
	}

	g_storage->UpdateParticipant(pd);


	delete[] pD;
	return true;

};
// if party==NULL connect with all parts
bool VS_MultiConfService::ConnectPart(VS_ParticipantDescription &pd, const char *party, long fltr, int dir)
{
	dprint4("ConnectPart %s to %s fltr = %lx, dir = %d \n", pd.m_user_id.m_str, party?party:"ALL", fltr, dir);
	char name[512];
	char name2[512];
	stream::Track tracks[4];
	unsigned nTracks;
	VS_ConferenceDescription cd;
	VS_ParticipantDescription *pD = NULL;
	int NumOfPart = 0, j;
	VS_Map::Iterator i, i2;
	VS_Map *sPartMap1, *sPartMap2 = 0;

	if (!g_storage->FindConference(SimpleStrToStringView(pd.m_conf_id), cd))
		return false;

	i = m_parts.Find(pd.m_user_id);
	if (i==m_parts.End()) { m_parts.Insert(pd.m_user_id, 0); i = m_parts.Find(pd.m_user_id); }
	sPartMap1 = (VS_Map*)(*i).data;

	if (!party) {
		NumOfPart = g_storage->GetParticipants(pd.m_conf_id, pD);
		if (NumOfPart <= 0) {
			return false;
		}

		// set broadcast status
		if (fltr) {
			pd.m_brStatus &=~BS_SND_PAUSED;
			if (fltr & VS_RcvFunc::FLTR_RCV_DATA) {
				// find any broadcaster on track 5
				/*int brStatus = 0;
				for (int i = 0; i<NumOfPart; i++)
					brStatus |=pD[i].m_brStatus;
				if ((brStatus&BS_TRACK5_ON)==0)*/
					pd.m_brStatus |=BS_TRACK5_ON;
			}
			else
				pd.m_brStatus &=~BS_TRACK5_ON;
		}
		else {
			pd.m_brStatus =BS_SND_PAUSED;
		}
        if (!(fltr & VS_RcvFunc::FLTR_RCV_DATA)) {
			pd.m_brStatus &=~BS_OWNER_SS;
			pd.m_brStatus &=~BS_OWNER_DS;

		}
		g_storage->UpdateParticipant(pd);
	}
	else {
		pD = new VS_ParticipantDescription[1];
		NumOfPart = g_storage->FindParticipant(party, pD[0]);
		if (NumOfPart!=1) {
			delete[] pD;
			return false;
		}
	}

	for (j = 0; j<NumOfPart; j++) {
		long result = SCR_UNKNOWN;
		VS_RcvFunc::SetName(name, pd.m_user_id, pD[j].m_user_id);
		i = sPartMap1->Find(name);
		VS_RcvFunc::SetName(name2, pD[j].m_user_id, pd.m_user_id); // revers
		i2 = sPartMap1->Find(name2);

		if (pd.m_user_id == pD[j].m_user_id) {			// myself coonection
			if (i==sPartMap1->End()) {					// new myself connect;
				VS_StreamPartDesc spd;
				result = NewPartConnect(spd, pd, pD[j], cd.m_type, fltr);
				if (result != SCR_CONNECTED_OK) {
					delete[] pD;
					return false;
				}
				else {
					sPartMap1->Insert(spd.m_name, &spd);
					if (spd.m_type == RCT_INTERCOM)
						SenderConnected(spd, result, pd.m_server_id, pd.m_user_id);
					if(pd.m_caps.IsValid())
					{
						m_sr->SetParticipantCaps(pd.m_conf_id.m_str, pd.m_user_id.m_str, pd.m_caps.Buffer(), pd.m_caps.Size());
					}
				}
			}
			else {										// do nothing now
			}
		}
		else {											// coonect with differrent paraticipant
			if (cd.m_SubType==GCST_ALL_TO_OWNER || cd.m_SubType==GCST_PENDING_ALL_TO_OWNER) {
				if (cd.m_owner != pD[j].m_user_id && cd.m_owner != pd.m_user_id &&
					pD[j].m_role != PR_LEADER && pd.m_role != PR_LEADER)
					continue;
			}
			int subDir = dir; //local directions rule
			auto IsTransceiver = [](const VS_ParticipantDescription & pd) {
				return pd.m_ClientType == CT_TRANSCODER || pd.m_ClientType == CT_WEB_CLIENT || pd.m_ClientType == CT_TRANSCODER_CLIENT;
			};
			if (IsTransceiver(pd)) {
				if (IsTransceiver(pD[j]))
					continue;
				else
					subDir&= ~MRVD_FORWARD;
			}
			else if (IsTransceiver(pD[j])) {
				if (IsTransceiver(pd))
					continue;
				else
					subDir&= ~MRVD_BACKWARD;
			}

			if (i!= sPartMap1->End() && subDir&MRVD_FORWARD) {					// found, apply any change
				VS_StreamPartDesc* spd = (VS_StreamPartDesc*)(*i).data;
				dprint3("connection found: %s \n", spd->m_name.m_str);
				if (fltr) {
					if (spd->m_fltr^fltr & VS_RcvFunc::FLTR_ALL_MEDIA) {
						i = m_parts.Find(pD[j].m_user_id);
						if (i==m_parts.End()) { //first call (can not be)
							m_parts.Insert(pD[j].m_user_id, 0);
							i = m_parts.Find(pD[j].m_user_id);
						}
						sPartMap2 = (VS_Map*)(*i).data;
						// any change in FLTR_ALL_MEDIA
						long lfltr1 = CheckParticipantsConnects(cd, pd, pD[j], fltr);
						long lfltr2 = VS_CheckFmt(pd.m_caps, pD[j].m_caps, lfltr1);
						if (cd.m_type==CT_MULTISTREAM) {
							VS_RcvFunc::SetTracks(lfltr2, tracks, nTracks);
							if (m_sr->SetParticipantReceiverTracks(spd->m_conf.m_str, spd->m_name.m_str, spd->m_rcv.m_str, tracks, nTracks)) {
								result = SCR_CHANGED_OK;
							}
							else {
								dprint1("SetParticipantReceiverTracks (%s %s %s %x %d) FALSE \n",
									spd->m_conf.m_str, spd->m_name.m_str, spd->m_rcv.m_str, static_cast<unsigned>(tracks[0]) | static_cast<unsigned>(tracks[1])<<8 | static_cast<unsigned>(tracks[2])<<16 | static_cast<unsigned>(tracks[3])<<24, nTracks);
								result = SCR_FAILED;
							}
						}
						else {
							result = SCR_CHANGED_OK;
						}
						if (result==SCR_CHANGED_OK) {	// update
							long dvs = spd->m_dvs & (DVS_MASK_RCV|DVS_MASK_RCV<<16) | pD[j].m_devStatus;
							VS_SetRcvDevStatus(dvs, fltr, lfltr1, 0);
							VS_SetRcvDevStatus(dvs, lfltr1, lfltr2, 1);
							spd->m_fltr = lfltr2;
							spd->m_dvs = dvs;
							i = sPartMap2->Find(name);
							if (i!= sPartMap2->End()) {	// found, apply any change
								((VS_StreamPartDesc*)(*i).data)->m_fltr = lfltr2;
								((VS_StreamPartDesc*)(*i).data)->m_dvs = dvs;
							}
						}
						ReceiverConnected(*spd, result, pd.m_server_id, pd.m_user_id);
					}
				}
				else {									// remove from SR
					if (cd.m_type == CT_MULTISTREAM) {
						m_sr->RemoveParticipant(spd->m_conf.m_str, spd->m_name.m_str);
					}
					else {
						VS_SimpleStr Snd, Rcv, conf, name;
						conf = spd->m_conf;
						name = spd->m_name;
						StreamPartRemoved(conf, name, Snd, Rcv);
					}
				}
			}
			else if (i2!= sPartMap1->End() && subDir&MRVD_BACKWARD) {					// found, apply any change
				VS_StreamPartDesc* spd = (VS_StreamPartDesc*)(*i2).data;
				dprint3("connection found: %s \n", spd->m_name.m_str);
				if (fltr) {	// todo: some bugs here
					if (spd->m_fltr^fltr & VS_RcvFunc::FLTR_ALL_MEDIA) {
						i = m_parts.Find(pD[j].m_user_id);
						if (i==m_parts.End()) { //first call (can not be)
							m_parts.Insert(pD[j].m_user_id, 0);
							i = m_parts.Find(pD[j].m_user_id);
						}
						sPartMap2 = (VS_Map*)(*i).data;
						// any change in FLTR_ALL_MEDIA
						long lfltr1 = CheckParticipantsConnects(cd, pD[j], pd, fltr);
						long lfltr2 = VS_CheckFmt(pD[j].m_caps, pd.m_caps, lfltr1);
						if (cd.m_type==CT_MULTISTREAM) {
							VS_RcvFunc::SetTracks(lfltr2, tracks, nTracks);
							if (m_sr->SetParticipantReceiverTracks(spd->m_conf.m_str, spd->m_name.m_str, spd->m_rcv.m_str, tracks, nTracks)) {
								result = SCR_CHANGED_OK;
							}
							else {
								dprint1("SetParticipantReceiverTracks (%s %s %s %x %d) FALSE \n",
									spd->m_conf.m_str, spd->m_name.m_str, spd->m_rcv.m_str, static_cast<unsigned>(tracks[0]) | static_cast<unsigned>(tracks[1])<<8 | static_cast<unsigned>(tracks[2])<<16 | static_cast<unsigned>(tracks[3])<<24, nTracks);
								result = SCR_FAILED;
							}
						}
						else {
							result = SCR_CHANGED_OK;
						}
						if (result==SCR_CHANGED_OK) {	// update
							long dvs = spd->m_dvs;
							VS_SetRcvDevStatus(dvs, fltr, lfltr1, 0);
							VS_SetRcvDevStatus(dvs, lfltr1, lfltr2, 1);
							spd->m_fltr = lfltr2;
							spd->m_dvs = dvs;
							i = sPartMap2->Find(name2);
							if (i!= sPartMap2->End()) { 	// found, apply any change
								((VS_StreamPartDesc*)(*i).data)->m_fltr = lfltr2;
								((VS_StreamPartDesc*)(*i).data)->m_dvs = dvs;
							}
						}
						ReceiverConnected(*spd, result, pD[j].m_server_id, pD[j].m_user_id);
					}
				}
				else {									// remove from SR
					if (cd.m_type == CT_MULTISTREAM) {
						m_sr->RemoveParticipant(spd->m_conf.m_str, spd->m_name.m_str);
					}
					else {
						VS_SimpleStr Snd, Rcv, conf, name;
						conf = spd->m_conf;
						name = spd->m_name;
						StreamPartRemoved(conf, name, Snd, Rcv);
					}
				}
			}
			else {										// new, create all to all coonections...
				if (fltr==0)							// nothing to create
					continue;
				VS_StreamPartDesc spd;
				i = m_parts.Find(pD[j].m_user_id);
				if (i==m_parts.End()) {					//first call
					m_parts.Insert(pD[j].m_user_id, 0);
					i = m_parts.Find(pD[j].m_user_id);
				}
				sPartMap2 = (VS_Map*)(*i).data;

				if (subDir&MRVD_FORWARD) {
					long lfltr = CheckParticipantsConnects(cd, pd, pD[j], fltr);
					if (lfltr) {
						result = NewPartConnect(spd, pd, pD[j], cd.m_type, lfltr);
						if (result == SCR_CONNECTED_OK) {
							VS_SetRcvDevStatus(spd.m_dvs, fltr, lfltr, 0);
							sPartMap1->Insert(spd.m_name, &spd);// new record
							sPartMap2->Insert(spd.m_name, &spd);// new record
						}
						ReceiverConnected(spd, result, pd.m_server_id, pd.m_user_id);
					}
				}

				if (subDir&MRVD_BACKWARD) {
					long lfltr = CheckParticipantsConnects(cd, pD[j], pd, fltr);
					if (lfltr) {
						result = NewPartConnect(spd, pD[j], pd, cd.m_type, lfltr);
						if (result == SCR_CONNECTED_OK) {
							VS_SetRcvDevStatus(spd.m_dvs, fltr, lfltr, 0);
							sPartMap1->Insert(spd.m_name, &spd);// new record
							sPartMap2->Insert(spd.m_name, &spd);// new record
							ReceiverConnected(spd, result, pD[j].m_server_id, pD[j].m_user_id);
						}
						else {
							// no notify for oposite if failed
						}
					}
				}
			}
		}
	}
	delete[] pD;
	return true;
}


bool VS_MultiConfService::DisconnectPart(const char *part, int type)
{
	dprint4("DisconnectPart %s %d \n", part, type);

	VS_Map::Iterator i;
	i = m_parts.Find(part);
	if (i==m_parts.End()) return false;

	VS_Map* sPartMap = (VS_Map*)(*i).data;
	if		(type == 1) {
		for (i = sPartMap->Begin(); i!=sPartMap->End(); ++i) {
			VS_StreamPartDesc* spd = (VS_StreamPartDesc*)(*i).data;
			m_sr->RemoveParticipant(spd->m_conf.m_str, spd->m_name.m_str); // remove all part receivers
			dprint4("m_sr->RemoveParticipant(%s %s) \n", spd->m_conf.m_str, spd->m_name.m_str);
		}
	}
	else if (type == 2) {
		int size = sPartMap->Size();
		for (int j = 0; j< size; j++) {
			i = sPartMap->Begin();
			VS_SimpleStr Snd, Rcv, conf, name;
			VS_StreamPartDesc* spd = (VS_StreamPartDesc*)(*i).data;
			m_sr->RemoveParticipant(spd->m_conf.m_str, spd->m_name.m_str); // remove all part receivers
			dprint4("m_sr->RemoveParticipant(%s %s) \n", spd->m_conf.m_str, spd->m_name.m_str);
			conf = spd->m_conf;
			name = spd->m_name;
			StreamPartRemoved(conf, name, Snd, Rcv);
		}
	}
	else {
		int size = sPartMap->Size();
		for (int j = 0; j< size; j++) {
			i = sPartMap->Begin();
			VS_SimpleStr Snd, Rcv, conf, name;
			VS_StreamPartDesc* spd = (VS_StreamPartDesc*)(*i).data;
			conf = spd->m_conf;
			name = spd->m_name;
			StreamPartRemoved(conf, name, Snd, Rcv);
		}
	}
	return true;
}


void VS_MultiConfService::StreamPartRemoved(const char * conf, const char *part, VS_SimpleStr &Snd, VS_SimpleStr &Rcv)
{
	dprint3("StreamPartRemoved %s \n", part);

	char snd[1024]; *snd = 0;
	char rcv[1024]; *rcv = 0;
	Snd.Empty(); Rcv.Empty();
	VS_RcvFunc::GetNames(part, snd, rcv); // obtain name antiderivatives

	VS_Map::Iterator i;
	VS_ParticipantDescription pd;
	VS_Map* sPartMap = 0;

	// oposite operation
	i = m_parts.Find(rcv);
	if (i==m_parts.End()) {
		dprint1("rcv not FOUND \n");
		Rcv = rcv;
	}
	else {
		sPartMap = (VS_Map*)(*i).data;
		i = sPartMap->Find(part);
		if (i!=sPartMap->End()) {
			VS_StreamPartDesc* spd = (VS_StreamPartDesc*)(*i).data;
			sPartMap->Erase(i);
		}
		else {
			dprint1("part %s of %s NOT FOUND \n", part, rcv);
		}
		if (sPartMap->Size()==0) {
			m_parts.Erase(rcv);
			Rcv = rcv;
		}
	}
	if (strcmp(snd, rcv)==0)
		return;

	i = m_parts.Find(snd);
	if (i==m_parts.End()) {
		dprint1("snd not FOUND \n");
		Snd = snd;
	}
	else {
		sPartMap = (VS_Map*)(*i).data;
		i = sPartMap->Find(part);
		if (i!=sPartMap->End()) {
			VS_StreamPartDesc* spd = (VS_StreamPartDesc*)(*i).data;
			if (g_storage->FindParticipant(snd, pd))
				ReceiverConnected(*spd, SCR_DISCONNECTED_OK, pd.m_server_id, pd.m_user_id);
			sPartMap->Erase(i);
		}
		else {
			dprint1("part %s of %s NOT FOUND \n", part, snd);
		}
		if (sPartMap->Size()==0) {
			m_parts.Erase(snd);
			Snd = snd;
		}
	}
}



/******************************************************************************
* 1 - Notifiy participant by CONFERENCEDELETED_METHOD
* 2 - Release participant (log it)
*****************************************************************************/
void VS_MultiConfService::RemoveParticipant_Event(const char* participant, long cause, VS_ConfPartStat* ps)
{
	VS_AutoLock lock(this);
	dprint3("RemovePart_Event %s %ld %d \n", participant? participant:"", cause, ps != 0);

	if (!participant)
		return;

	auto participant_iter = m_fecc_pending.find(participant);
	if (participant_iter != m_fecc_pending.end())
		m_fecc_pending.erase(participant_iter);

	for (auto it = m_fecc_pending.begin(); it != m_fecc_pending.end(); ) {
		auto it2 = it->second.find(participant);
		if (it2 != it->second.end())
			it2 = it->second.erase(it2);
		if (it->second.empty())
			it = m_fecc_pending.erase(it);
		else
			++it;
	}

	m_part_devices.erase(participant);

	VS_ConferenceDescription cd;
	VS_ParticipantDescription pd;
	VS_SimpleStr Snd, Rcv;												// for multistream conf
	bool IsRemoved = false;

	if (ps) {															// router notify
		if (!g_storage->FindConference(SimpleStrToStringView(ps->conf), cd))
			return; // already deleted all parts
		if (cd.m_type==CT_MULTISTREAM) {								// delete connectivity
			StreamPartRemoved(cd.m_name, participant, Snd, Rcv);
			if (!!Snd) RemoveParticipant_Event(Snd, cause);
			if (!!Rcv) RemoveParticipant_Event(Rcv, cause);
			return;
		}
		else
			if (!g_storage->FindParticipant(participant, pd))	return;	// nothing to delete
	}
	else {
		if (!g_storage->FindParticipant(participant, pd))	return;		// nothing to delete
		// part exists!
	}

	if (g_storage->FindConference(SimpleStrToStringView(pd.m_conf_id), cd)) {
		switch(cd.m_type)
		{
		case CT_PRIVATE:
			if (cd.m_state==cd.PARTICIPANT_ACCEPTED) {
				cd.m_state = cd.CONFERENCE_ENDED;
				cd.m_logCause = cd.TERMINATED_BY_PART_LEAVE;
				cd.SetTimeExp(TIME_EXP_END);
				m_sr->RemoveConference(cd.m_name.m_str);
				g_storage->UpdateConference(cd);
			}
			else if (cd.m_state != cd.CONFERENCE_ENDED && cause != pd.DISCONNECT_ENDCONF){
				cd.m_state = cd.CONFERENCE_ENDED;
				RemoveConference_Event(cd, cd.TERMINATED_BY_PART_LEAVE);
			}
			else {
				// do nothing
			}
			break;
		case CT_PRIVATE_DIRECTLY:
			if (cd.m_state != cd.CONFERENCE_ENDED && cause != pd.DISCONNECT_ENDCONF) {
				cd.m_state = cd.CONFERENCE_ENDED;
				RemoveConference_Event(cd, cd.TERMINATED_BY_PART_LEAVE);
			}
			break;
		case CT_PUBLIC:
		case CT_HALF_PRIVATE:
		case CT_BROADCAST:
		case CT_VIPPUBLIC:
			if (cd.m_owner==pd.m_user_id || (cd.m_type==CT_HALF_PRIVATE && cd.m_party==pd.m_user_id)) {
				if (cd.m_state!=cd.CONFERENCE_ENDED) {
					cd.m_state = cd.CONFERENCE_ENDED;
					cd.m_logCause = cd.TERMINATED_BY_PART_LEAVE;
					cd.SetTimeExp(TIME_EXP_END);
					g_storage->UpdateConference(cd);
					m_sr->RemoveConference(cd.m_name.m_str);
				}
				else{
					// do nothing
				}
			}
			else {
				m_sr->RemoveParticipant(cd.m_name.m_str, pd.m_user_id.m_str);
			}
			break;
		case CT_MULTISTREAM:
			DisconnectPart(pd.m_user_id, cause==pd.DISCONNECT_ENDCONF? 2 : 1);
			break;
		case CT_INTERCOM:
			DisconnectPart(pd.m_user_id, 0);
			IsRemoved = true;
			break;
		}
	}
	else
		IsRemoved = true;

	IsRemoved|=	cause==pd.DISCONNECT_ENDCONF ||cause==pd.DISCONNECT_BYSTREAM;

	if (pd.m_cause==pd.DISCONNECT_UNKNOWN)
		pd.m_cause=(VS_ParticipantDescription::Disconnect_Cause)cause;
	if (IsRemoved) {
		dprint2("\t Removed ---------- %s \n", pd.m_user_id.m_str);
		if (ps) {					// save any stats
			pd.m_bytesRcv = (int)(ps->stat.allReceiverBytes>>10);
			pd.m_bytesSnd = (int)(ps->stat.allSenderBytes>>10);
			pd.m_reconRcv = ps->stat.receiverNConns;
			pd.m_reconSnd = ps->stat.senderNConns;
		}
		VS_Container rCnt;
		rCnt.AddValue(METHOD_PARAM, CONFERENCEDELETED_METHOD);
		rCnt.AddValue(NAME_PARAM, pd.m_conf_id);
		rCnt.AddValueI32(RESULT_PARAM, CONFERENCE_DELETED_OK);
		rCnt.AddValueI32(CAUSE_PARAM, pd.m_cause);
		PostRequest(pd.m_server_id, pd.m_user_id, rCnt);
		g_storage->DeleteParticipant(pd);
		if (pd.m_port)
			m_mcast.FreePort(pd.m_mcast_ip, pd.m_port);
		if (pd.m_cause==pd.DISCONNECT_BYKICK)
			SendCommandMessage(CCMD_CONFEV_REMOVEDBY, pd.m_user_id, pd.m_server_id);

		if (!!cd.m_name) {												// conf exists
			if (cd.m_type==CT_PUBLIC || cd.m_type==CT_VIPPUBLIC || cd.m_type==CT_BROADCAST || cd.m_type==CT_HALF_PRIVATE) {
				SendJoinMessage(cd, pd.m_user_id, 1);
			}

			LogParticipantLeave(pd);

			VS_Container rCnt;
			rCnt.AddValue(METHOD_PARAM, DELETEPARTICIPANT_METHOD);
			rCnt.AddValue(CONFERENCE_PARAM, cd.m_name);
			rCnt.AddValue(USERNAME_PARAM, pd.m_user_id);
			rCnt.AddValueI32(CAUSE_PARAM, pd.m_cause);
			VS_BinBuff buff; buff.Set(rCnt);
			PostRequest(OurEndpoint(), 0, buff.Buffer(), buff.Size(), 0, CHAT_SRV);
			PostRequest(OurEndpoint(), 0, buff.Buffer(), buff.Size(), 0, DS_CONTROL_SRV);

			m_confRestriction->OnRemovePartEvent(pd, cd, this);

			if ( cd.m_state!=cd.CONFERENCE_ENDED && cause!=pd.DISCONNECT_ENDCONF &&
				(cd.m_type==CT_MULTISTREAM || cd.m_type==CT_INTERCOM) )
			{
				if (g_storage->GetNumOfParts(cd.m_name)==0) {
					cd.m_state = cd.CONFERENCE_ENDED;
					RemoveConference_Event(cd, cd.TERMINATED_BY_PART_LEAVE);
				}
			}

			SendPartsList(pd, PLT_DEL);
			FreeSlot(cd, SimpleStrToStringView(pd.m_user_id));

			VS_SimpleStr value;
			if (m_confRestriction->GetAppProp("lstatus_set", value) && !!value)
			{
				UpdateLStatus(cd.m_name, pd.m_user_id, 0);
			}
		}
	}
	else
		g_storage->UpdateParticipant(pd);

	VS_AutoInviteService::Subscribe(pd.m_conf_id, pd.m_user_id);
}

void VS_MultiConfService::RemoveConference_Event(VS_ConferenceDescription& cd, long cause)
{
	assert(cd.m_name.m_str != nullptr);
	dprint3("RemoveConference_Event <%s>, cause = %ld\n", cd.m_name.m_str, cause);

	RemoveConfKicks(SimpleStrToStringView(cd.m_name));

	VS_AutoInviteService::RemoveConf(cd.m_name);

	std::vector<tPartServer> part_server;
	g_storage->GetParticipants(cd.m_name, part_server);
	for (auto &i : part_server)
		RemoveParticipant_Event(i.first, VS_ParticipantDescription::DISCONNECT_ENDCONF);

	if (cd.m_logCause == cd.UNKNOWN)
		cd.m_logCause = (VS_ConferenceDescription::TerminationCause)cause;

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, CONFERENCEDELETED_METHOD);
	rCnt.AddValue(NAME_PARAM, cd.m_name);
	rCnt.AddValueI32(RESULT_PARAM, CONFERENCE_DELETED_OK);
	rCnt.AddValueI32(CAUSE_PARAM, cd.m_logCause);

	PostRequest(0, cd.m_owner, rCnt);
	if (cd.m_party)
		PostRequest(0, cd.m_party, rCnt);

	PostRequest(OurEndpoint(), 0, rCnt, 0, CHAT_SRV);
	m_RTSPBroadcastModule->StopBroadcast(cd.m_name.m_str);
	auto transceiversPool = m_transceiversPool.lock();
	if (!transceiversPool) {
		dstream1 << "Error\n TransceiversPool not found!\n";
		return;
	}

	m_confRestriction->SetRecordState(cd, RS_NO_RECORDING);

	auto streamsCircuit = transceiversPool->GetTransceiverProxy(cd.m_name.m_str);
	if (streamsCircuit) {
		std::static_pointer_cast<VS_ConfControlInterface>(streamsCircuit)->StopConference(cd.m_name.m_str);
	}
	for (auto& announce: cd.m_rtspAnnounces)
	{
		announce.second.active = false;
		announce.second.reason.clear();
	}
	m_confRestriction->UpdateConference_RTSPAnnounce(cd);
	m_sr->RemoveConference(cd.m_name.m_str); // podstrachovka
	g_storage->DeleteConference(cd);

	// log end
	LogConferenceEnd(cd);

	if(cd.m_type==CT_MULTISTREAM || cd.m_type==CT_INTERCOM ) {
		cd.m_name.Empty();
		m_confRestriction->UpdateMultiConference(cd, false);
	}
}

int VS_MultiConfService::RestrictBitrate(const char *conferenceName, const char *participantName, int bitrate)
{
	VS_ParticipantDescription pd;
	dprint3("Restrict Bitrate for %s is %d\n", participantName, bitrate);
	if (participantName && g_storage->FindParticipant(participantName, pd)) {
		VS_Container rCnt;
		rCnt.AddValue(METHOD_PARAM, SENDCOMMAND_METHOD);
		rCnt.AddValueI32(RESTRICTBITRATE_PARAM, bitrate);
		rCnt.AddValue(CONFERENCE_PARAM, conferenceName);
		if (bitrate > 0) {
			if (PostRequest(pd.m_server_id, pd.m_user_id, rCnt))
				return 0;
		}
		else {
			rCnt.AddValue(USERNAME_PARAM, participantName);
			rCnt.AddValue(SERVER_PARAM, (const char*)pd.m_server_id);
			if (PostRequest(OurEndpoint(), 0, rCnt, 0, 0, 5000))
				return 0;
		}
	}
	return -1;
}

int VS_MultiConfService::RestrictBitrateSVC(const char *conferenceName, const char *participantName, int v_bitrate, int bitrate, int old_bitrate)
{
	VS_ParticipantDescription pd;
	if (participantName && g_storage->FindParticipant(participantName, pd)) {
		VS_Container rCnt;
		rCnt.AddValue(METHOD_PARAM, SENDCOMMAND_METHOD);
		if (pd.m_version >= 44) {
			int64_t bitrate_svc = int64_t( ((int64_t)v_bitrate << 32) | bitrate );
			rCnt.AddValueI64(RESTRICTBITRATESVC_PARAM, bitrate_svc);
			dprint3("Restrict Bitrate SVC for %s is (v = %d, d = %d), ver = %d\n", participantName, v_bitrate, bitrate - v_bitrate, pd.m_version);
		} else {
			rCnt.AddValueI32(RESTRICTBITRATE_PARAM, old_bitrate);
			dprint3("Restrict Bitrate SVC for %s is all = %d, ver = %d\n", participantName, old_bitrate, pd.m_version);
			bitrate = old_bitrate;
		}
		rCnt.AddValue(CONFERENCE_PARAM, conferenceName);
		if (bitrate > 0) {
			if (PostRequest(pd.m_server_id, pd.m_user_id, rCnt))
				return 0;
		}
		else {
			rCnt.AddValue(USERNAME_PARAM, participantName);
			rCnt.AddValue(SERVER_PARAM, (const char*)pd.m_server_id);
			if (PostRequest(OurEndpoint(), 0, rCnt, 0, 0, 5000))
				return 0;
		}
	}
	return -1;
}

void VS_MultiConfService::RequestKeyFrame(const char *conferenceName, const char *participantName)
{
	VS_ParticipantDescription pd;
	dprint3("Request Key Frame for %s\n", participantName);
	if (participantName && g_storage->FindParticipant(participantName, pd)) {
		VS_Container rCnt;
		rCnt.AddValue(METHOD_PARAM, SENDCOMMAND_METHOD);
		rCnt.AddValue(REQESTKEYFRAME_PARAM, true);
		rCnt.AddValue(CONFERENCE_PARAM, conferenceName);
		PostRequest(pd.m_server_id, pd.m_user_id, rCnt);
	}
}

void VS_MultiConfService::SetParticipantCaps(const char* conf_name, const char* part_name, const void* caps_buf, size_t buf_sz)
{
}

void VS_MultiConfService::TurnOffReceiverVideo(VS_StreamPartDesc* spd, const char* dst_server, const char* dst_user)
{
	int fltr = spd->m_fltr&~VS_RcvFunc::FLTR_RCV_VIDEO; // clear bit
	stream::Track tracks[4];
	unsigned nTracks;
	VS_RcvFunc::SetTracks(fltr, tracks, nTracks);
	if (m_sr->SetParticipantReceiverTracks(spd->m_conf.m_str, spd->m_name.m_str, spd->m_rcv.m_str, tracks, nTracks)) {
		VS_SetRcvDevStatus(spd->m_dvs, fltr, fltr, 2);
		spd->m_fltr = fltr;								// update
		VS_Map::Iterator i = m_parts.Find(spd->m_rcv);	// find receiver's spds
		if (i!=m_parts.End()) {
			VS_Map *sPartMap2 = (VS_Map*)(*i).data;
			i = sPartMap2->Find(spd->m_name);
			if (i!= sPartMap2->End()) {					// found, apply any change
				((VS_StreamPartDesc*)(*i).data)->m_fltr = fltr;
				((VS_StreamPartDesc*)(*i).data)->m_dvs = spd->m_dvs;
			}
		}
		ReceiverConnected(*spd, SCR_CHANGED_OK, dst_server, dst_user);
	}
}


void VS_MultiConfService::SendCommand_Method(VS_Container &cnt)
{
	int32_t bitrate = 0;
	if (cnt.GetValue(RESTRICTBITRATE_PARAM, bitrate) && bitrate==-1) {
		VS_ConferenceDescription cd;
		if (!g_storage->FindConference(cnt.GetStrValueView(CONFERENCE_PARAM), cd) || cd.m_type != CT_MULTISTREAM)
			return;
		const char* participantName = cnt.GetStrValueRef(USERNAME_PARAM);
		const char* server = m_recvMess->SrcServer();
		VS_Map::Iterator it = m_parts.Find(participantName);
		if (it==m_parts.End())
			return;
		VS_Map *sPartMap1 = (VS_Map*)(*it).data;
		if (cd.m_SubType!=GCST_ROLE) {
			for (it = sPartMap1->Begin(); it!=sPartMap1->End(); ++it) {
				VS_StreamPartDesc* spd = (VS_StreamPartDesc*)(*it).data;
				if (!spd->IsIam() && spd->m_snd==participantName && spd->m_fltr & VS_RcvFunc::FLTR_RCV_VIDEO) {
					TurnOffReceiverVideo(spd, server, participantName);
					return;
				}
			}
		}
		else {
			VS_ParticipantDescription *parts = 0;
			int NumOfPart = 0;
			VS_ParticipantDescription *rcvp[3] = {0, 0, 0};
			if ((NumOfPart = g_storage->GetParticipants(cd.m_name, parts))>0) {
				for (int i = 0; i<NumOfPart; i++) {
					if (parts[i].m_role==PR_LEADER)
						rcvp[0] = &parts[i];
					if (parts[i].m_role==PR_PODIUM)
						rcvp[1] = &parts[i];
					if (parts[i].m_role==PR_REPORTER)
						rcvp[2] = &parts[i];
				}
				for (int i = 0; i<3; i++) {
					if (rcvp[i]) {
						char name[256];
						VS_RcvFunc::SetName(name, participantName, rcvp[i]->m_user_id);
						it = sPartMap1->Find(name);
						if (it!=sPartMap1->End()) {
							VS_StreamPartDesc* spd = (VS_StreamPartDesc*)(*it).data;
							if (!spd->IsIam() && spd->m_fltr & VS_RcvFunc::FLTR_RCV_VIDEO) {
								TurnOffReceiverVideo(spd, server, participantName);
								break;
							}
						}
					}
				}
				delete[] parts;
			}
		}
	}
}


void VS_MultiConfService::PingParticipant_Method(const char *conf, const char *part, int32_t *sysLoad)
{
	if (!conf || ! part) return;
	VS_ParticipantDescription pd;
	if (g_storage->FindParticipant(part, pd) && pd.m_conf_id==conf) {
		pd.m_addledTick = std::chrono::steady_clock::now();
		g_storage->UpdateParticipant(pd);
		if (sysLoad) {
			m_load.emplace_back(conf, part, *sysLoad);
		}
	}
}

void VS_MultiConfService::UpdateFrameMBpsParticipants_Method(VS_Container &cnt)
{
	const char *conf = 0, *partFrom = 0, *partTo = 0;
	int32_t frameSizeMB;
	while (cnt.Next()) {
		if (strcasecmp(cnt.GetName(), CONFERENCE_PARAM) == 0) {
			conf = cnt.GetStrValueRef();
		} else if (strcasecmp(cnt.GetName(), USERNAME_PARAM) == 0) {
			partTo = cnt.GetStrValueRef();
			break;
		}
	}
	std::vector<stream::ParticipantFrameSizeInfo> mbpart;
	while (cnt.Next()) {
		if (strcasecmp(cnt.GetName(), USERNAME_PARAM) == 0) {
			partFrom = cnt.GetStrValueRef();
		} else if (strcasecmp(cnt.GetName(), FRAMESIZEMBUSER_PARAM) == 0) {
			cnt.GetValue(frameSizeMB);
			if (partFrom) {
				mbpart.emplace_back(partFrom, frameSizeMB);
			}
		}
	}
	if (mbpart.size() > 0) {
		m_sr->SetParticipantFrameSizeMB(conf, partTo, std::move(mbpart));
	}
}

void VS_MultiConfService::LogConnectFailure_Method(VS_Container &cnt)
{
	const char *conf = cnt.GetStrValueRef(CONFERENCE_PARAM);
	const char *owner = cnt.GetStrValueRef(OWNER_PARAM);
	const char *user = cnt.GetStrValueRef(USERNAME_PARAM);
	const char *ip = cnt.GetStrValueRef(HOST_PARAM);
	if (!ip) ip = "";

	dprint4("connect error|%s|%s|%s|%s\n", conf, owner, user, ip);
	// date_time|debug_level|debug_module|thread_id|type_message|conference_id|user_owner|user_remote|ip
}

void VS_MultiConfService::ManageConference(VS_Container &cnt)
{
	if (!m_recvMess->IsFromServer() && !m_confRestriction->CheckSessionID(cnt.GetStrValueRef(SESSION_PARAM)))
		return;

	auto conf = cnt.GetStrValueView(CONFERENCE_PARAM);
	if (conf.empty())
		return;
	auto conf_str = StringViewToSimpleStr(conf);	// todo(kt): delete when m_confWriterModule API is changed

	vs_user_id user = VS_RealUserLogin(cnt.GetStrValueView(USERNAME_PARAM)).GetID();
	VS_ParticipantDescription pd;
	bool pd_found = false;
	if (!!user)
		pd_found = g_storage->FindParticipant(SimpleStrToStringView(user), pd);

	auto Func = cnt.GetStrValueView("Func");

	if (Func == "SetLayout") {
		if (!pd_found && pd.m_user_id.IsEmpty())
			pd.m_user_id = user;
		layout_for f = layout_for::all;
		cnt.GetValueI32(TYPE_PARAM, f);
		MC_SetLayout(conf, pd, f, cnt.GetStrValueView(LAYOUT_PARAM));
	}

	if (!!user && (!pd_found || SimpleStrToStringView(pd.m_conf_id) != conf))
		return;

	if (Func == "SetRole") {
		int32_t role = 0; cnt.GetValue(ROLE_PARAM, role);
		if (pd.m_role == role)
			return;
		if (role == PR_REMARK)	// remark could be obtained only by participant query
			return;
		if (pd.m_IsOperatorByGroups)	// operators role couldn't be changed
			return;
		VS_ConferenceDescription cd;
		if (!g_storage->FindConference(conf, cd) || cd.m_owner == pd.m_user_id)	// owners role couldn't be changed
			return;

		switch (role)
		{
		case PR_PODIUM:
			role = PR_REPORTER;
			VS_FALLTHROUGH;
		case PR_REPORTER:
		case PR_LEADER:
			CheckPodiumsBeforeCast(role, VS_RcvFunc::FLTR_DEFAULT_MULTIS, pd.m_conf_id);
			VS_FALLTHROUGH;
		case PR_COMMON:
		case PR_EQUAL:
			SetRole(pd, role);
		}
	}
	else if (Func == "JoinPodium") {
		if (pd.m_role == PR_LEADER) {
			if (pd.m_brStatus & BS_SND_PAUSED) {
				long fltr = VS_RcvFunc::FLTR_DEFAULT_MULTIS;
				CheckPodiumsBeforeCast(pd.m_role, fltr, pd.m_conf_id);

				ConnectPart(pd, 0, fltr, MRVD_BACKWARD);
				UpdateMediaFiltr(pd, fltr);
				NotifyRole(pd);
			}
		}
		else if (pd.m_role == PR_COMMON || pd.m_role == PR_REMARK) {
			long fltr = VS_RcvFunc::FLTR_DEFAULT_MULTIS;
			CheckPodiumsBeforeCast(PR_REPORTER, fltr, pd.m_conf_id);
			SetRole(pd, PR_REPORTER);
		}
	}
	else if (Func == "LeavePodium") {
		if (pd.m_role == PR_LEADER) {
			if ((pd.m_brStatus & BS_SND_PAUSED) == 0) {
				long fltr = 0;
				ConnectPart(pd, 0, fltr, MRVD_BACKWARD);
				UpdateMediaFiltr(pd, fltr);
				NotifyRole(pd);
			}
		}
		else if (pd.m_role == PR_REPORTER) {
			SetRole(pd, PR_COMMON);
		}
	}
	else if (Func == "QueryJoinPodium") {
		// query to participant to be a reporter
		if (pd.m_role == PR_COMMON || pd.m_role == PR_REMARK)
			RequestReporterRole(pd);
	}
	else if (Func == "Recording") {
		VS_ConferenceDescription cd;
		if (!g_storage->FindConference(conf, cd))
			return;

		int32_t type = 0; cnt.GetValue(TYPE_PARAM, type);

		VS_ConfRecordingState newState = (VS_ConfRecordingState)type;
		VS_ConfRecordingState currState = RS_NO_RECORDING;
		m_confRestriction->GetRecordState(cd, currState);

		if (newState == RS_NO_RECORDING) {
			if (currState == RS_RECORDING || currState == RS_PAUSED)
				m_confWriterModule->StopRecordConference(conf_str);
			else
				return;
		}
		else if (newState == RS_RECORDING) {
			if (currState == RS_NO_RECORDING)
				m_confWriterModule->StartRecordConference(conf_str);
			else if (currState == RS_PAUSED)
				m_confWriterModule->ResumeRecordConference(conf_str);
			else
				return;
		}
		else if (newState == RS_PAUSED) {
			if (currState == RS_RECORDING)
				m_confWriterModule->PauseRecordConference(conf_str);
			else
				return;
		}

		m_confRestriction->SetRecordState(cd, newState);
	}
	else if (Func == CHANGEDEVICE_METHOD) {
		const char* type = cnt.GetStrValueRef(TYPE_PARAM);
		const char* id = cnt.GetStrValueRef(ID_PARAM);
		const char* name = cnt.GetStrValueRef(NAME_PARAM);
		MC_ChangeDevice(pd, type, id, name);
	}
	else if (Func == SETDEVICESTATE_METHOD) {
		const char* type = cnt.GetStrValueRef(TYPE_PARAM);
		const char* id = cnt.GetStrValueRef(ID_PARAM);
		bool mute = false;
		int32_t volume = 0;
		if (cnt.GetValue(MUTE_PARAM, mute)) {
			MC_SetDeviceMute(pd, type, id, mute);
		}
		if (cnt.GetValue(VOLUME_PARAM, volume)) {
			MC_SetDeviceVol(pd, type, id, volume);
		}
	}
}

void VS_MultiConfService::MC_SetLayout(string_view stream_id, const VS_ParticipantDescription& pd, layout_for f, string_view new_layout)
{
	VS_ConferenceDescription cd;
	if (!g_storage->FindConference(stream_id, cd))
		return;

	if (!m_confRestriction->UpdateLayout(SimpleStrToStringView(cd.m_call_id), new_layout, f, SimpleStrToStringView(pd.m_user_id)))
		return;

	auto FillCnt = [](string_view stream_id, string_view lt) -> VS_Container
	{
		VS_Container cnt;
		cnt.AddValue(METHOD_PARAM, MANAGELAYOUT_METHOD);
		cnt.AddValue(CONFERENCE_PARAM, stream_id);
		cnt.AddValue(TYPE_PARAM, CLT_FIXED);
		cnt.AddValue(LAYOUT_PARAM, lt);
		return cnt;
	};

	if (f == layout_for::all) {
		VS_BinBuff buff;
		// check all parts
		std::vector<tPartServer> parts;
		g_storage->GetParticipants(StringViewToSimpleStr(stream_id), parts);
		for (auto &i : parts) {
			auto layout_for_part = m_confRestriction->ReadLayout(SimpleStrToStringView(cd.m_call_id), layout_for::individual, SimpleStrToStringView(i.first));
			if (layout_for_part == new_layout) {
				if (!buff.IsValid()) {
					VS_Container cnt = FillCnt(stream_id, new_layout);
					buff.Set(cnt);
				}
				PostRequest(i.second, i.first, buff.Buffer(), buff.Size(), 0, CONFERENCE_SRV);
			}
			else {
				// resend old?
				VS_Container cnt = FillCnt(stream_id, layout_for_part);
				PostRequest(i.second, i.first, cnt, 0, CONFERENCE_SRV);
			}
		}
		SetOrInviteOnPodiumByLayout(cd, std::string(new_layout), 0, true);
	}

	if (f == layout_for::all || f == layout_for::mixer) {
		auto l = m_confRestriction->ReadLayout(SimpleStrToStringView(cd.m_call_id), layout_for::mixer);
		auto transceiversPool = m_transceiversPool.lock();
		if (!transceiversPool) {
			dstream1 << "Error\n TransceiversPool not found!\n";
			return;
		}
		auto streamsCircuit = transceiversPool->GetTransceiverProxy(cd.m_name.m_str);
		if (streamsCircuit) {
			if (auto confCtrl = streamsCircuit->ConfControl())
				confCtrl->UpdateLayout(cd.m_name, l.c_str());
		}
		std::string id = g_storage->GetOneTrascoderPartId(cd.m_name);
		if (!id.empty())
			ResendSlidesCommand(cd.m_name.m_str, id);
	} else if (f == layout_for::individual) {
		if (!pd.m_server_id.IsEmpty() && !pd.m_user_id.IsEmpty()) {
			auto l = m_confRestriction->ReadLayout(SimpleStrToStringView(cd.m_call_id), layout_for::individual, SimpleStrToStringView(pd.m_user_id));
			VS_Container cnt = FillCnt(stream_id, l);
			PostRequest(pd.m_server_id, pd.m_user_id, cnt, 0, CONFERENCE_SRV);
		}
	}
}

void VS_MultiConfService::MC_ChangeDevice(VS_ParticipantDescription &pd, const char* type, const char* id, const char* name)
{
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, CHANGEDEVICE_METHOD);
	cnt.AddValue(CONFERENCE_PARAM, pd.m_conf_id);
	cnt.AddValue(TYPE_PARAM, type);
	cnt.AddValue(ID_PARAM, id);
	cnt.AddValue(NAME_PARAM, name);
	cnt.AddValueI32(CAUSE_PARAM, DeviceChangingCause::SERVER);
	PostRequest(pd.m_server_id, pd.m_user_id, cnt, 0, CONFERENCE_SRV);
}

void VS_MultiConfService::MC_SetDeviceVol(VS_ParticipantDescription &pd, const char* type, const char* id, int32_t volume)
{
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, SETDEVICESTATE_METHOD);
	cnt.AddValue(CONFERENCE_PARAM, pd.m_conf_id);
	cnt.AddValue(TYPE_PARAM, type);
	cnt.AddValue(ID_PARAM, id);
	cnt.AddValue(VOLUME_PARAM, volume);
	cnt.AddValueI32(CAUSE_PARAM, DeviceChangingCause::SERVER);
	PostRequest(pd.m_server_id, pd.m_user_id, cnt, 0, CONFERENCE_SRV);
}

void VS_MultiConfService::MC_SetDeviceMute(VS_ParticipantDescription &pd, const char* type, const char* id, bool mute)
{
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, SETDEVICESTATE_METHOD);
	cnt.AddValue(CONFERENCE_PARAM, pd.m_conf_id);
	cnt.AddValue(TYPE_PARAM, type);
	cnt.AddValue(ID_PARAM, id);
	cnt.AddValue(MUTE_PARAM, mute);
	cnt.AddValueI32(CAUSE_PARAM, DeviceChangingCause::SERVER);
	PostRequest(pd.m_server_id, pd.m_user_id, cnt, 0, CONFERENCE_SRV);
}

void VS_MultiConfService::LogParticipantDevice(const char* conf, const char* call_id, const char* action,
	const char* device_id, const char* device_type, const char* device_name, bool is_active, bool* pmute, int32_t* pvolume)
{
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, LOGPARTICIPANTDEVICE_METHOD);
	cnt.AddValue(CONFERENCE_PARAM, conf);
	cnt.AddValue(USERNAME_PARAM, call_id);
	cnt.AddValue(FUNC_PARAM, action);
	cnt.AddValue(ID_PARAM, device_id);
	cnt.AddValue(TYPE_PARAM, device_type);
	cnt.AddValue(NAME_PARAM, device_name);
	cnt.AddValue(ACTIVE_PARAM, is_active);
	if (pmute)
		cnt.AddValue(MUTE_PARAM, *pmute);
	if (pvolume)
		cnt.AddValue(VOLUME_PARAM, *pvolume);

	PostRequest(OurEndpoint(), 0, cnt, 0, LOG_SRV);
}

void VS_MultiConfService::LogParticipantRole(const VS_ParticipantDescription& pd)
{
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, LOGPARTICIPANTROLE_METHOD);
	cnt.AddValue(CONFERENCE_PARAM, pd.m_conf_id);
	cnt.AddValue(USERNAME_PARAM, pd.m_user_id);
	cnt.AddValueI32(ROLE_PARAM, (pd.m_role & 0xff) | (pd.m_brStatus << 8));
	PostRequest(OurEndpoint(), 0, cnt, 0, LOG_SRV);
}

void VS_MultiConfService::ResendSlidesCommand(string_view conf, string_view part)
{
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, SENDSLIDESTOUSER_METHOD);
	cnt.AddValue(CONFERENCE_PARAM, conf);
	cnt.AddValue(CALLID_PARAM, part);
	PostRequest(OurEndpoint(), 0, cnt, 0, CHAT_SRV);
}

void VS_MultiConfService::DeviceList_Method(VS_Container &cnt)
{
	const char *user = cnt.GetStrValueRef(USERNAME_PARAM);
	const char *conf = cnt.GetStrValueRef(CONFERENCE_PARAM);
	if (!conf || !user)
		return;

	VS_ParticipantDescription pd;
	if (!g_storage->FindParticipant(user, pd) || pd.m_conf_id != conf)
		return;

	decltype(m_part_devices)::iterator pi = m_part_devices.emplace(user, std::map<std::string, VS_DeviceInfo>()).first;

	std::map<std::string, VS_DeviceInfo> devices;
	VS_DeviceInfo *info = 0;
	std::string id;

	cnt.Reset();
	while (cnt.Next()) {
		if (strcasecmp(cnt.GetName(), TYPE_PARAM) == 0) {
			info = &devices[cnt.GetStrValueRef()];
		}
		else if (info == 0)
			continue;
		else if (strcasecmp(cnt.GetName(), ID_PARAM) == 0) {
			id = cnt.GetStrValueRef();
		}
		else if (strcasecmp(cnt.GetName(), NAME_PARAM) == 0) {
			std::string name = cnt.GetStrValueRef();
			if (id.empty() || name.empty())
				continue;
			info->device_list.emplace(std::move(id), std::move(name));
		}
	}

	for (auto &in : devices) {
		decltype(in.second.device_list) del, add;
		auto &m1 = pi->second[in.first].device_list;
		auto &m2 = in.second.device_list;
		std::set_difference(m1.begin(), m1.end(), m2.begin(), m2.end(), std::inserter(del, del.begin()));
		std::set_difference(m2.begin(), m2.end(), m1.begin(), m1.end(), std::inserter(add, add.begin()));
		for (auto &a : add)
			LogParticipantDevice(conf, user, "add", a.first.c_str(), in.first.c_str(), a.second.c_str());
		for (auto &d : del)
			LogParticipantDevice(conf, user, "remove", d.first.c_str(), in.first.c_str(), d.second.c_str());
		m1 = m2;
	}
}

void VS_MultiConfService::DeviceChanged_Method(VS_Container &cnt)
{
	const char *user = cnt.GetStrValueRef(USERNAME_PARAM);
	const char *conf = cnt.GetStrValueRef(CONFERENCE_PARAM);
	const char *type = cnt.GetStrValueRef(TYPE_PARAM);
	const char *id = cnt.GetStrValueRef(ID_PARAM);
	if (!user || !conf || !type || !id)
		return;
	const char *name = cnt.GetStrValueRef(NAME_PARAM);
	DeviceChangingCause cause(DeviceChangingCause::UNKNOWN); cnt.GetValueI32(CAUSE_PARAM, cause);

	decltype(m_part_devices)::iterator pi = m_part_devices.find(user);
	if (pi == m_part_devices.end())
		return;

	if (!*id || strcasecmp(id, "none") == 0) {
		auto & curr_id = pi->second[type].current;
		if (!curr_id.empty()) {
			LogParticipantDevice(conf, user, "activate", curr_id.c_str(), type, nullptr);
			curr_id.clear();
		}
	}
	else {
		pi->second[type].current = id;
		LogParticipantDevice(conf, user, "activate", id, type, nullptr, true);
	}
}

void VS_MultiConfService::DeviceState_Method(VS_Container &cnt)
{
	const char *user = cnt.GetStrValueRef(USERNAME_PARAM);
	const char *conf = cnt.GetStrValueRef(CONFERENCE_PARAM);
	const char *type = cnt.GetStrValueRef(TYPE_PARAM);
	const char *id = cnt.GetStrValueRef(ID_PARAM);
	int32_t volume(0); bool bvolume = cnt.GetValue(VOLUME_PARAM, volume);
	bool mute(false); bool bmute = cnt.GetValue(MUTE_PARAM, mute);
	DeviceChangingCause cause(DeviceChangingCause::UNKNOWN); cnt.GetValueI32(CAUSE_PARAM, cause);

	decltype(m_part_devices)::iterator pi = m_part_devices.find(user);
	if (pi == m_part_devices.end())
		return;

	if (bmute) pi->second[type].mute = mute;
	if (bvolume) pi->second[type].volume = volume;

	LogParticipantDevice(conf, user, "alter", id, type, nullptr, true, bmute ? &mute : nullptr, bvolume ? &volume : nullptr);
}

void VS_MultiConfService::QueryDevices_Method(VS_Container &cnt)
{
	const char *user = cnt.GetStrValueRef(USERNAME_PARAM);
	const char *conf = cnt.GetStrValueRef(CONFERENCE_PARAM);
	if (!conf || !user)
		return;

	VS_ParticipantDescription pd;
	if (!g_storage->FindParticipant(user, pd) || pd.m_conf_id != conf)
		return;

	decltype(m_part_devices)::iterator pi = m_part_devices.find(user);
	if (pi == m_part_devices.end())
		return;
}


/*****************************************************************************/
////////////////////////////////////////////////////////////////////////////////
// INVITE_METHOD
////////////////////////////////////////////////////////////////////////////////

class InviteMulti_Task: public VS_PoolThreadsTask, public VS_ConfLog
{
	boost::shared_ptr<VS_PresenceService> m_presenceService;
	InviteInfo v;
	bool m_cancel_and_redirect;
	boost::weak_ptr<VS_ConfRestrictInterface> m_confRestr;
	bool m_from_web_config = false;
	bool m_must_join = false;

public:

	InviteMulti_Task(const boost::shared_ptr<VS_PresenceService>& PresSRV, InviteInfo& i, bool cancel_and_redirect, const boost::weak_ptr<VS_ConfRestrictInterface>& confRestr, const bool from_web_config, const bool must_join)
		: m_presenceService(PresSRV), v(i), m_cancel_and_redirect(cancel_and_redirect), m_confRestr(confRestr), m_from_web_config(from_web_config), m_must_join(must_join)
	{}

	bool IsValid() const override { return !!v.conf && !!v.user; }

	void Run() {
		bool local = false;
		VS_UserPresence_Status status = USER_INVALID;
		VS_ConferenceDescription	cd;
		// 1- caller, 2 - other
		VS_UserData			ud1, ud2;
		VS_ParticipantDescription pd1;
		vs_user_id			user_id2=v.user;
		VS_Reject_Cause cause = PARTISIPANT_NOT_AVAILABLE_NOW;
		unsigned fwd_timeout(30000);
		VS_CallIDInfo ci2;

		// Start Invitaton process
		do {

			if (!g_storage->FindConference(SimpleStrToStringView(v.conf), cd) || (cd.m_type != CT_MULTISTREAM && cd.m_type != CT_INTERCOM)) {
				cause = INVALID_CONFERENCE; break;
			}

			if (m_cancel_and_redirect)
			{
				VS_Container rCnt;
				rCnt.AddValue(METHOD_PARAM, CONFERENCEDELETED_METHOD);
				rCnt.AddValue(NAME_PARAM, v.conf);
				rCnt.AddValueI32(RESULT_PARAM, CONFERENCE_DELETED_OK);
				rCnt.AddValueI32(CAUSE_PARAM, VS_ParticipantDescription::DISCONNECT_UNKNOWN);
				PostRequest(OurEndpoint(), v.user, rCnt);
			}

			//call id can contain escaped characters: #tel:+38097156%2f2764
			//here we will unescape them: #tel:+38097156%2f2764 -> #tel:+38097156/2764
			std::unique_ptr<CURL, CURL_deleter> curl(::curl_easy_init());
			if (curl) {
				int new_len = 0;
				std::unique_ptr<char, curl_free_deleter> unescaped_user_id2(::curl_easy_unescape(curl.get(), v.user.m_str, v.user.Length(), &new_len));
				if (unescaped_user_id2)
					user_id2 = unescaped_user_id2.get();
			}

			bool localOwner = false;
			if (!!v.src_user)
			{
				if (!m_from_web_config)
				{
					bool IsParticipant = g_storage->FindParticipant(SimpleStrToStringView(v.src_user), pd1);
					if (!IsParticipant || !(pd1.m_role == PR_LEADER || v.src_user == cd.m_owner)) {
						cause = INVALID_PARTISIPANT; break;
					}
				}
				localOwner = g_storage->FindUser(SimpleStrToStringView(v.src_user), ud1);
				if (!localOwner) {
					ud1.m_name = v.src_user;
					ud1.m_rights |= VS_UserData::UR_COMM_DIALER;		// allow roaming parts to use dialer
					ud1.m_appID = pd1.m_appID;
				}
			}
			if (strstr(user_id2, "/") != 0 && m_presenceService->IsRegisteredTransId(user_id2))
			{
				const auto our_endpoint = OurEndpoint();
				ci2.m_serverID = !!our_endpoint ? our_endpoint : std::string{};
				local = true;
				status = USER_AVAIL;
				LogParticipantInvite(v.conf, v.src_user, ud1.m_appID, user_id2, std::chrono::system_clock::now(), VS_ParticipantDescription::MULTIS_INVITE);
				break;
			}

			status = m_presenceService->ResolveWithForwarding(v.src_user, user_id2, &v.fwd_limit, m_cancel_and_redirect,
				&ci2, &ud1, &fwd_timeout);

			{
				const auto our_endpoint = OurEndpoint();
				local = our_endpoint == nullptr || *our_endpoint == 0 ? ci2.m_serverID.empty() : ci2.m_serverID == our_endpoint;
			}
			if (v.confKick)
			{
				v.confKick->RemoveConfKick(SimpleStrToStringView(v.conf), SimpleStrToStringView(v.user));
				v.confKick->RemoveConfKick(SimpleStrToStringView(v.conf), SimpleStrToStringView(user_id2));
			}

			if (v.confRestriction->IsVCS() && !local && !v.confRestriction->CheckInviteMulti_Roaming(user_id2))
			{
				VS_Container rCnt;
				rCnt.AddValue(METHOD_PARAM, INVITEREPLY_METHOD);
				rCnt.AddValue(USERNAME_PARAM, OurEndpoint());
				rCnt.AddValueI32(RESULT_PARAM, 1);
				PostRequest(0, v.src_user, rCnt);
				return ;
			}

			const char* other_bs = (!local) ? ci2.m_homeServer.c_str() : nullptr;

			//log here
			LogParticipantInvite(v.conf, v.src_user, ud1.m_appID, user_id2, std::chrono::system_clock::now(), VS_ParticipantDescription::MULTIS_INVITE, other_bs);

			if (localOwner && ud1.m_rating >= g_rating_th) {
				std::lock_guard<std::mutex> lock(g_storage->m_BookLock);
				VS_Storage::VS_UserBook* ib = g_storage->GetUserBook(ud1.m_name, AB_INVERSE);
				if (ib && !ib->IsInBook(user_id2)) {
					return;
				}
			}
		} while (false);

		if (status==USER_AVAIL) {
			// INVITEMULTI_METHOD

			vs_user_id username = v.alias ? v.alias : v.src_user;
			if (!username) {
				username = OurEndpoint();
				if (!!username) {
					char* ptr = strstr(username, "#");
					if (ptr) *ptr = 0;
				}
			}

			VS_Container rCnt;
			rCnt.AddValue(METHOD_PARAM, INVITETOMULTI_METHOD);
			rCnt.AddValue(CALLID2_PARAM, user_id2);
			rCnt.AddValue(CONFERENCE_PARAM, cd.m_name);
			rCnt.AddValue(USERNAME_PARAM, username);
			rCnt.AddValueI32(SEPARATIONGROUP_PARAM, ud1.m_SeparationGroup);
			rCnt.AddValue(NAME_PARAM, cd.m_name);// was callid
			rCnt.AddValue(TOPIC_PARAM, cd.m_topic);
			rCnt.AddValue(PASSWORD_PARAM, v.passwd);
			rCnt.AddValue(DISPLAYNAME_PARAM, ud1.m_displayName);
			rCnt.AddValue(CALLID_PARAM, v.alias ? v.alias : ud1.m_name);
			rCnt.AddValueI32(TYPE_PARAM, cd.m_type);
			rCnt.AddValueI32(SUBTYPE_PARAM, cd.m_SubType);
			if (v.m_caps.IsValid())
				rCnt.AddValue(CLIENTCAPS_PARAM, v.m_caps.Buffer(), v.m_caps.Size());
			if (!!v.server)
				rCnt.AddValue(SERVER_PARAM, v.server);
			if (m_must_join)
				rCnt.AddValue(MUSTJOIN_PARAM, m_must_join);

			if (!local) {
				rCnt.AddValue(NEEDCONNECTINFO_PARAM, true);
				PostRequest(ci2.m_serverID.c_str(), 0, rCnt, 20000); // 0 for user to send message on users server

				VS_Container ecnt;
				if (g_appServer->GetNetInfo(ecnt))
				{
					const char* to_user = (ci2.m_ml_cap == VS_MultiLoginCapability::UNKNOWN ||
						ci2.m_ml_cap == VS_MultiLoginCapability::SINGLE_USER)? user_id2.m_str: nullptr;
					VS_TransportRouterServiceHelper::PostRequest(ci2.m_serverID.c_str(), to_user, ecnt, 0, CONFIGURATION_SRV);
				}
			}
			else {
				g_storage->FindUser(SimpleStrToStringView(user_id2), ud2);
				if (ud2.m_protocolVersion > 30) // don't allow old users
					PostRequest(ci2.m_serverID.c_str(), user_id2, rCnt);
			}


			v.timeout = std::chrono::steady_clock::now() + (fwd_timeout ? std::chrono::seconds(fwd_timeout) : std::chrono::minutes(2));
			v.fwd_limit =  fwd_timeout ? v.fwd_limit : 0;
			v.user = user_id2;
			g_storage->StartInvitationProcess( v );
		}
		else
		{
			auto confRestr = m_confRestr.lock();
			v.server = confRestr ? confRestr->GetAnyBSbyDomain(SimpleStrToStringView(v.user)) : VS_SimpleStr();
			if (!local && v.server.IsEmpty())
				v.server = OurEndpoint();
			if (!!v.server)
			{
				VS_UserData to, from;
				std::chrono::system_clock::time_point time;
				VS_Container	cnt;
				g_storage->GetServerTime(time);

				if (g_storage->FindUser(SimpleStrToStringView(cd.m_owner), from)) {
					vs_user_id invited_user;
					if (g_storage->FindUser(SimpleStrToStringView(user_id2), to) && !!to.m_name)
						invited_user = to.m_name;
					else
						invited_user = user_id2;


					cnt.AddValue(METHOD_PARAM, SENDMAIL_METHOD);
					cnt.AddValue(CALLID_PARAM, from.m_name);
					cnt.AddValue(DISPLAYNAME_PARAM, from.m_displayName);
					cnt.AddValue(CALLID2_PARAM, invited_user);
					cnt.AddValue(USERNAME_PARAM, invited_user);		// for BS::LOCATION_SRV
					cnt.AddValue("app_name", from.m_appName);
					cnt.AddValue(TIME_PARAM, time);
					cnt.AddValue(CONFERENCE_PARAM, cd.m_name);
					cnt.AddValue(NAME_PARAM, cd.m_call_id);
					cnt.AddValue(TOPIC_PARAM,cd.m_topic);
					cnt.AddValue(PASSWORD_PARAM,cd.m_password);

					VS_TransportRouterServiceHelper::PostRequest(v.server, 0, cnt, LOG_SRV, LOCATE_SRV);
				}
			}
		}
	}
	void PostRequest(const char *server, const char *user, const VS_Container &cnt, unsigned long time = default_timeout)
	{
		VS_TransportRouterServiceHelper::PostRequest(server, user ,cnt,0, CONFERENCE_SRV,time,CONFERENCE_SRV);
	}
};

void VS_MultiConfService::Invite_Method(VS_Container &cnt)
{
	int32_t type = 0;
	cnt.GetValue(TYPE_PARAM, type);
	if (type==CT_MULTISTREAM || type==CT_INTERCOM)
	{
		size_t size;
		const void * buff = cnt.GetBinValueRef(CLIENTCAPS_PARAM, size);

		InviteInfo i;
		i.src_user = m_recvMess->SrcUser();
		i.confRestriction = m_confRestriction;
		i.confKick = this;
		i.conf = cnt.GetStrValueRef(CONFERENCE_PARAM);
		i.user = cnt.GetStrValueRef(CALLID_PARAM);
		i.passwd =  cnt.GetStrValueRef(PASSWORD_PARAM);
		i.conf_type = i.CONF_MULTY;
		i.m_caps.Set(buff, size);
		i.fwd_limit = 2;
		i.timeout = std::chrono::steady_clock::now();
		i.m_type = (VS_DirectConnect_Type)NO_DIRECT_CONNECT;
		i.alias = cnt.GetStrValueRef(ALIAS_PARAM);
		i.server = cnt.GetStrValueRef(SERVER_PARAM);

		VS_PoolThreadsTask* task = new InviteMulti_Task(m_presenceService, i, false, m_confRestriction, false, false);

		if (task->IsValid())
			PutTask(task, "InviteMulti", 30);
		else
			delete task;
	}
	else {
		VS_ConferenceService::Invite_Method(cnt);
	}
}

void VS_MultiConfService::InviteMulti_Method(VS_Container &cnt)
{
	const auto user = cnt.GetStrValueView(CALLID2_PARAM);
	VS_UserData ud;
	if (g_storage->FindUser(user, ud) && ud.m_protocolVersion > 30) // only for new clients!
		PostRequest(0, user.c_str(), cnt);
}


class ReqInviteMulti_Task: public VS_PoolThreadsTask, public VS_TransportRouterServiceHelper
{
	boost::shared_ptr<VS_PresenceService> m_presenceService;
	VS_SimpleStr			m_srcServer;		/// src user server
	vs_user_id				m_srcCallId;		/// src user CallId
	std::string				m_srcDName;			/// src user Display Name
	VS_SimpleStr			m_email;			/// owner of conf

public:
	ReqInviteMulti_Task(const boost::shared_ptr<VS_PresenceService>& PresSRV, const char* srcServer, const char* CallId, std::string &DName, const char* email)
		: m_presenceService(PresSRV), m_srcServer(srcServer), m_srcCallId(CallId), m_srcDName(DName), m_email(email) {}

	void Run() {
		VS_SimpleStr alias_call_id = m_email;
		VS_CallIDInfo ci;
		VS_UserPresence_Status status = m_presenceService->Resolve(m_email, ci, false);
		bool result = /*ci.m_serverID.operator !=(OurEndpoint()) &&*/ status==USER_MULTIHOST;

		if (result) {
			dprint3("MCS: ReqInv Task message to %s:%s\n", ci.m_serverID.c_str(), m_email.m_str);
			VS_Container rCnt;
			rCnt.AddValue(METHOD_PARAM, REQINVITE_METHOD);
			rCnt.AddValue(USERNAME_PARAM, m_srcCallId);
			rCnt.AddValue(CALLID_PARAM, m_srcCallId);
			rCnt.AddValue(DISPLAYNAME_PARAM, m_srcDName);
			rCnt.AddValue(ALIAS_PARAM, alias_call_id);
			const auto our_endpoint = OurEndpoint();
			if (!/*eq*/(our_endpoint == nullptr || *our_endpoint == 0 /*empty*/ ? ci.m_serverID.empty() : ci.m_serverID == our_endpoint)) {
				rCnt.AddValue(TRANSPORT_SRCUSER_PARAM, m_email);
				PostRequest(ci.m_serverID.c_str(), 0, rCnt, "32", CONFERENCE_SRV);
			} else {
				PostRequest(OurEndpoint(), m_email, rCnt, "32", CONFERENCE_SRV);
			}
		}
		else{
			dprint3("MCS: ReqInv Task fail, host %s, status=%d \n", m_email.m_str, status);
			VS_Container rCnt;
			rCnt.AddValue(METHOD_PARAM, REJECT_METHOD);
			rCnt.AddValue(CALLID_PARAM, m_email);
			rCnt.AddValue(CONFERENCE_PARAM, "");
			rCnt.AddValueI32(CAUSE_PARAM, PARTISIPANT_IS_BUSY);
			PostRequest(m_srcServer, m_srcCallId, rCnt, 0, CONFERENCE_SRV);
		}
	}
};


void VS_MultiConfService::ReqInvite_Method(const char *host, const char* user_id, const char* dn)
{
	if (!host || !*host) return;

	long cause = CONFERENCE_IS_BUSY;
	bool result = false;
	VS_ConferenceDescription cd;
	VS_UserData ud,udh;

	if (!user_id || !*user_id) return;
	// do not support for old clients
	if (atou_s(m_recvMess->AddString()) < 31) {
		// todo: return some message
		return;
	}

	do {
		VS_SimpleStr from_user = m_recvMess->SrcUser();

		if (!m_confRestriction->CheckInviteMulti_Roaming(from_user))
		{
			cause = CONFERENCE_IS_BUSY;
			break;
		}

		if (!!from_user && !g_storage->FindUser(user_id, ud)) {	// only from user (server bypass this)
			cause = INVALID_PARTISIPANT;
			break;
		}else if (!from_user) {
			ud.m_name = user_id;
			if(dn) ud.m_displayName = dn;
		}
		if (ud.m_rating >= g_rating_th) {
			std::lock_guard<std::mutex> lock(g_storage->m_BookLock);
			VS_Storage::VS_UserBook* ib = g_storage->GetUserBook(ud.m_name, AB_INVERSE);
			if (ib && !ib->IsInBook(host)) {
				cause = ud.m_protocolVersion > 33 ? REJECTED_BY_BADRATING : REJECTED_BY_PARTICIPANT;
				break;
			}
		}

		if (g_storage->FindUser(host, udh)) {
			if (g_storage->FindConferenceByUser(host, cd))
				if ((cd.m_type==CT_MULTISTREAM || cd.m_type==CT_INTERCOM) && cd.m_owner== host)
					result = g_storage->GetNumOfParts(cd.m_name)<cd.m_MaxParticipants;
				else cause = INVALID_CONFERENCE;
			else	result = true;
		}
		else {
			PutTask(new ReqInviteMulti_Task(m_presenceService, m_recvMess->SrcServer(), user_id, ud.m_displayName, host), "ReqInviteMulti", 30);
			return;
		}
	} while (false);

	if (result) {
		VS_Container rCnt;
		rCnt.AddValue(METHOD_PARAM, REQINVITE_METHOD);
		rCnt.AddValue(USERNAME_PARAM, ud.m_name);
		rCnt.AddValue(CALLID_PARAM, ud.m_name);
		rCnt.AddValue(DISPLAYNAME_PARAM, ud.m_displayName);
		PostRequest(OurEndpoint() ,host, rCnt); // if ok, host is local
	}
	else{
		VS_Container rCnt;
		rCnt.AddValue(METHOD_PARAM, REJECT_METHOD);
		rCnt.AddValue(CALLID_PARAM, host);
		g_storage->FindConferenceByUser(host, cd);
		rCnt.AddValue(CONFERENCE_PARAM, "");
		rCnt.AddValueI32(CAUSE_PARAM, cause);
		PostRequest(0, user_id, rCnt, 0, CONFERENCE_SRV);
	}
}

void VS_MultiConfService::InviteReply_Method(VS_Container &cnt)
{
	const char *conf = cnt.GetStrValueRef(CONFERENCE_PARAM);
	const auto user = cnt.GetStrValueView(NAME_PARAM);
	const char *from = cnt.GetStrValueRef(USERNAME_PARAM);

	if (!conf || !from)
		return;

	VS_ParticipantDescription	pd;

	if (g_storage->FindParticipant(user, pd) && pd.m_conf_id==conf)
		PostRequest(pd.m_server_id, pd.m_user_id, cnt);
}

void VS_MultiConfService::InviteUsers_Method(VS_Container &cnt)
{
	if(!m_recvMess->IsFromServer()&& !m_confRestriction->CheckSessionID(cnt.GetStrValueRef(SESSION_PARAM)))
		return;
	const auto cid = cnt.GetStrValueView(CONFERENCE_PARAM);
	if (cid.empty())
		return;
	bool must_join(false);
	cnt.GetValue(MUSTJOIN_PARAM, must_join);
	VS_ConferenceDescription cd;
	int32_t result = g_storage->FindConferenceByCallID(cid, cd);
	if (result != 0 && g_storage->FindConference(cid, cd))	// find by name (stream_id)
		result = 0;
	if (result != 0) {		// not running
		VS_Container tmp;
		int res = m_confRestriction->FindMultiConference(cid.c_str(), cd, tmp, 0);
		if (res != 0 && res != VSS_CONF_NOT_STARTED)
			return;
		if (cd.m_type != CT_MULTISTREAM && cd.m_type != CT_INTERCOM)
			return;
		if (cd.m_topic.empty())
			cd.m_topic.assign(cid.data(), cid.size());
		result = CreateStreamConference(vs::ignore<long>(), cd.m_owner, nullptr, cd);
	}
	else {
		dstream2 << "Conf with name exists, join to it  <" << cd.m_name.m_str << ">" <<
			", maxp=" << cd.m_MaxParticipants << ", maxcast=" << cd.m_MaxCast <<
			", t=" << cd.m_type <<
			", st=" << cd.m_SubType <<
			", is_pub=" << cd.m_public <<
			", res=" << result <<
			", topic= " << cd.m_topic << "\n";
	}

	if (result == 0)	// no error
	{
		std::string server = OurEndpoint();
		auto pos = server.find('#');
		if (pos != std::string::npos)
			server.erase(pos);

		cnt.Reset();
		while (cnt.Next())
		{
			if (strcasecmp(cnt.GetName(), CALLID_PARAM) == 0) {
				auto ptr = cnt.GetStrValueRef();
				if (!ptr)
					continue;
				std::string user = ptr;
				VS_ReplaceAll(user, "%5c", "\\");

				InviteInfo i;
				i.src_user = server.c_str(); // whithout #vcs!
				i.confRestriction = m_confRestriction;
				i.confKick = this;
				i.conf = cd.m_name;
				i.user = user.c_str();
				i.passwd = cd.m_password;
				i.conf_type = i.CONF_MULTY;
				i.fwd_limit = 2;
				i.timeout = std::chrono::steady_clock::now();
				i.m_type = (VS_DirectConnect_Type)NO_DIRECT_CONNECT;
				i.server = server.c_str();

				VS_PoolThreadsTask* task = new InviteMulti_Task(m_presenceService, i, false, m_confRestriction, true, must_join);

				if (task->IsValid())
					PutTask(task, "InviteMulti", 30);
				else
					delete task;
			}
		}

		if (!cd.m_name.IsEmpty())
		{
			cnt.AddValue(ID_PARAM, cd.m_name);
			PostRequest(OurEndpoint(), nullptr, cnt, nullptr, AUTH_SRV);
		}
	}
}

void VS_MultiConfService::SetMyLStatus_Method(VS_Container &cnt)
{
	const char* call_id = cnt.GetStrValueRef(CALLID_PARAM);
	const char* conf_name = cnt.GetStrValueRef(CONFERENCE_PARAM);
	int32_t lstatus(0);
	if (!cnt.GetValue(LSTATUS_PARAM, lstatus))
		return ;
	if (!call_id || !*call_id || !conf_name || !*conf_name)
		return ;

	dprint3("SetMyLStatus: conf=%s, from=%s, lstatus=%d\n", conf_name, call_id, lstatus);
	VS_ParticipantDescription pd;
	if (!g_storage->FindParticipant(call_id, pd))
		return ;
	pd.m_lstatus = lstatus;
	if (!g_storage->UpdateParticipant(pd))
		return ;
	UpdateLStatus(conf_name, call_id, lstatus);
}

void VS_MultiConfService::ClearAllLStatuses_Method(VS_Container &cnt)
{
	const char* call_id = cnt.GetStrValueRef(CALLID_PARAM);
	const char* conf_name = cnt.GetStrValueRef(CONFERENCE_PARAM);
	if (!call_id || !*call_id || !conf_name || !*conf_name)
		return ;
	dprint3("ClearAllLStatuses: conf=%s, from=%s\n", conf_name, call_id);
	VS_ConferenceDescription cd;
	VS_ParticipantDescription pd;
	if (!g_storage->FindConference(conf_name, cd) || !g_storage->FindParticipant(call_id, pd))
		return ;
	if (strcasecmp(cd.m_owner,call_id)!=0 && pd.m_role != PR_LEADER)
		return ;
	UpdateAllLStatuses(conf_name, true);
}

void VS_MultiConfService::UpdateAllLStatuses(const char* conf_name, bool reset, const char* only_for_call_id)
{
	if (!conf_name || !*conf_name)
		return ;

	dprint4("UpdateAllLStatuses: conf=%s, reset=%d, only_for=%s\n", conf_name, reset, only_for_call_id);
	VS_ParticipantDescription *parts = 0;
	int NumOfPart = 0;
	if ((NumOfPart = g_storage->GetParticipants(conf_name, parts))>0) {
		int32_t hash(0);
		VS_Container rCnt;
		rCnt.AddValue(METHOD_PARAM, SEARCHADDRESSBOOK_METHOD);
		rCnt.AddValueI32(ADDRESSBOOK_PARAM, AB_LSTATUS);
		rCnt.AddValueI32(RESULT_PARAM, NumOfPart);
		rCnt.AddValueI32(CAUSE_PARAM, SEARCH_DONE);
		rCnt.AddValue(HASH_PARAM, hash);
		for (int i = 0; i<NumOfPart; i++)
		{
			if (reset)
			{
				parts[i].m_lstatus = 0;
				g_storage->UpdateParticipant(parts[i]);
			}

			rCnt.AddValue(CALLID_PARAM, parts[i].m_user_id);
			rCnt.AddValueI32(LSTATUS_PARAM, parts[i].m_lstatus);
		}

		VS_BinBuff buff; buff.Set(rCnt);

		for (int i = 0; i<NumOfPart; i++)
			if (!only_for_call_id || !*only_for_call_id || (!parts[i].m_user_id.IsEmpty() && !strcasecmp(parts[i].m_user_id, only_for_call_id)))
				PostRequest(parts[i].m_server_id, parts[i].m_user_id, buff.Buffer(), buff.Size(), 0, ADDRESSBOOK_SRV);
		delete[] parts;
	}
}

void VS_MultiConfService::UpdateLStatus(const char* conf_name, const char* call_id, unsigned long lstatus)
{
	if (!conf_name||!*conf_name||!call_id||!*call_id)
		return ;
	dprint4("UpdateLStatus: conf=%s, call_id=%s, lstatus=%ld\n", conf_name, call_id, lstatus);
	int32_t hash(0);
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, SEARCHADDRESSBOOK_METHOD);
	rCnt.AddValueI32(ADDRESSBOOK_PARAM, AB_LSTATUS);
	rCnt.AddValueI32(RESULT_PARAM, 1);
	rCnt.AddValueI32(CAUSE_PARAM, SEARCH_DONE);
	rCnt.AddValue(HASH_PARAM, hash);
	rCnt.AddValue(CALLID_PARAM, call_id);
	rCnt.AddValueI32(LSTATUS_PARAM, lstatus);
	VS_BinBuff buff; buff.Set(rCnt);

	std::vector<tPartServer> part_server;
	g_storage->GetParticipants(conf_name, part_server);
	if (buff.Size())
		for (auto &i: part_server)
			PostRequest(i.second, i.first, buff.Buffer(), buff.Size(), 0, ADDRESSBOOK_SRV);
}

std::chrono::steady_clock::time_point VS_MultiConfService::Now()
{
	return (m_now == std::chrono::steady_clock::time_point())? std::chrono::steady_clock::now(): m_now;
}

bool VS_MultiConfService::Timer( unsigned long tickcount )
{
	VS_ConferenceService::Timer( tickcount );

	if (tickcount - m_last_sr_sysload_update_time > 1000) {
		if (m_load.size() > 0) {
			dstream2 << "Set sysload, N = " << m_load.size();
			m_sr->SetParticipantSystemLoad(std::move(m_load));
			m_last_sr_sysload_update_time = tickcount;
		}
	}

	if (tickcount - m_invite_check_time > 1000) {
		OnTimer_CheckInvites();
		m_invite_check_time = tickcount;
	}
	auto now = Now();
	if (now - m_last_fecc_pending_time > std::chrono::seconds(5)) {
		m_last_fecc_pending_time = now;
		for (auto it = m_fecc_pending.begin(); it != m_fecc_pending.end(); )
		{
			auto& from_map = it->second;
			for (auto it2 = from_map.begin(); it2 != from_map.end(); )
			{
				auto& p = it2->second;
				if (p.first == eFeccRequestType::REQUEST_ACCESS &&
					now - p.second > std::chrono::seconds(30)) {
					auto from = it->first.c_str();
					auto to = it2->first.c_str();
					dstream4 << "Generate FECC::DENY_BY_TIMEOUT_ACCESS from=" << from << ", to=" << to;
					VS_Container cnt;
					cnt.AddValue(METHOD_PARAM, FECC_METHOD);
					cnt.AddValueI32(TYPE_PARAM, eFeccRequestType::DENY_BY_TIMEOUT_ACCESS);
					cnt.AddValue(FROM_PARAM, from);
					cnt.AddValue(TO_PARAM, to);
					PostRequest(nullptr, to, cnt);
					it2 = from_map.erase(it2);
				}
				else
					++it2;
			}
			if (from_map.empty())
				it = m_fecc_pending.erase(it);
			else
				++it;
		}
	}
	return true;
}

void VS_MultiConfService::OnTimer_CheckInvites()
{
	std::vector<InviteInfo> invites;
	g_storage->GetTimedoutInvites( invites,  InviteInfo::CONF_MULTY);
	for (unsigned i = 0; i < invites.size(); i++)
		if (invites[i].fwd_limit > 0)
			PutTask(new InviteMulti_Task(m_presenceService, invites[i],true,m_confRestriction,false,false), "InviteMultiOnTmr");
}

void VS_MultiConfService::UserRegistrationInfo_Method(VS_Container &cnt)
{
	const auto from = cnt.GetStrValueView(FROM_PARAM);
	VS_SimpleStr alias = cnt.GetStrValueRef(ALIAS_PARAM);
	int32_t *cmd = cnt.GetLongValueRef(CMD_PARAM);
	if (!from.empty() && cnt.GetStrValueRef(DISPLAYNAME_PARAM))
	{
		VS_UserData ud;
		if (g_storage->FindUser(from, ud))
		{
			const char *pDn = cnt.GetStrValueRef(DISPLAYNAME_PARAM);
			if (pDn) ud.m_displayName = pDn;
			if (!!alias)
				ud.m_callId = alias;
			g_storage->UpdateUser(from, ud);

			if (cmd && *cmd)
			{
				VS_SimpleStr user = cnt.GetStrValueRef(NAME_PARAM);
				if (!!user)
				{
					VS_Container rCnt;
					rCnt.AddValue(METHOD_PARAM, USERREGISTRATIONINFO_METHOD);
					rCnt.AddValue(USERNAME_PARAM, from);
					rCnt.AddValue(DISPLAYNAME_PARAM, ud.m_displayName);
					if (!!alias)
						rCnt.AddValue(CALLID_PARAM, alias);
					else
						rCnt.AddValue(CALLID_PARAM, from);
					PostRequest(0, user.m_str, rCnt);
				}
			}
		}
	}
}

void VS_MultiConfService::OnRTSPAnnounceStatusReport(string_view conf_name, string_view announce_id, bool is_active, string_view reason)
{
	VS_ConferenceDescription cd;
	if (!g_storage->FindConference(conf_name, cd))
	{
		dstream4 << "OnRTSPAnnounceStatusReport: conf=" << conf_name << " announce=" << announce_id << ": conf not found";
		return;
	}

	const auto announce_it = cd.m_rtspAnnounces.find(announce_id);
	if (announce_it == cd.m_rtspAnnounces.end())
	{
		dstream4 << "OnRTSPAnnounceStatusReport: conf=" << conf_name << " announce=" << announce_id << ": announce not found";
		return;
	}

	announce_it->second.active = is_active;
	announce_it->second.reason = std::string(reason);
	g_storage->UpdateConference(cd);

	if (!m_confRestriction->UpdateConference_RTSPAnnounce(cd, announce_id))
	{
		dstream4 << "OnRTSPAnnounceStatusReport: conf=" << conf_name << " announce=" << announce_id << ": update failed";
		return;
	}
}
