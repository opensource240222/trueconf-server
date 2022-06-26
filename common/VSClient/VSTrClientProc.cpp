/**
 **************************************************************************
 * \file VSTrClientProc.cpp
 * (c) 2002-2014 TrueConf
 * \brief Implementation of transport client processor.
 *
 * \b Project Client
 * \author SMirnovK
 * \date 18.12.2002
 *
 ****************************************************************************/

/*@{ \name Network functions includes */
#include "std/cpplib/utf8_old.h"
#include "../std/cpplib/VS_EndpointId.h"
#include "../std/cpplib/VS_RegistryKey.h"
#include "..\std\cpplib\VS_RcvFunc.h"
#include "..\std\cpplib\VS_WideStr.h"
#include "std/cpplib/StrFromVariantT.h"
#include "../std/cpplib/VS_Utils.h"
#include "..\std\cpplib\diskid.h"
#include "std-generic/cpplib/deleters.h"
#include "std/cpplib/md5.h"

#include "../streams/Client/VS_StreamClient.h"
#include "../acs/ConnectionManager/VS_ConnectionManager.h"
#include "../acs/Lib/VS_AcsLib.h"
#include "../WinFirewallManager/VS_FirewallMgr.h"
#include "../UPnPLib\VS_UPnPInterface.h "
#include "VS_ConnectionsCheck.h"
#include "../acs/Lib/VS_IPPortAddress.h"
#include "../sudis/unit_solutions/sudis.h"

/*@}*/

/*@{ \name General includes */
#include "VS_ApplicationInfo.h"
#include "VSClientBase.h"
#include "VSTrClientProc.h"
#include "VS_Dmodule.h"
#include "VS_MiscCommandProc.h"
#include "Transcoder/AudioCodec.h"
#include "Transcoder/VideoCodec.h"
#include "VS_SysBenchmarkWindows.h"
#include "../std/cpplib/VS_VideoLevelCaps.h"
#include "ScreenCapturerFactory.h"
#include <boost/bind.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <Strsafe.h>
#include <shellapi.h>
#include "Transcoder/GetTypeHardwareCodec.h"
#include "Transcoder/VS_RetriveVideoCodec.h"
/*@}*/

#include <ctime>
#include <iomanip>
#include "net/DNSUtils/VS_DNSTools.h"

void VSGetDirectDrawInfo(VS_SimpleStr &prop);

/*@{ \name Registered servises messages*/
#define WM_USERLOGIN				WM_APP+2	//< User Login Service Message
#define WM_ENDPOINTREG				WM_APP+3	//< Endpoint Registration Service Message
#define WM_CONFERENCE				WM_APP+4	//< Conference Service Message
#define WM_USERPESENCE				WM_APP+5	///< User Presence Service Message
#define WM_NETWORKCONF				WM_APP+6	///< Network Configuration Service Message
#define WM_PING						WM_APP+7	///< Ping Service Message
#define WM_CHAT						WM_APP+8	///< Chat Service Message
#define WM_CLIENT					WM_APP+9	///< Client Service Message
#define WM_TORRENT					WM_APP+10	///< Torrent Message
#define WM_MAX_SERIVCE_MESSAGE		WM_TORRENT	///< Torrent Message

/*@}*/
/*@{ \name Timeouts */
#define SEND_TIMEOUT				20*1000		///< Time to message return as notify
#define TESTNHP_TIMEOUT				6*1000		///< Time to test direct connection for connect
#define TESTDCONNECT_TIMEOUT		4*1000		///< Time to test direct connection for connect
#define TESTDACCEPT_TIMEOUT			7*1000		///< Time to test direct connection for accept
#define TIMEOTS_WAIT_TODISCONNECT	2			///< Numper of timeouts (or notify) to declare disconnect from server
#define STREAM_CONNECT_TIMEOUT		20*1000		///< Time to strems connected
#define TESTSERVERCONNECTS_TIMEOUT	4*1000		///< Time to test connects to other server in conference
/*@}*/

/*@{ \name  Notifiy message */
#define WM_NOTIFY_GUI				WM_USER+16	///< Message returned to gui window showing status change
#define WM_NOTIFY_CALL				WM_USER+17  ///< Message returned to gui window showing new conf success
#define WM_NOTIFY_GUI_CPY			WM_USER+18	///< Message for VZOchat7 copy container
/*@}*/

const char _InputBandwidthName[]="InputBandwidth";

/// if true do not try dirrect connection. Set by GUI
DWORD CVSTrClientProc::m_dwSuppressDirectCon = 0;

/// if true Nat Hole Punching test used
DWORD CVSTrClientProc::m_dwUseNhp = 0;

/// if true NHP used if passed OK
DWORD CVSTrClientProc::m_dwForceNhp = 0;

/// if true set Qos for sender stream
DWORD CVSTrClientProc::m_dwMarkQos = 0;

/// NHP stream
boost::shared_ptr<VS_StreamClientTransmitter> CUserDesk::Trmtr;
VS_Lock						CUserDesk::Trmtr_lock;

/// UDP Direct Streams for ipv4 and ipv6 address family
boost::shared_ptr<VS_StreamClientTransmitter> CUserDesk::DirectUdpTrmtr;
//VS_Lock						CUserDesk::DirectUdp_lock;

// Somebody decided that it's a good idea to pass a pointer to a C++ class (VS_Container) to Delphi code...
// To fix that and allow changing VS_Container without breaking external code we return relevant parts of VS_Container as plain struct.
struct VS_ContainerData
{
	VS_ContainerData(const VS_Container& cnt)
		: value(cnt.m_value)
		, valueSize(cnt.m_valueSize)
		, valueCnt(cnt.m_valueCnt)
	{
	}

private:
	// All members are private to prevent manipulation.
	void* unused_1 /*vtable*/ = nullptr;
	void* unused_2 /*currKey*/ = nullptr;
	void* unused_3 /*currValue*/ = nullptr;
	void* value;
	unsigned long valueSize;
	unsigned long valueCnt;
	unsigned long unused_4 /*memory size*/ = 0;

};

/**
******************************************************************************
* Constructor. Set to zero variables
* \date    09-01-2003
******************************************************************************/
CVSTrClientProc::CVSTrClientProc(CVSInterface* pParentInterface):
CVSInterface("Protocol", pParentInterface), m_MyThreadID(0), m_StoredStatus(USER_STATUS_UNDEF), m_PartsTrack(5000)
{
	net::dns_tools::init_loaded_options();
	m_bwt = 0;
	m_ExternalWindow = 0;
	m_ExternalThread = 0;
	m_AutoConnect = false;
	m_CleanOnSubscribe = true;
	/// set default tracks
	const stream::Track tracks[] = {
		stream::Track::audio,
		stream::Track::video,
		stream::Track::old_command,
		stream::Track::data,
		stream::Track::command,
	};
	SetTracks(tracks, sizeof(tracks)/sizeof(tracks[0]));

	for (int i = 0; i<TIME_MAX_TIMER; i++)
		m_TimerObjects[i] = 0;
	/*****************************************/
#ifdef _BUILD_CONFERENDO
	_variant_t vr(512);
	ReadParam((char*)_InputBandwidthName, &vr.GetVARIANT());
	if (int(vr)<32 || int(vr)>512)
		vr = 512;
#else
	_variant_t vr(1024);
	ReadParam((char*)_InputBandwidthName, &vr.GetVARIANT());
	if (int(vr)<32 || int(vr)>10240)
		vr = 1024;
#endif
	m_Status.MyInfo.ClientCaps.SetBandWRcv(vr);
	m_Status.MyInfo.ClientCaps.SetClientFlags(VS_ClientCaps::ClientFlags::HAS_HANGUP_FLAGS);
	/*****************************************/
	m_hRestrictMedia = CreateEvent(0, 0, 0, 0);
	m_NhpEvent = CreateEvent(0, 1, 0, 0);
	m_DirectUdpConnEvent = CreateEvent(0,1,0,0);

	m_LogoutEvent = CreateEvent(0, 0, 0, 0);
	m_SrvProtocolVersion = 1;
	m_EndpointType = 0;
	m_SrvFlags = 0;

	*(unsigned __int64*)&m_sec_token_expiry=0;
	m_sec_pack=0; m_sec=0;m_sec_fail=false;

	VS_RegistryKey key(true, REG_CurrentConfiguratuon, false, true);
	key.GetValue(&m_dwUseNhp, 4, VS_REG_INTEGER_VT, "UseNhp");
	key.GetValue(&m_dwForceNhp, 4, VS_REG_INTEGER_VT, "ForceNhp");
	key.GetValue(&m_dwMarkQos, 4, VS_REG_INTEGER_VT, "MarkQos");
	m_DirectPort = VS_GenKeyByMD5()%60000+5000;
	if (!key.GetValue(&m_DirectPort, 4, VS_REG_INTEGER_VT, "DirectPort"))
		key.SetValue(&m_DirectPort, 4, VS_REG_INTEGER_VT, "DirectPort");

	char buff[256] = {0};
	// read last user
	if (m_pKey->GetValue(buff, 255, VS_REG_STRING_VT, "Lastlogin") >0)
		m_LastLogin = buff;
	// read server key
	if (m_pKey->GetValue(buff, 255, VS_REG_STRING_VT, REG_Key)>0)
		m_AutoLoginKey = buff;

	VS_ReadDiskandOsKeys(m_DiskOS);

	// make up app id
	VS_ReadClientHardwareKey(m_AppId);
	m_AutoLogin = !!m_AutoLoginKey;

	m_Status.MyInfo.SetNHPStream(boost::shared_ptr<VS_StreamClientRtp>(new VS_StreamClientRtp));
	m_Status.MyInfo.SetUDPStream(boost::shared_ptr<VS_StreamClientRtp>(new VS_StreamClientRtp));
	m_podium_time = m_podium_start_t = 0;
	m_HQAuto = 1;
	key.GetValue(&m_HQAuto, 4, VS_REG_INTEGER_VT, "HQ Auto");
	m_SndStereo = 0;
	key.GetValue(&m_SndStereo, 4, VS_REG_INTEGER_VT, "SndStereo");
	if (m_SndStereo > 0) m_HQAuto = 0;
	m_RcvStereo = 0;
	key.GetValue(&m_RcvStereo, 4, VS_REG_INTEGER_VT, "RcvStereo");
	m_enchSLayers = 1;
	key.GetValue(&m_enchSLayers, 4, VS_REG_INTEGER_VT, "enchSLayers");
	m_loadSystem = 70;
	m_bTranscoder = false;
	m_client_type = CT_SIMPLE_CLIENT;
	m_ConnectState = CS_UNKNOWN;

    pFileSendStartedCB = NULL;
    pFileDownloadStatusCB = NULL;
}

/**
*****************************************************************************
* Destructor. Remove temporary endpoint(s)
* \date    09-01-2003
******************************************************************************/
CVSTrClientProc::~CVSTrClientProc()
{
	VS_UPnPInterface* unpn = VS_UPnPInterface::Instance();
	if (unpn) unpn->PrepareToDie();

	/*****************************************/
	_variant_t var(m_Status.MyInfo.ClientCaps.GetBandWRcv());
	WriteParam((char*)_InputBandwidthName, &var.GetVARIANT());
	/*****************************************/

	if (m_bwt) delete m_bwt;

	DisconnectProcedure();
	DesactivateThread();

	if (m_hRestrictMedia) CloseHandle(m_hRestrictMedia);
	if (m_NhpEvent) CloseHandle(m_NhpEvent);
	if(m_DirectUdpConnEvent) CloseHandle(m_DirectUdpConnEvent);
	if (m_LogoutEvent) CloseHandle(m_LogoutEvent);

	if(m_sec && *(unsigned __int64*)&m_sec_token_expiry!=0)
		(m_sec->FreeCredentialsHandle)(&m_sec_token);
	if(m_sec_pack)
		(m_sec->FreeContextBuffer)(m_sec_pack);
}


/**
*****************************************************************************
* Copmpose transport message from container and params. Send it.
* \date    01-02-2008
******************************************************************************/
DWORD CVSTrClientProc::ComposeSend(VS_Container &cnt, const char* service, const char* server = 0, const char *user = 0, unsigned long timeout = SEND_TIMEOUT)
{
	void* body;
	size_t bodySize;
	if (m_ConnectState == CS_CONNECTED && cnt.SerializeAlloc(body, bodySize)) {
		VS_ClientMessage tMsg(service, VSTR_PROTOCOL_VER, 0, service, timeout, body, bodySize, user, 0, server);
		free(body);
		return tMsg.Send();
	}
	else
		return 0;
}

/**
******************************************************************************
* Return Broker name within conference name
* \date    08-09-2003
******************************************************************************/
const char * CVSTrClientProc::ReturnBroker(const char* conf)
{
	const char * broker = VS_GetConfEndpoint(conf);
	if (!broker) broker = m_CurrBroker;
	return broker;
}

/**
******************************************************************************
* Read Currnet Network Configuration property
* \date    29-09-2003
******************************************************************************/
bool CVSTrClientProc::ReadPropNetConfig()
{
	char PropCurrConfig[1024] = {0};
	char buff[1024] = {0};
	auto tcp = net::endpoint::ReadConnectTCP(1, m_CurrBroker.m_str);
    if (!tcp) return false;
	const char *netType = 0;
	switch(VSProxySettings::SetNetType(-1)) // read
	{
    case 1:  netType = "Dial-up";		break;
	case 2:  netType = "GPRS/EDGE";		break;
	case 3:  netType = "DSL";			break;
	case 4:  netType = "Cable";			break;
	case 5:  netType = "T1";			break;
	case 6:  netType = "Wi-Fi";			break;
	default: netType = "I dont know";	break;
	}
	sprintf(buff, "Type:      %s\n", netType); strcat(PropCurrConfig, buff);
	sprintf(buff, "Broker:    %s\n", m_CurrBroker.m_str); strcat(PropCurrConfig, buff);
	sprintf(buff, "Host:      %s\n", tcp->host.c_str()); strcat(PropCurrConfig, buff);
	sprintf(buff, "Port:      %d\n", tcp->port); strcat(PropCurrConfig, buff);
	sprintf(buff, "Protocol:  %s\n", tcp->protocol_name.c_str()); strcat(PropCurrConfig, buff);
	if (!tcp->socks_host.empty())
	{
		sprintf(buff, "ProxyHost: %s\n", tcp->socks_host.c_str()); strcat(PropCurrConfig, buff);
		sprintf(buff, "ProxyPort: %d\n", tcp->socks_port); strcat(PropCurrConfig, buff);
		sprintf(buff, "SocksVer : %d\n", tcp->socks_version); strcat(PropCurrConfig, buff);
	}
	if (!tcp->http_host.empty())
	{
		sprintf(buff, "ProxyHost: %s\n", tcp->http_host.c_str()); strcat(PropCurrConfig, buff);
		sprintf(buff, "ProxyPort: %d\n", tcp->http_port); strcat(PropCurrConfig, buff);
	}

    VS_IPPortAddress addr;
    if (addr.SetIPFromHostName(tcp->host.c_str())) {
        char buf[128];
        if (addr.GetHostByIp(buf, sizeof(buf))) {
            m_server_ip = buf;
            m_server_port = tcp->port;
        }
    }

	if (!m_bTranscoder) {
		VS_SimpleStr trk;
		if (GetProperties("trackers", trk) == ERR_OK) {
			char *buf = trk;
			m_trackers.clear();
			char *p = strtok(buf, ";");
			while (p != NULL) {
				m_trackers.emplace_back(p);
				p = strtok(NULL, ";");
			}

			// let libtorrent register uploads on new trackers
			for (auto &item : m_items) {
				m_torrent->GetControl(VS_FileTransfer::hash_str_from_uri(item.second.magnet)).AddTrackers(m_trackers);
			}
		}
		TrSetLimits(m_dw_lim, m_up_lim);
	}

	// hashed props
	unsigned int hashOld = 0, hashNew = 0;
	// hashOld
	VS_RegistryKey key(true, REG_CurrentConfiguratuon, false);
	key.GetValue(&hashOld, 4, VS_REG_INTEGER_VT, "HardwareHash");
	// hashNew
	VS_SimpleStr total;
	total+= m_PropSysConfig_OS;
	total+= m_PropSysConfig_Processor;
	total+= m_PropDirectX;
	total+= APP_version;
	total+= VSTR_PROTOCOL_VER;
	total+= m_PropCurrConfig;
	total+= m_PropAudioCupture;
	total+= m_PropVideoCupture;
	total+= m_PropAudioRender;
	hashNew = VS_SimpleChkSum(total, strlen(total));

	SetEpProperties("sys_conf", m_PropSysConfig_OS, false);
	SetEpProperties("Processor", m_PropSysConfig_Processor, false);
	SetEpProperties("Direct X", m_PropDirectX, false);
	SetEpProperties("app_name", GUI_version, false);
	SetEpProperties("version", APP_version, false);
	SetEpProperties("prot_version", VSTR_PROTOCOL_VER, false);
	SetEpProperties("type", _itoa(m_EndpointType, buff, 10), false);
	SetEpProperties("Hardware Config", m_PropCurrConfig, false);
	SetEpProperties("Audio Capture", m_PropAudioCupture, false);
	SetEpProperties("Video Capture", m_PropVideoCupture, false);
	SetEpProperties("Audio Render", m_PropAudioRender, false);
	key.SetValue(&hashNew, 4, VS_REG_INTEGER_VT, "HardwareHash");

	// send and clear, might not be send at en of net test
	if (*m_bwt->m_Out) {
		SetEpProperties("Network Test", m_bwt->m_Out, false);
		m_bwt->m_Out[0] = 0;
	}
	SetEpProperties("Performance", m_PropPerformance, false);
	// always send
	SetEpProperties("DiskOs", m_DiskOS, false);
	SetEpProperties("Network Info", PropCurrConfig, true);

	return true;
}

/**
******************************************************************************
* Read Currnet Configuration properties
* \date    29-09-2003
******************************************************************************/
bool CVSTrClientProc::ReadProps()
{
	std::unique_ptr<char, free_deleter> str;

	m_PropCurrConfig.Empty();

	VS_RegistryKey key(true, "Client");
	key.GetValue(str, VS_REG_STRING_VT, "RenderAudioName");
	m_PropCurrConfig+="Audio Render :    ";
	if (str && *str)
		m_PropCurrConfig += str.get();
	str = nullptr;

	VS_RegistryKey key1(true, "Client\\AudioCaptureSlot");
	key1.GetValue(str, VS_REG_STRING_VT, "DeviceName");
	m_PropCurrConfig+= "\nAudio Capture:    ";
	if (str && *str)
		m_PropCurrConfig += str.get();
	str = nullptr;

	VS_RegistryKey key2(true, "Client\\VideoCaptureSlot");
	key2.GetValue(str, VS_REG_STRING_VT, "DeviceName");
	m_PropCurrConfig+= "\nVideo Capture:    ";
	if (str && *str)
		m_PropCurrConfig += str.get();
	str = nullptr;

	m_PropCurrConfig+="\n";
	VSGetSystemInfo_OS(m_PropSysConfig_OS);
	VSGetSystemInfo_Processor(m_PropSysConfig_Processor);
	VSGetDirectDrawInfo(m_PropDirectX);

	VS_SysBenchmarkWindows benchmark;
	int benchMBps = benchmark.GetBenchMBps(ENCODER_SOFTWARE);
	int sndMBps = benchmark.GetSndMBps(ENCODER_SOFTWARE);
	int rcvMBps = benchmark.GetRcvMBps(ENCODER_SOFTWARE);
	int rcvGroupMBps = benchmark.GetRcvGroupMBps(ENCODER_SOFTWARE);
	VSGetSystemInfo_Performance(m_PropPerformance, benchMBps, sndMBps, rcvMBps, rcvGroupMBps);

	return true;
}

/**
******************************************************************************
* Set Online status, set home broker
* \date    08-08-2003
******************************************************************************/
DWORD CVSTrClientProc::SendConfStat(TConferenceStatistics *snd, TConferenceStatistics *rcv)
{
	char buff[1024]; *buff = 0;
	int bytes_s = snd->avg_send_bitrate >> 10,
		bytes_r = rcv->avg_rcv_bitrate >> 10;
	// new statistic
	snd->loss_rcv_packets = rcv->loss_rcv_packets;
	snd->avg_jitter = rcv->avg_jitter;
	snd->participant_time /= 1000;
	if (m_podium_time != 0) {
		snd->broadcast_time = m_podium_time / 1000;
		if (m_podium_start_t != 0) {
			snd->broadcast_time += (timeGetTime() - m_podium_start_t) / 1000;
		}
	} else {
		if (snd->avg_send_bitrate > 0) snd->broadcast_time = snd->participant_time;
	}
	if (snd->broadcast_time != 0)
		snd->avg_send_bitrate = ((snd->avg_send_bitrate >> 10) << 3) / (snd->broadcast_time + 1);
	else
		snd->avg_send_bitrate = ((snd->avg_send_bitrate >> 10) << 3) / (snd->participant_time + 1);
	snd->avg_rcv_bitrate  = ((rcv->avg_rcv_bitrate >> 10) << 3) / (snd->participant_time + 1);
	snd->size_of_stat = sizeof(TConferenceStatistics);

	//old statistic
	int time_s, time_r;
	time_s = time_r = snd->participant_time;
	int bitrate_s = bytes_s*8/(time_s+1);
	int bitrate_r = bytes_r*8/(time_r+1);
	sprintf(buff, "Name        %s\nBytes   s/r %5d/%-5d kB\nBitrate s/r %5d/%-5d kbit\nDuration    %02d:%02d:%02d",
		m_LastConfName.m_str, bytes_s, bytes_r, bitrate_s, bitrate_r, (time_s/3600)%24, (time_s/60)%60, time_s%60);
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, LOGPARTSTAT_METHOD);
	cnt.AddValue(CONFERENCE_PARAM,(const char*)m_LastConfName);
	cnt.AddValue(CALLID_PARAM,(const char*)m_Status.MyInfo.UserName);
	cnt.AddValueI32(BYTES_SENT_PARAM, bytes_s);
	cnt.AddValueI32(BYTES_RECEIVED_PARAM, bytes_r);
	cnt.AddValue(CONF_BASE_STAT_PARAM, snd, snd->size_of_stat);
	ComposeSend(cnt, LOG_SRV, ReturnBroker(m_LastConfName));

	return SetEpProperties("Last Conf", buff);
}

/**
******************************************************************************
* Update picture for public conference
* \date    29-09-2008
******************************************************************************/
void CVSTrClientProc::SendConferncePicture(void* buff, unsigned long size)
{
	if (!buff || !size)
		return;
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, UPDATECONFERENCEPIC_METHOD);
	cnt.AddValue(CONFERENCE_PARAM, m_Status.ConfInfo[0].Conference);
	cnt.AddValue(PICTURE_PARAM, buff, size);

	ComposeSend(cnt, LOG_SRV, m_CurrBroker);
}


/**
******************************************************************************
* Set Online status, set home broker
* \date    08-08-2003
******************************************************************************/
void CVSTrClientProc::SetOnlineStatus(bool set)
{
	if (set) {
		// set
		if (!(m_Status.dwStatus&STATUS_SERVAVAIL)) {
			// we are online
			m_Status.dwStatus = STATUS_SERVAVAIL;
			// write to registry
			VS_WriteAS(m_CurrBroker);
			char defServer[256] = {0};
			if (!VS_ReadAS(defServer, true) || !m_ServerList.IsInList(defServer))
				VS_WriteAS(m_CurrBroker, true);
			// notify about server name
			PostProc(VSTRCL_ERR_SERVNTVALID);
			// send query for properties
			GetAppProperties();
			// renew timer to mesure server distance
			SetTimerObject(TIME_CHECKSRV);
		}
	}
	else {
		// clear
		if (m_Status.dwStatus&STATUS_SERVAVAIL) {
			// clear conferences info
			for (int i = 0; i<m_Status.MAX_CONFINFO; i++)
				m_Status.ConfInfo[i].Clean();
		}
		// set offline
		m_Status.dwStatus = 0;
	}
}

/**
******************************************************************************
* Set or clear Incall status
* \param	set		- true for set flag
* \date    08-08-2003
******************************************************************************/
void CVSTrClientProc::SetIncallStatus(bool set, DWORD ret, int invNum) {
	if (set) {
		m_Status.dwStatus&=~STATUS_INCALL_PROGRESS;
		m_Status.dwStatus|=STATUS_INCALL;
		SetTimerObject(TIME_INCALL);
		PostProc(ret, invNum);
	}
	else {
		RemoveTimerObjects(TIME_INCALL);
		m_Status.dwStatus&=~STATUS_INCALL;
		PostProc(VSTRCL_CONF_CALL);
	}
}

/**
******************************************************************************
* Set or clear flaq for "Request Invite" event
* \param	set		- true for set flag
* \date    08-08-2003
******************************************************************************/
void CVSTrClientProc::SetReqInviteStatus(bool set){
	if (set) {
		m_Status.dwStatus|=STATUS_REQINVITE;
		SetTimerObject(TIME_INCALL);
	}
	else {
		RemoveTimerObjects(TIME_INCALL);
		m_Status.dwStatus&=~STATUS_REQINVITE;
		PostProc(VSTRCL_CONF_CALL);
	}
}


/**
******************************************************************************
* Set or disable Bwt mode and disable transport coonnect if Bwt wizard turned on
* \param	set		- not zero to turn wizard on
* \date    08-08-2003
******************************************************************************/
void CVSTrClientProc::BwtWizardOn(int mode)
{
	m_bwt->WizardOn(mode);
	if (mode)	// force disconnect
		DisconnectProcedure();
	else
		ConnectServer(m_CurrBroker);
}

/**
******************************************************************************
* Set in Firewall
* \date    23-09-2010
******************************************************************************/
void CVSTrClientProc::SetFirewall()
{
	VS_RegistryKey key(true, "");
	long val = 0;
	key.GetValue(&val, 4, VS_REG_INTEGER_VT, "Initialized");
	if (!val) {
		VS_FirewallMgr *mgr = VS_FirewallMgr::Instance();
		if (mgr && mgr->AddFirewall(VS_WinXPFirewall::Instance())) {
			_bstr_t filename(GetCommandLine());
			int numArgs = 0;
			LPWSTR * argv = CommandLineToArgvW((wchar_t *)filename, &numArgs);
			if (argv) {
				_bstr_t appname(GUI_version);
				mgr->AddApplication(argv[0], (wchar_t *)appname, 0);
			}
		}
	}
}


/**
******************************************************************************
* Places a message in the message queue of the Notification window in GUI
* \param	msg			- message
* \param	wparam		- first message parameter
* \param	lparam		- second message parameter
* \date    08-08-2003
******************************************************************************/
BOOL CVSTrClientProc::PostStatusMessage(UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (m_ExternalWindow) return PostMessage(m_ExternalWindow, msg, wparam, lparam);
	else if (m_ExternalThread) return PostThreadMessage(m_ExternalThread, msg, wparam, lparam);
	else return FALSE;
}

/**
******************************************************************************
* Initialize Connect to server
* \param	next			- connect to next server
* \date    06-02-2008
******************************************************************************/
DWORD CVSTrClientProc::DoConnect(bool next)
{
	if (m_Status.dwStatus&STATUS_SERVAVAIL || !m_AutoConnect)
		return ERR_WARN;
	DWORD ret = VSTRCL_ERR_SERVER;
	if (next) {
		if (m_CurrBroker==REG_TempServerName) {
			ret = VSTRCL_ERR_SERVNOTAV;
		}
		else {
			int res = m_ServerList.Next(m_CurrBroker);
			if (res==-1) {
				DTRACE(VSTM_PRTCL, "DoConnect: reach end of list");
				ret = VSTRCL_ERR_SERVNOTAV;
			}
			if (!m_CurrBroker) {
				DTRACE(VSTM_PRTCL, "DoConnect: NO SERVERS! skip Connect");
				return VSTRCL_ERR_SERVNOTAV;
			}
		}
	}
	ConnectServer(m_CurrBroker);
	return ret;
}

/**
******************************************************************************
* Callback from transport, say that server have different name
* \param	old_name		- connecting server name
* \param	new_name		- connected server name
* \date    08-02-2008
* Do not call any transport func to prevent deadlock
******************************************************************************/
void CVSTrClientProc::OnSidChange(const char *old_name, const char *new_name)
{
	DTRACE(VSTM_PRTCL, "OnSidChange: %s -> %s", old_name, new_name);
	auto tcp = net::endpoint::ReadConnectTCP(1, old_name);
	if (tcp) {
		net::endpoint::ClearAllConnectTCP(new_name);
		net::endpoint::AddConnectTCP(*tcp, new_name);
		net::endpoint::Remove(old_name);
		m_ServerList.RenameServer(old_name, new_name);
	}
}

/**
******************************************************************************
* Callbat from transport
* \param	to_name			- name of endpoint
* \param	error			- cause of callback
* \date    06-02-2008
* Do not call any transport func to prevent deadlock
******************************************************************************/
void CVSTrClientProc::OnConnect(const char *to_name, const long error)
{
	DTRACE(VSTM_PRTCL, "OnConnect of %s | ERR = %d", to_name, error);
	DWORD Ret = 0;
	if (error==err_ok) {
		if (!(m_Status.dwStatus&STATUS_SERVAVAIL)) {
			// set current server
			m_CurrBroker = to_name;
			m_PrevBroker = to_name;
			m_ConnectState = CS_CONNECTED;
			// update in server list
			m_ServerList.SetEvent(m_CurrBroker, VS_ServerList::SRVT_LASTCONNET);
			// set gate
			VS_SetTransportClientDefaultGate(m_CurrBroker);
			// check ststus
			VS_WriteAS(m_CurrBroker);
			CheckUserLoginStatus();
		}
		else if (m_CurrBroker==to_name) {
			// can be reconnect, check it
			CheckUserLoginStatus();
		}
		else {
			// renew connect timer
			SetTimerObject(TIME_CONNECT);
		}
	}
	else {
		if (m_PrevBroker==to_name) {
			m_ConnectState = CS_DISCONNECTED;
			SetOnlineStatus(false);
			if (error==err_antikclient) {
				// show message
				Ret = VSTRCL_ERR_TRANSPOLD;
			}
			else {
				// renew connect timer
				switch(error)
				{
				case err_ssl_could_not_established:
				case err_cert_expired:
				case err_cert_not_yet_valid:
				case err_cert_is_invalid:
				case err_srv_name_is_invalid:
				case err_version_do_not_match:
					Ret = VSTRCL_SSL_ERR - error;
					RemoveTimerObjects(TIME_CONNECT);

					break;
				default:
					SetTimerObject(TIME_CONNECT, 5000);
				};
			}
			VS_SetTransportClientDefaultGate("");
			m_PrevBroker.Empty();
		}else if(!m_PrevBroker)
		{
			switch(error)
			{
			case err_ssl_could_not_established:
			case err_cert_expired:
			case err_cert_not_yet_valid:
			case err_cert_is_invalid:
			case err_srv_name_is_invalid:
			case err_version_do_not_match:
				Ret = VSTRCL_SSL_ERR - error;
			};
		}
	}
	PostProc(Ret);
}

/**
******************************************************************************
* Determine current state of Control message interaface
* \date    09-01-2003
******************************************************************************/
void CVSTrClientProc::PostProc(DWORD Ret, DWORD MeesId)
{
	if (Ret==VSTRCL_ERR_TIMEOUT) {
		return;
	}

	DTRACE(VSTM_PRTCL, "postrproc st=%04x, ret=%04x, lp=%x, th = %x", m_Status.dwStatus>>16, Ret, MeesId, GetCurrentThreadId());
	MonitorStatus();

	if (m_Status.dwStatus&STATUS_INCALL) RemoveTimerObjects(TIME_TSTACC); // clear test dirrect connect timeout

	if (m_Status.dwStatus&STATUS_MESSAGE) {
		m_Status.dwStatus&=~STATUS_MESSAGE;			// clear for new messages
		PostStatusMessage( WM_NOTIFY_GUI, (m_Status.dwStatus|STATUS_MESSAGE)&0x0FFF0000, MeesId);
	}
	else if (m_Status.dwStatus&STATUS_COMMAND) {
		m_Status.dwStatus&=~STATUS_COMMAND;			// clear for new messages
		PostStatusMessage( WM_NOTIFY_GUI, (m_Status.dwStatus|STATUS_COMMAND)&0x0FFF0000, MeesId);
	}
	else if ((Ret&ERR_RET) || (Ret == ERR_OK)) {
		PostStatusMessage( WM_NOTIFY_GUI, (m_Status.dwStatus & 0xFFFF0000) | Ret, MeesId);
	}

	if (m_Status.dwStatus&STATUS_CONFERENCE) {
		if (m_Status.CurrConfInfo->confState == m_Status.CurrConfInfo->CONF_STATE_COMING)
			GetConferense();	// init vars
	}

	m_Status.dwLastReturn = Ret;
	m_Status.dwStatus &= 0xFFFF0000;
	m_Status.dwStatus |= m_Status.dwLastReturn;
}

/**
* Fill specified parametrs
*
* \param	Conference		- string to copy created conference
* \param	Endpoint		- string to copy endpoint to connect
* \return	Connetion type (both connect = 0, both accept = 1, accept&connect = 2)
* \date    09-01-2003		Created
******************************************************************************/
DWORD CVSTrClientProc::GetConferense(char *Conference, char *Endpoint)
{
	VS_AutoLock lock(this);
	DWORD ret = 0;
	*Conference = 0;
	*Endpoint = 0;

	if (!!m_Status.CurrConfInfo->Conference) {
		m_Status.CurrConfInfo->confState = CUserDesk::CONF_STATE_DONE;
		strcpy(Conference, m_Status.CurrConfInfo->Conference);

		if (m_Status.CurrConfInfo->confType==0) {// private, symetric
			switch(m_Status.CurrConfInfo->ConnectType)
			{
			case DIRECT_ACCEPT:
				if (m_Status.CurrConfInfo->UserName)
					strcpy(Endpoint, m_Status.CurrConfInfo->UserName);
				ret = 1;
				break;
			case DIRECT_CONNECT:
				if (m_Status.CurrConfInfo->UserName)
					strcpy(Endpoint, m_Status.CurrConfInfo->UserName);
				ret = 5;
				break;
			case DIRECT_SELF:
				strcpy(Endpoint, m_Status.MyInfo.UserName);
				ret = 2;
				break;
			case DIRECT_UNKNOWN:
			case NO_DIRECT_CONNECT:
				strcpy(Endpoint, ReturnBroker(m_Status.CurrConfInfo->Conference));
				ret = 0;
				break;
			case DIRECT_NHP:
				if (m_Status.CurrConfInfo->UserName)
					strcpy(Endpoint, m_Status.CurrConfInfo->UserName);
				ret = 6;
				break;
			case DIRECT_UDP:
				if (m_Status.CurrConfInfo->UserName)
					strcpy(Endpoint, m_Status.CurrConfInfo->UserName);
				else
					*Endpoint = 0;
				ret = 7;
				break;
			}
		}
		else if (m_Status.CurrConfInfo->confType==2) {// public host
			strcpy(Endpoint, ReturnBroker(m_Status.CurrConfInfo->Conference));
			ret = 4;
		}
		else if (m_Status.CurrConfInfo->confType==3){//  public member
			strcpy(Endpoint, ReturnBroker(m_Status.CurrConfInfo->Conference));
			ret = 0;
		}
		else if (m_Status.CurrConfInfo->confType==4){//  Multi member
			strcpy(Endpoint, ReturnBroker(m_Status.CurrConfInfo->Conference));
			ret = 4;
		}
		else if (m_Status.CurrConfInfo->confType==5){//  Multi Intercom member
			strcpy(Conference, m_Status.CurrConfInfo->Conference);
			strcpy(Endpoint, m_CurrBroker);
			ret = 3;
		}
	}
	m_LastConfName = Conference;
	return ret;

}


/**
******************************************************************************
* Store pointed tracks
* \param	tracks		- pointer to array of tracks
* \param	nTrack		- number of tracks
* \date    11-03-2004
******************************************************************************/
void  CVSTrClientProc::SetTracks(const stream::Track* tracks, unsigned nTrack)
{
	memset(m_tracks, 0, sizeof(m_tracks));
	m_nTrack = 0;
	if (tracks && nTrack) {
		auto num = std::min<unsigned>(nTrack, sizeof(m_tracks));
		memcpy(m_tracks, tracks, num);
		m_nTrack = num;
	}
}


/**
******************************************************************************
* Create curr conf sender and reciever
* \date    11-03-2004
******************************************************************************/
void CVSTrClientProc::GetConferense()
{
	char ConfName[MAX_PATH] = {0}, EndpointName[MAX_PATH] = {0}, MyName[MAX_PATH] = {0}, OtherName[MAX_PATH] = {0};
	DWORD dMode = GetConferense(ConfName, EndpointName);
	GetMyName(MyName); _strlwr(MyName);				/// convert to lowercase to correct use whith streams
	GetOtherName(OtherName); _strlwr(OtherName);
	if(!*ConfName || !*EndpointName || !*MyName) {
		PostStatusMessage( WM_NOTIFY_CALL, 5, NULL);
		return;
	}

	boost::shared_ptr<VS_StreamClientSender> snd;
	boost::shared_ptr<VS_StreamClientReceiver> rcv;

	if (dMode==6) {/// NHP
		snd = CUserDesk::Trmtr;
		rcv = CUserDesk::Trmtr;
	}
	else if(dMode == 7) //Direct UDP
	{
		snd =  CUserDesk::DirectUdpTrmtr;
		rcv =  CUserDesk::DirectUdpTrmtr;
	}
	else {
		snd.reset(new VS_StreamClientSender);
		rcv.reset(new VS_StreamClientReceiver);
	}

	HANDLE hEvents[2];
	hEvents[0]=CreateEvent(NULL,FALSE,FALSE,NULL);
	hEvents[1]=CreateEvent(NULL,FALSE,FALSE,NULL);
	long streamConTimeout = STREAM_CONNECT_TIMEOUT;

	if (dMode==0 || dMode==4) {
		if (m_CurrBroker!=(const char*)EndpointName || !!m_defaultDomain) {
			unsigned long srv_resp = -1;
			VS_ConnectionsCheckFast(EndpointName, streamConTimeout/2, &srv_resp);
			DTRACE(VSTM_PRTCL, "Check stream connects of Server %s, resp=%d", EndpointName, srv_resp);
		}
	}

	DTRACE(VSTM_PRTCL, "mode=%d CN=%s, MyN=%s, ON=%s, Endp=%s", dMode, ConfName,MyName,OtherName,EndpointName);
	snd->SetQOSStream(m_dwMarkQos!=0);
	rcv->SetQOSStream(m_dwMarkQos!=0);


	bool multicast_ret;
	char ip[256] = { 0 };
	long lenght = sizeof(ip);
	switch(dMode)
	{
	case 5:
		/// It is very strange, but if you set fast on rcv in case=5
		/// and set fast on snd in case=1 - latency will be big.
		snd->SetFastStream();///!< FOR LOCAL NET ONLY
	case 0:
		snd->Connect(ConfName,MyName,OtherName,EndpointName,m_tracks,m_nTrack,streamConTimeout,hEvents[0],FALSE);
		rcv->Connect(ConfName,MyName,OtherName,EndpointName,m_tracks,m_nTrack,streamConTimeout,hEvents[1],FALSE);
		break;
	case 1:
		rcv->SetFastStream();///!< FOR LOCAL NET ONLY
		snd->Accept(ConfName,MyName,OtherName,EndpointName,m_tracks,m_nTrack,streamConTimeout,hEvents[0],FALSE);
		rcv->Accept(ConfName,MyName,OtherName,EndpointName,m_tracks,m_nTrack,streamConTimeout,hEvents[1],FALSE);
		break;
	case 2:
		snd->Connect(ConfName,MyName,OtherName,EndpointName,m_tracks,m_nTrack,streamConTimeout,hEvents[0],FALSE);
		rcv->Accept(ConfName,MyName,OtherName,EndpointName,m_tracks,m_nTrack,streamConTimeout,hEvents[1],FALSE);
		break;
	case 3:
		VS_GetEndpointSourceIP(EndpointName, ip, lenght);
		multicast_ret = m_Status.MyInfo.config.connTCPSucs.size() >= 2
			&& snd->Connect(m_Status.MyInfo.config.connTCPSucs[1]->port, m_Status.MyInfo.config.connTCPSucs[1]->host.c_str(), m_Status.MyInfo.ConferenseKey, 0, 0, 0, ip);
		SetEvent(hEvents[0]);
		SetEvent(hEvents[1]);
		break;
	case 6:
	case 7:/// NHP already has been connected
		SetEvent(hEvents[0]);
		SetEvent(hEvents[1]);
		break;
	case 4:
		snd->Connect(ConfName,MyName,OtherName,EndpointName,m_tracks,m_nTrack,streamConTimeout,hEvents[0],FALSE);
		SetEvent(hEvents[1]);
		break;
	}

	DWORD ret = WaitForMultipleObjects(2, hEvents, TRUE, streamConTimeout);
	CloseHandle(hEvents[0]);
	CloseHandle(hEvents[1]);
	BOOL Abort = 0;
	if (ret==WAIT_OBJECT_0 || ret==WAIT_OBJECT_0+1) {
		switch(dMode)
		{
		case 0:
		case 5:
			Abort|= snd->ConnectType()!=vs_stream_client_connect_type_connect;
			Abort|= rcv->ConnectType()!=vs_stream_client_connect_type_connect;
			break;
		case 1:
			Abort|= snd->ConnectType()!=vs_stream_client_connect_type_accept;
			Abort|= rcv->ConnectType()!=vs_stream_client_connect_type_accept;
			break;
		case 2:
			Abort|= snd->ConnectType()!=vs_stream_client_connect_type_connect;
			Abort|= rcv->ConnectType()!=vs_stream_client_connect_type_accept;
			break;
		case 3:
			Abort = !multicast_ret;
			break;
		case 6:
		case 7:
			break;
		case 4:
			Abort|= snd->ConnectType()!=vs_stream_client_connect_type_connect;
			break;
		}
	}
	else {
		Abort = 1;
		DTRACE(VSTM_PRTCL, "TimeOut in Streams Connect");
	}
	m_Status.CurrConfInfo->Snd = snd;
	m_Status.CurrConfInfo->Rcv = rcv;
	if (Abort){
		m_Status.CurrConfInfo->CloseStreams();
		DTRACE(VSTM_PRTCL, "\tConf streams Closed");
		PostStatusMessage( WM_NOTIFY_CALL, 5, NULL);
		m_FailedEndpoint = EndpointName;
		SendFailureConnect(EndpointName, ConfName, OtherName);
	}
	else{
		m_FailedEndpoint.Empty();
		DTRACE(VSTM_PRTCL, "\tConf streams continued");
		PostStatusMessage( WM_NOTIFY_CALL, 6, NULL);
		DeviceStatusChanged();
	}
}

DWORD CVSTrClientProc::SendFailureConnect(char *Endpoint, char *Conference, char* owner)
{
	if (!Endpoint || !Conference || !owner)
		return -1;

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, CONNECTFAILURE_METHOD);
	rCnt.AddValue(CONFERENCE_PARAM, Conference);
	rCnt.AddValue(OWNER_PARAM, owner);
	rCnt.AddValue(USERNAME_PARAM, m_Status.MyInfo.UserName);

	char ip[256] = {0};
	long len = sizeof(ip);
	VS_GetEndpointSourceIP(m_CurrBroker, ip, len);
	rCnt.AddValue(HOST_PARAM, ip);

	auto connect_data = net::endpoint::Serialize(true, Endpoint);
	if (!connect_data.empty())
		rCnt.AddValue(EPCONFIG_PARAM, static_cast<const void*>(connect_data.data()), connect_data.size());

	ComposeSend(rCnt, CONFERENCE_SRV, Endpoint);
	return ERR_OK;
}


/**
******************************************************************************
* \return curr conf stream sender
* \date    11-03-2004
******************************************************************************/
boost::shared_ptr<VS_StreamClientSender> CVSTrClientProc::GetConferenseSender(){
	VS_AutoLock lock(&CUserDesk::Trmtr_lock);
	boost::shared_ptr<VS_StreamClientSender> snd = m_Status.CurrConfInfo->Snd;
	m_Status.CurrConfInfo->Snd.reset();
	return snd;
}

/**
******************************************************************************
* \return curr conf stream receiver
* \date    11-03-2004
******************************************************************************/
boost::shared_ptr<VS_StreamClientReceiver> CVSTrClientProc::GetConferenseReceiver(){
	VS_AutoLock lock(&CUserDesk::Trmtr_lock);
	boost::shared_ptr<VS_StreamClientReceiver> rcv = m_Status.CurrConfInfo->Rcv;
	m_Status.CurrConfInfo->Rcv.reset();
	return rcv;
}

/**
******************************************************************************
* \return curr conf stream symmetric crypt key
* \date    31-03-2009
******************************************************************************/
bool CVSTrClientProc::GetSimKey(char *UserName, VS_BinBuff &key)
{
	VS_AutoLock lock(this);
	key.Empty();
	VS_SimpleStr ckey;
	if (!UserName || !*UserName || m_Status.MyInfo.UserName==UserName)
		ckey = m_Status.CurrConfInfo->ConfSimKey;
	else {
		if (m_Status.CurrConfInfo->confType==0)
			ckey = m_Status.CurrConfInfo->ConfSimKey;
		else {
			VSClientRcvDesc rd;
			if (m_Status.CurrConfInfo->RsvList.GetRcvDesc(UserName, rd))
				ckey = rd.m_symkey;
		}
	}
	if (!!ckey && ckey.Length() >= 32) {
		char *c = ckey;
		unsigned char buff[16];
		for (int i = 0; i<16; i++) {
			int val = 0;
			sscanf(c+2*i, "%02X", &val);
			buff[i] = val;
		}
		key.Set(buff, 16);
	}
	return key.IsValid();
}


/**
****************************************************************************
* Fill specified parametrs about logged in User
*
* \param	UserName		- string to copy User Name (Login)
* \param	FirstName		- string to copy First Name
* \param	LastName		- string to copy Last Name
* \param	Email			- string to copy Email adress
* \date    09-01-2003
******************************************************************************/
int  CVSTrClientProc::GetMyName(char *UserName, char *FirstName, char *LastName, char *Email)
{
	VS_AutoLock lock(this);

	CUserDesk* ud = &m_Status.MyInfo;
	if (UserName) {
		if (!!ud->UserName)		strcpy(UserName, ud->UserName);
		else UserName[0] = 0;
	}
	if (FirstName) {
		if (!!ud->DisplayName) {
			strncpy(FirstName, ud->DisplayName, 252);
			*(int*)(FirstName + 252) = 0;
		}
		else FirstName[0] = 0;
	}
	if (LastName)
		LastName[0] = 0;
	if (Email) {
		if  (!!ud->CallId)		strcpy(Email, ud->CallId);
		else Email[0] = 0;
	}
	return ud->Rights;
}

/**
****************************************************************************
* Fill specified parametrs about called User
*
* \param	UserName		- string to copy User Name (Login)
* \param	FirstName		- string to copy First Name
* \param	LastName		- string to copy Last Name
* \param	Email			- string to copy Email adress
* \date    09-01-2003
*******************************************************************************/
void CVSTrClientProc::GetOtherName(char *UserName, char *FirstName, char *LastName, char *Email)
{
	VS_AutoLock lock(this);

	CUserDesk* ud;
	if (m_Status.dwStatus&STATUS_INCALL)			// incall
		ud = &m_Status.ConfInfo[1];
	else if (m_Status.dwStatus&STATUS_REQINVITE)	// req invite
		ud = &m_Status.ConfInfo[2];
	else
		ud = m_Status.CurrConfInfo;

	if (UserName) {
		if (!!ud->UserName)		strcpy(UserName, ud->UserName);
		else UserName[0] = 0;
	}

	if (FirstName) {
		if (!!ud->DisplayName) {
			strncpy(FirstName, ud->DisplayName, 252);
			*(int*)(FirstName + 252) = 0;
		}
		else FirstName[0] = 0;
	}

	if (LastName)
		LastName[0] = 0;
	if (Email) {
		if  (!!ud->CallId)		strcpy(Email, ud->CallId);
		else Email[0] = 0;
	}
	m_Status.dwStatus&=~STATUS_USERINFO;  /// clear after read
}

/**
****************************************************************************
* Fill specified parametrs about Users in address book
*
* \param	List		- referense to assign array of
* \date    20-03-2003
******************************************************************************/
void CVSTrClientProc::GetUsersList(void **List, int book)
{
	DTRACE(VSTM_PRTCL, "Qery Of GetUsersList, book = %d", book);
}

/**
****************************************************************************
* 	Fill specified by Name Propeties
*
* \param	Name			- name of Propeties
* \param	Propeties		- Propeties (string)
* \date    15-05-2003
******************************************************************************/
DWORD CVSTrClientProc::GetProperties(const char* Name, char* Propeties)
{
	if (!Name || !Propeties)
		return -1;
	VS_AutoLock lock(this);
	const char *prop = m_UserProp.GetStrValueRef(Name);
	if (!prop || !*prop)
		prop = m_PropContainer.GetStrValueRef(Name);
	if (prop){
		int slen = strlen(prop);
		if (slen >= 255) {
			memcpy(Propeties, prop, 255); // GUI restriction
			Propeties[255] = 0;
			return slen;
		}
		else {
			memcpy(Propeties, prop, slen);
			Propeties[slen] = 0;
			return ERR_OK;
		}
	}
	else
		return -2;
}

DWORD CVSTrClientProc::GetProperties(const char* Name, VS_SimpleStr &Propeties)
{
	if (!Name)
		return -1;

	variant_t vname = Name;
	int res = Process(GET_PARAM, "GetProperties", &vname.GetVARIANT());
	if (res == VS_INTERFACE_OK) {
		Propeties = vs::StrFromVariantT(vname);
		return ERR_OK;
	}
	return -2;
}


/**
****************************************************************************
* 	Fill From and Mess by Id
* \date    15-05-2003
******************************************************************************/
int CVSTrClientProc::GetMessage( int Id, char* From, char* Mess, char* to /*= 0*/, char* Dn /*= 0*/, long long *time )
{
	VS_Container *cnt = m_MessContainers.GetList(Id);
	if (!cnt)
		return -1;
	const char *param = 0;
#define VS_SAFE_COPY(par, name)	if (par) {(par)[0] = 0; param = cnt->GetStrValueRef(name); if (param) strcpy(par, param); }
	VS_SAFE_COPY(From, FROM_PARAM);
	VS_SAFE_COPY(Mess, MESSAGE_PARAM);
	VS_SAFE_COPY(to, TO_PARAM);
	VS_SAFE_COPY(Dn, DISPLAYNAME_PARAM);
	int32_t type = MSG_NORMAL; cnt->GetValue(TYPE_PARAM, type);

	if (time)
	{
		size_t size;
		FILETIME *ft = (FILETIME *)cnt->GetBinValueRef(FILETIME_PARAM, size);
		if (ft && sizeof(FILETIME) == size)
		{
			LARGE_INTEGER date;
			date.HighPart = ft->dwHighDateTime;
			date.LowPart = ft->dwLowDateTime;
			date.QuadPart -= 11644473600000UL * 10000;
			date.QuadPart /= 10000000;
			*time = date.LowPart;
		} else *time = std::time(0);
	}

	return type;
}

/**
****************************************************************************
* 	Fill From and Command by Id
* \date    15-05-2003
******************************************************************************/
void CVSTrClientProc::GetCommand(int Id, char* From, char* Command)
{
	VS_Container *cnt = m_CommandContainers.GetList(Id);
	if (!cnt)
		return;
	const char *param = 0;
	VS_SAFE_COPY(From, FROM_PARAM);
	VS_SAFE_COPY(Command, MESSAGE_PARAM);
}

/**
****************************************************************************
* Connect new rcv
* \date    08-10-2003
******************************************************************************/
int CVSTrClientProc::GetRcvAction(int Id, char *name, boost::shared_ptr<VS_StreamClientReceiver> &rcv, long &fltr)
{
	int res = m_Status.CurrConfInfo->RsvList.GetAction(Id, name, rcv, fltr);
	if (res==2) {
		long dvs = 0;
		if (GetRcvDevStatus(name, &dvs))
			NotifyDvs(name, dvs);
	}
	return res;
}

/**
****************************************************************************
* Get Rcv device statuses
* \date    08-10-2003
******************************************************************************/
bool CVSTrClientProc::GetRcvDevStatus(char *name, long *dvs)
{
	VSClientRcvDesc rd;
	if (m_Status.CurrConfInfo->RsvList.GetRcvDesc(name, rd)) {
		*dvs = rd.m_dvs;
		return true;
	}
	return false;
}

/**
****************************************************************************
* 	Get Rcv MediaFormat
* \return	-2 = not found
*			-1 = invalid format
*			 0 = all ok
* \date    09-04-2004
******************************************************************************/
int CVSTrClientProc::GetMediaFormat(char *name, VS_MediaFormat &fmt, VS_ClientCaps *pCaps)
{
	VSClientRcvDesc rd;
	VS_ClientCaps *pCapsCurr = 0;
	fmt.SetAudio(0, 0);
	fmt.SetVideo(0, 0, 0);
	switch(m_Status.CurrConfInfo->confType)
	{
	case 0:
	case 3:
		if (m_Status.MyInfo.UserName == (const char *)name) {
			fmt = m_Status.CurrConfInfo->ClientCaps.GetFmtFromToMe(m_Status.MyInfo.ClientCaps);
			pCapsCurr = &(m_Status.MyInfo.ClientCaps);
			fmt.dwSVCMode = m_Status.CurrConfInfo->SVCMode;
		} else if (m_Status.CurrConfInfo->UserName == (const char *) name) {
			fmt = m_Status.MyInfo.ClientCaps.GetFmtFromToMe(m_Status.CurrConfInfo->ClientCaps);
			pCapsCurr = &(m_Status.CurrConfInfo->ClientCaps);
		} else
			return -2;
		break;
	default:
		if (m_Status.MyInfo.UserName == (const char *)name) {
			m_Status.MyInfo.ClientCaps.GetMediaFormat(fmt);
			pCapsCurr = &(m_Status.MyInfo.ClientCaps);
			fmt.dwSVCMode = m_Status.CurrConfInfo->SVCMode;
			if (m_Status.CurrConfInfo->confType == 5) fmt.dwSVCMode = 0xffffffff;
		} else if (m_Status.CurrConfInfo->RsvList.GetRcvDesc(name, rd)) {
			fmt = m_Status.MyInfo.ClientCaps.GetFmtFromToMe(rd.m_caps);
			pCapsCurr = &(rd.m_caps);
		} else
			return -2;
		break;
	}
	if (pCaps) {
		void *pBuff = 0;
		size_t size = 0;
		if (pCapsCurr->Get(pBuff, size)) {
			pCaps->Set(pBuff, size);
			free(pBuff);
		}
	}
	DTRACE(VSTM_PRTCL, "MEDIAFORMAT of %s: %3dx%3d - %.4s, %5d Hz - %x len=%d, stereo = %u b", name,
		fmt.dwVideoWidht, fmt.dwVideoHeight, &fmt.dwVideoCodecFCC, fmt.dwAudioSampleRate, fmt.dwAudioCodecTag, fmt.dwAudioBufferLen, fmt.dwStereo);
	if (fmt.IsAudioValid() || fmt.IsVideoValid())
		return m_Status.CurrConfInfo->confType;
	else
		return -1;
}

/**
****************************************************************************
* 	Set Snd format
* \date    09-04-2004
******************************************************************************/
void CVSTrClientProc::SetMediaFormat(VS_MediaFormat &fmt, int rating, int level, int level_group)
{
	VS_AutoLock lock(this);

	m_SourceFmt = fmt;
	m_Status.MyInfo.ClientCaps.SetMediaFormat(m_SourceFmt);
	m_Status.MyInfo.ClientCaps.SetAudioRcv(VSCC_AUDIO_DEFAULT|VSCC_AUDIO_ANYFREQ|VSCC_AUDIO_ANYBLEN);
	DWORD dwVideoFlag = VSCC_VIDEO_DEFAULT | VSCC_VIDEO_ANYSIZE | VSCC_VIDEO_ANYCODEC | VSCC_VIDEO_DYNCHANGE |
						VSCC_VIDEO_MULTIPLICITY8 | VSCC_VIDEO_MULTIPLICITY4 | VSCC_VIDEO_MULTIPLICITY2 | VSCC_VIDEO_H264PORTRETFIX | VSCC_VIDEO_RCVSTEREO;
	m_Status.MyInfo.ClientCaps.SetVideoRcv(dwVideoFlag);
	long StreamDC = VSCC_STREAM_ADAPTIVE_DATA_DECODE | VSCC_STREAM_CAN_DECODE_SSL | VSCC_STREAM_CAN_CHANGE_MF_RCV;
	if (m_SourceFmt.dwVideoCodecFCC == VS_VCODEC_VPX) StreamDC |= VSCC_STREAM_CAN_USE_SVC;
	m_Status.MyInfo.ClientCaps.SetStreamsDC(StreamDC);
	m_Status.MyInfo.ClientCaps.SetRating(rating);
	m_Status.MyInfo.ClientCaps.SetLevel(level);
	m_Status.MyInfo.ClientCaps.SetLevelGroup(level_group);

	uint16_t codecs[100];
	size_t codecsNum = 0;
	int Id = 0;
	for (Id = 0; Id < 0xffff && codecsNum <100; Id++) {
		AudioCodec *cdc = VS_RetriveAudioCodec(Id, true);
		if (cdc) {
			codecs[codecsNum++] = Id;
			delete cdc;
		}
	}
	m_Status.MyInfo.ClientCaps.SetAudioCodecs(codecs, codecsNum);

	DWORD vcodecsNumAll = 9;
	uint32_t vcodecs[100];
	size_t vcodecsNum = 0;
	unsigned int vId = 0;
	for (vId = 0; vId < vcodecsNumAll; vId++) {
		VideoCodec *cdc = VS_RetriveVideoCodec(VS_EnumVCodecs[vId], true);
		if (cdc) {
			vcodecs[vcodecsNum++] = VS_EnumVCodecs[vId];
			delete cdc;
		} else if (VS_EnumVCodecs[vId] == VS_VCODEC_H264) {
			if (GetTypeHardwareCodec() == ENCODER_H264_INTEL ||
				GetTypeHardwareCodec() == ENCODER_H264_INTEL_MSS ||
				GetTypeHardwareCodec() == ENCODER_H264_NVIDIA) {
				vcodecs[vcodecsNum++] = VS_EnumVCodecs[vId];
			}
		}
	}
	m_Status.MyInfo.ClientCaps.SetVideoCodecs(vcodecs, vcodecsNum);
}

/**
****************************************************************************
* 	Prepare Format for private or group conf
* \date    07-05-2011
******************************************************************************/
void CVSTrClientProc::PrepareCaps(bool groupconf)
{
	VS_AutoLock lock(this);

	if (!m_bTranscoder) {
		ScreenCapturer *capturer = ScreenCapturerFactory::Create(CapturerType::GDI);
		size_t w = 0, h = 0;
		if (capturer && capturer->GetMaxScreenParams(w, h)) {
			SetScreenResolution(w, h);
		}
		delete capturer;
	}

	if (groupconf) {
		VS_MediaFormat fmt;
		m_Status.MyInfo.ClientCaps.GetMediaFormat(fmt);
		unsigned int w = fmt.dwVideoWidht, h = fmt.dwVideoHeight, fps = fmt.dwFps, fourcc = fmt.dwVideoCodecFCC, stereo = fmt.dwStereo;
		if (m_HQAuto != 0) {

#ifndef _BUILD_CONFERENDO
			if (m_enchSLayers != 0) {
				w = 1280; h = 720; fps = 15; fourcc = VS_VCODEC_VPX;
			} else {
				w = 640; h = 360; fps = 15; fourcc = VS_VCODEC_VPX;
			}
#else
			w = 640; h = 480; fps = 15; fourcc = VS_VCODEC_VPX;
#endif

			if (m_SourceFmt.dwVideoWidht < w && m_SourceFmt.dwVideoHeight < h) {
				w = m_SourceFmt.dwVideoWidht;
				h = m_SourceFmt.dwVideoHeight;
				fps = std::min<unsigned int>(m_SourceFmt.dwFps, 15);
			}
		}
		fmt.SetVideo(w, h, fourcc, fps, stereo);
		fmt.SetAudio(fmt.dwAudioSampleRate, VS_ACODEC_OPUS_B0914);
		m_Status.MyInfo.ClientCaps.SetMediaFormat(fmt);
		m_Status.MyInfo.ClientCaps.SetAudioSnd(VSCC_AUDIO_DEFAULT);
		m_Status.MyInfo.ClientCaps.SetVideoSnd(VSCC_VIDEO_DEFAULT);
		m_Status.MyInfo.ClientCaps.ClearMediaFormatRcv();
	}
	else {
		if (m_HQAuto != 0) {

#ifndef _BUILD_CONFERENDO
			if (m_SourceFmt.dwVideoWidht >= 640 && m_SourceFmt.dwVideoHeight >= 360) {
				m_SourceFmt.dwVideoWidht = 640;
				m_SourceFmt.dwVideoHeight = 360;
			}
#else
			if (m_SourceFmt.dwVideoWidht >= 640 && m_SourceFmt.dwVideoHeight >= 480) {
				m_SourceFmt.dwVideoWidht = 640;
				m_SourceFmt.dwVideoHeight = 480;
			}
#endif

			m_SourceFmt.SetVideo(m_SourceFmt.dwVideoWidht, m_SourceFmt.dwVideoHeight, VS_VCODEC_VPX);
		}
		m_Status.MyInfo.ClientCaps.SetMediaFormat(m_SourceFmt);
		m_Status.MyInfo.ClientCaps.SetAudioSnd(VSCC_AUDIO_DEFAULT|VSCC_AUDIO_ANYFREQ|VSCC_AUDIO_ANYBLEN);
		m_Status.MyInfo.ClientCaps.SetVideoSnd(VSCC_VIDEO_DEFAULT|VSCC_VIDEO_ANYCODEC|VSCC_VIDEO_ANYSIZE);
	}
}

/**
****************************************************************************
* 	Set auto media mode
* \date    11-01-2013
******************************************************************************/
void CVSTrClientProc::SetAutoMode(int autoMode)
{
	VS_AutoLock lock(this);
	m_HQAuto = autoMode;
}

/**
****************************************************************************
* 	Set System Load
* \date    11-01-2013
******************************************************************************/
void CVSTrClientProc::SetSystemLoad(int load)
{
	VS_AutoLock lock(this);
	m_loadSystem = load;
}

/**
****************************************************************************
* 	Set frame MBps by user
* \date    11-01-2013
******************************************************************************/
void CVSTrClientProc::SetFrameSizeMBUser(char *name, int frameSizeMB)
{
	if (m_Status.dwStatus&STATUS_CONFERENCE) {
		bool ret = m_Status.CurrConfInfo->RsvList.SetFrameSizeMBUser(name, frameSizeMB);
		if (ret) {
			SetTimerObject(TIME_SND_MBPSLIST, 1000);
		}
	}
}

void CVSTrClientProc::SetScreenResolution(long width, long height)
{
	m_Status.MyInfo.ClientCaps.SetScreenWidth(width);
	m_Status.MyInfo.ClientCaps.SetScreenHeight(height);
}

HANDLE CVSTrClientProc::GetRestrictHandle()
{
	return m_hRestrictMedia;
}

/**
***************************************************************************
* Check track for sending
* \date    07-11-2005
* \return true for allowed track
*****************************************************************************/
bool	CVSTrClientProc::IsTrackSent(int track) {
	if		(track==1)
		return (m_Status.CurrConfInfo->sfltr&m_Status.CurrConfInfo->lfltr&VS_RcvFunc::FLTR_RCV_AUDIO)!=0;
	else if (track==2)
		return (m_Status.CurrConfInfo->sfltr&m_Status.CurrConfInfo->lfltr&VS_RcvFunc::FLTR_RCV_VIDEO)!=0;
	else if (track==5)
		return (m_Status.CurrConfInfo->sfltr&m_Status.CurrConfInfo->lfltr&VS_RcvFunc::FLTR_RCV_DATA)!=0;
	else
		return true;
}


/**
***************************************************************************
* Return AutoLogin
* \date    29-07-2003
*****************************************************************************/
DWORD CVSTrClientProc::GetAutoLogin()
{
	return m_Status.dwStatus&STATUS_LOGGEDIN ? m_AutoLogin : -1;
}


/**
****************************************************************************
* \date    01-02-2007
* Store specified DNS for ServerList discovery()
* \date    22.02.2008
* Check current server paramet
*
* \param	service		- wanted service where info will be found
******************************************************************************/
DWORD CVSTrClientProc::SetDiscoveryServise(const char* service)
{
	VS_SimpleStr ls = service;
	if (!ls) {
		VS_RegistryKey key(true, REG_CurrentConfiguratuon);
		char rs[256] = {0};
		key.GetValue(rs, 256, VS_REG_STRING_VT, "domain");
		if (*rs)
			ls = rs;
	}
	VS_InstallConnectionManager(m_AppId);
	m_ServerList.Reload(ls);
	m_defaultDomain = ls;
	return 0;
}

/**
****************************************************************************
* \date    22.02.2008
* Copy all servers + current server
*
* \param	size		- maximum number to copy, must be >2
* \param	pName		- char[size][MAX_PATH]
* \return number of copied servers
******************************************************************************/
int CVSTrClientProc::GetServers(int size, unsigned char*pName)
{
	if (size<2)
		return 0;

	char server[256] = {0};
	VS_ReadAS(server);
	bool found = false;

	VS_SimpleStr *servers = 0;
	int n = m_ServerList.GetValidServers(servers);
	int i = 0;
	if (n > 0) {
		for (; i<n && i<size-2; i++) {
			strcpy((char*)pName + i*MAX_PATH, servers[i]);
			if (servers[i]==server)
				found = true;
		}
		delete[] servers;
	}
	else {
		*pName = 0;
		i++;
	}
	//if (!found) {
	//	strcpy((char*)pName + i*MAX_PATH, server); i++;
	//}
	strcpy((char*)pName + i*MAX_PATH, server);
	return i+1;
}


/**
****************************************************************************
* Switch Server
*
* \param	szServer		- server to switch
* \return 0 if OK
* \date    06-02-2003
******************************************************************************/
DWORD CVSTrClientProc::ConnectServer(const char *szServer)
{
	DTRACE(VSTM_PRTCL, "ConnectServer %s", szServer);
	DisconnectProcedure(!!m_PrevBroker && m_PrevBroker!=szServer);
	if (szServer && *szServer)
		m_CurrBroker = szServer;
	else
		m_ServerList.Discovery(m_CurrBroker);
	DTRACE(VSTM_PRTCL, "CreateConnect to %s", m_CurrBroker.m_str);
	// to save after restart
	VS_WriteAS(m_CurrBroker);
	VS_CreateConnect(m_CurrBroker, 20000);
	// renew connect timer
	SetTimerObject(TIME_CONNECT);
	m_AutoConnect = true;
	m_ConnectState = CS_CONNECTING;
	return 0;
}



/**
****************************************************************************
* send some methods needed for server before transport disconnect
* \return 0 if OK
* \date    06-02-2003
******************************************************************************/
DWORD CVSTrClientProc::DisconnectProcedure(bool DoLogout)
{
	if (m_Status.dwStatus&STATUS_SERVAVAIL) {
		if (m_Status.dwStatus&STATUS_CONFERENCE)
			Hangup();
		if (DoLogout) {
			ResetEvent(m_LogoutEvent);
			LogoutUser(false);
			if (GetCurrentThreadId() != m_MyThreadID) {
				DWORD stime = timeGetTime();
				WaitForSingleObject(m_LogoutEvent, 1000);
				stime = timeGetTime()-stime;
				DTRACE(VSTM_PRTCL, "Other thread disc, wait %d ms", stime);
			}
			else {
				DTRACE(VSTM_PRTCL, "Our thread, sleep 400 ms");
				Sleep(400);
				Servises();
			}

		}
	}
	VS_SimpleStr &dserver = !!m_PrevBroker ? m_PrevBroker : m_CurrBroker;
	if (!!dserver) {
		DTRACE(VSTM_PRTCL, "Disconnect %s", dserver.m_str);
		VS_DisconnectEndpoint(dserver);
	}
	m_ConnectState = CS_DISCONNECTING;
	m_AutoConnect = false;
	return ERR_OK;
}


/**
****************************************************************************
* Initiate coonection to current server and current broker
*
* \return 0 if OK
* \date    06-02-2003
******************************************************************************/
DWORD CVSTrClientProc::InstallTransportSystem()
{
	if (!VS_InstallConnectionManager(m_AppId))									return -2;
	if (!VS_StreamClient::AdhereToConnectionManager())							return -3;
	if (!VS_InstallTransportClient(this) )										return -4;
	if (!VS_RegisterService(AUTH_SRV,			m_MyThreadID, WM_USERLOGIN))		return -5;
	if (!VS_RegisterService(CONFIGURATION_SRV,	m_MyThreadID, WM_NETWORKCONF))	return -5;
	if (!VS_RegisterService(CONFERENCE_SRV,		m_MyThreadID, WM_CONFERENCE))	return -5;
	if (!VS_RegisterService(CHAT_SRV,			m_MyThreadID, WM_CHAT))			return -5;
	if (!VS_RegisterService(CLIENT_SRV,			m_MyThreadID, WM_CLIENT))		return -5;
	if (!VS_RegisterService(PRESENCE_SRV,		m_MyThreadID, WM_USERPESENCE))	return -5;
	if (!VS_RegisterService(ADDRESSBOOK_SRV,	m_MyThreadID, WM_USERPESENCE))	return -5;
	if (!VS_RegisterService(TORRENT_SRV,		m_MyThreadID, WM_TORRENT))		return -5;
	if ( !VS_SetAuthenticationInterface(&m_Proxy))								return -7;

	//m_Status.MyInfo.SetNHPStream(new VS_StreamClientRtp);
	///m_Status.MyInfo.SetUDPStream(new VS_StreamClientRtp);

	return 0;
}

/**
****************************************************************************
* Initializate Client Control Protocol.
*
* \return ERR_OK if everything is OK
* \return -1 thread creation failed
* \return -2 Access Connection System Init failed
* \param wnd				- handler to window to notify for update GUI;
* \date    18-11-2002
******************************************************************************/
DWORD CVSTrClientProc::Init(HWND wnd)
{
	m_ExternalWindow = wnd;

	if (!ActivateThread((LPVOID)this, &m_MyThreadID))
		return -1;
	if (!VS_AcsLibInitial())
		return -2;
	m_bwt = new VSBwtClient(m_MyThreadID);
	m_Proxy.SetTh(m_MyThreadID);

	ReadProps();

	int cfg = -1;
	m_Proxy.SetNetMode(&cfg);	// read network settings
	m_Proxy.SetNetMode(&cfg);	// set network settings

	return ERR_OK;
}

/**
****************************************************************************
* Initializate Client Control Protocol.
*
* \return ERR_OK if everything is OK
* \return -1 thread creation failed
* \return -2 Access Connection System Init failed
* \param threadId				- thread to notify;
* \date    17-08-2006
******************************************************************************/
DWORD CVSTrClientProc::Init(DWORD threadId)
{
	m_ExternalThread = threadId;

	if (!ActivateThread(this, &m_MyThreadID))
		return -1;
	if (!VS_AcsLibInitial())
		return -2;

	m_bwt = new VSBwtClient(m_MyThreadID);
	m_Proxy.SetTh(m_MyThreadID);

	int cfg = -1;
	m_Proxy.SetNetMode(&cfg);	// read network settings
	m_Proxy.SetNetMode(&cfg);	// set network settings

	return ERR_OK;
}

/**
****************************************************************************
* Process incoming transport messages
*
* \date    06-02-2003
******************************************************************************/
void CVSTrClientProc::Servises()
{
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))	{
		if (msg.message >=WM_APP+2 && msg.message<=WM_MAX_SERIVCE_MESSAGE) {
			VS_ClientMessage tMsg(&msg);

#ifdef VZOCHAT7
			unsigned long forkBodySize;	void *forkBody; int keyFork(-1);
			VS_Container cnt;
			forkBodySize = tMsg.Body(&forkBody);
			if (cnt.Deserialize(forkBody, forkBodySize)) {
				keyFork = m_ForkContainers.AddList(&cnt);
				printf("--- * post proc fork %d %d\n", msg.message, keyFork);
			}
#endif
			switch (msg.message)
			{
			case WM_USERLOGIN:		LoginUserSrv(tMsg);			break;
			case WM_CONFERENCE:		ConferenceSrv(tMsg);		break;
			case WM_USERPESENCE:	UserPresenceSrv(tMsg);		break;
			case WM_NETWORKCONF:	UpdateConfigurationSrv(tMsg);break;
			case WM_CHAT:			ChatSrv(tMsg);				break;
			case WM_CLIENT:			ClientSrv(tMsg);			break;
			case WM_TORRENT:		TorrentSrv(tMsg);			break;
			}
#ifdef VZOCHAT7
			if (-1 != keyFork) {
				PostStatusMessage(WM_NOTIFY_GUI_CPY, msg.message, keyFork);
			}
#endif
		}
		else {
			switch (msg.message)
			{
			case WM_USER+22:
				SetEpProperties("Network Test", m_bwt->m_Out);
				break;
			case WM_APP+100:
				DTRACE(VSTM_PRTCL, "Forced Disconnect, reason: #%d", msg.lParam);
				if		(msg.lParam==1) {
					// connect by manual host-port
					ConnectServer(REG_TempServerName);
				}
				else if (msg.lParam==2) {
					if (m_AutoConnect) {
						//force reconnect with current server after protocol changing
						ConnectServer(m_CurrBroker);
					}
				}
				else {
					// server is going shutdown
					DisconnectProcedure();
					// select next server after 2..18 sec
					m_AutoConnect = true;
					SetTimerObject(TIME_CONNECT, 2000 + (VS_GenKeyByMD5() >> 18) );
				}
				break;
			case WM_APP+98:
				DTRACE(VSTM_PRTCL, "STUN: %x, %x", msg.wParam, msg.lParam);
				{
					const unsigned int sz = 256;
					char StunText[sz];
					VS_STUNClient::STUNMsgTranslate(msg.wParam, msg.lParam, StunText, sz);
					SetEpProperties("STUN Test", StunText);
				}
				break;
			case WM_APP+99:
				UpdateConfiguration(true);
				break;
			default:
				break; /// all other Widows messages
			}
		}
	}
}

/**
****************************************************************************
* Main Message fetching Loop.
*
* \return NOERROR if everything is OK	-1 if fatal error
* \date    18-11-2002
******************************************************************************/
DWORD CVSTrClientProc::Loop(LPVOID hEvDie)
{
	MSG msg;
	/// force to create message queue
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	HANDLE handles[2] = {hEvDie, g_DevStatus.GetEvent()};

	DWORD dwObj = InstallTransportSystem();
	while(TRUE)
	{
		Servises(); // check for unread messages
		TestTimerObjects();
		if (IsTimerObjectElapsed(TIME_CHECKSRV))
			SetTimerObject(TIME_CHECKSRV);
		if (IsTimerObjectElapsed(TIME_SND_PINGCONF))
			SetTimerObject(TIME_SND_PINGCONF);
		if (IsTimerObjectElapsed(TIME_CONNECT))
			SetTimerObject(TIME_CONNECT);
		dwObj = MsgWaitForMultipleObjects(2, handles, FALSE, 100, QS_ALLINPUT);
		switch(dwObj)
		{
		case WAIT_OBJECT_0: // die
			DisconnectProcedure();
			VS_UninstallTransportClient();
			m_Status.MyInfo.SetNHPStream(boost::shared_ptr<VS_StreamClientTransmitter>());
			VS_UninstallConnectionManager();
			{
				// write the best server
				VS_SimpleStr server(m_CurrBroker);
				if (m_ServerList.GetTheBest(server)) {
					VS_WriteAS(server);
					DTRACE(VSTM_PRTCL, "Found the Best Server: %s", server.m_str);
				}
			}
			return NOERROR;
		case WAIT_OBJECT_0 + 1:	// devise status
			DeviceStatusChanged();
			break;
		case WAIT_OBJECT_0 + 2: // message available
			Servises();
			break;
		case WAIT_TIMEOUT:
			break;
		default:
			Sleep(50);
			break;
		}
	}
	return -1;
}




/**
****************************************************************************
* Send Login User Message.
*
* \return ERR_OK
* \param UserName			- string containig User Name(Login);
* \param Password			- string containig User Login Password;
* \param AutoLogin			- Auto Login;
* \date    18-11-2002
*****************************************************************************/
DWORD CVSTrClientProc::LoginUser(char *UserLogin, char* Password, int AutoLogin, bool noEncript, const VS_ClientType cl_type)
{
	if (!UserLogin || !Password || !*UserLogin || !*Password)
		return VSTRCL_ERR_CURROP; // nothing to send
	DTRACE(VSTM_PRTCL, "LoginUser: %s", UserLogin);

	m_LastLogin = UserLogin;
	m_AutoLogin = AutoLogin;

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, LOGINUSER_METHOD);
	rCnt.AddValue(LOGIN_PARAM, UserLogin);
	rCnt.AddValue(USER_DEFAULT_DOMAIN,m_defaultDomain);

	char md5_pass[256]; *md5_pass = 0;
	if (noEncript ||(GetProperties("authentication_method", md5_pass)==ERR_OK && strcmp("1", md5_pass)==0) ) {
		rCnt.AddValue(PASSWORD_PARAM, Password);
	}
	else {
		VS_ConvertToMD5(Password, md5_pass);
		rCnt.AddValue(PASSWORD_PARAM, md5_pass);
	}
	rCnt.AddValue(HASH_PARAM, m_AppId);
	rCnt.AddValueI32(CLIENTTYPE_PARAM, cl_type);
	rCnt.AddValue(APPNAME_PARAM, GUI_version);
	rCnt.AddValue(FIELD1_PARAM, m_DiskOS);

	rCnt.CopyTo(m_LoginRetryCnt);

	ComposeSend(rCnt, AUTH_SRV);
	SetTimerObject(TIME_LGIN);
	return ERR_OK;
}

/**
****************************************************************************
* Send Logout User Message.
*
* \return ERR_OK
* \date    18-11-2002
******************************************************************************/
DWORD CVSTrClientProc::LogoutUser(bool clearAutoLogin)
{
	DTRACE(VSTM_PRTCL, "LogoutUser");
	if (clearAutoLogin) {
		// clear in registry
		SetAutoLogin(false);
		// clear current key
		m_AutoLoginKey.Empty();
	}

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, LOGOUTUSER_METHOD);
	m_LoginRetryCnt.Clear();

	ComposeSend(rCnt, AUTH_SRV);
	SetTimerObject(TIME_LGOUT);
	return ERR_OK;
}

/**
****************************************************************************
* Send Check User Login Status Message.
*
* \return ERR_OK
* \date    18-11-2002
******************************************************************************/
DWORD CVSTrClientProc::CheckUserLoginStatus()
{
	DTRACE(VSTM_PRTCL, "CheckUserLoginStatus on AS %s", m_CurrBroker.m_str);

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, CHECKUSERLOGINSTATUS_METHOD);

	ComposeSend(rCnt, AUTH_SRV);
	SetTimerObject(TIME_CHUSLS);
	return ERR_OK;
}

/**
****************************************************************************
* Auto Login
*
* \return ERR_OK
* \date    18-11-2002
******************************************************************************/
DWORD CVSTrClientProc::AutoLogin()
{
	if (!m_AutoLoginKey || !m_LastLogin)
		return ERR_OK;
	DTRACE(VSTM_PRTCL, "AutoLogin of %s", m_LastLogin.m_str);

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, LOGINUSER_METHOD);
	rCnt.AddValue(USER_DEFAULT_DOMAIN,m_defaultDomain);
	rCnt.AddValue(LOGIN_PARAM, m_LastLogin);
	rCnt.AddValue(KEY_PARAM, m_AutoLoginKey);
	rCnt.AddValue(HASH_PARAM, m_AppId);
	rCnt.AddValueI32(CLIENTTYPE_PARAM, m_client_type);
	rCnt.AddValue(APPNAME_PARAM, GUI_version);
	rCnt.AddValue(FIELD1_PARAM, m_DiskOS);

	rCnt.CopyTo(m_LoginRetryCnt);

	ComposeSend(rCnt, AUTH_SRV);
	SetTimerObject(TIME_LGIN);
	return ERR_OK;
}

/**
***************************************************************************
* Send AutoLogin Param
*
* \return ERR_OK
* \date    29-07-2003
*****************************************************************************/
DWORD CVSTrClientProc::SetAutoLogin(int autologin)
{
	m_AutoLogin = autologin;
	if (m_AutoLogin) {
		if (!!m_AutoLoginKey)
			m_pKey->SetString(m_AutoLoginKey, REG_Key);
	}
	else {
		m_pKey->SetString("", REG_Key);
	}
	return ERR_OK;
}

/**
****************************************************************************
* Security token renew function
*
* \return  true if success
* \date    02-08-2005
******************************************************************************/
bool CVSTrClientProc::RenewToken()
{
	if(m_sec_fail)
		return false;

	if(m_sec==0) {
		PSecurityFunctionTable (*pInit)( void );

		HMODULE sec_lib = LoadLibrary( "security.dll" );
		if(sec_lib!=NULL) {
			pInit = (PSecurityFunctionTable (*)( void )) GetProcAddress( sec_lib, "InitSecurityInterfaceA" );
			if ( pInit != NULL ) {
				m_sec = pInit();
			}
			else
				DTRACE(VSTM_PRTCL, "security.dll load failed" );
		}
		else
			DTRACE(VSTM_PRTCL, "security.dll load failed" );
	}

	if(m_sec==0) {
		m_sec_fail=true;
		return false;
	}

	if(m_sec_pack==0) {
		SECURITY_STATUS  code = (m_sec->QuerySecurityPackageInfo)( "NTLM", &m_sec_pack );
		if ( code != SEC_E_OK ) {
			DTRACE(VSTM_PRTCL, "NTLM package not found, code=%08x", code );
			return false;
		}
	}

	unsigned __int64 time=0, exp_time=*(unsigned __int64*)&m_sec_token_expiry;
	GetSystemTimeAsFileTime((FILETIME*)&time);
	if( exp_time <= time ) {
		if(exp_time!=0)
			(m_sec->FreeCredentialsHandle)(&m_sec_token);

		SECURITY_STATUS code = (m_sec->AcquireCredentialsHandle)( NULL, "NTLM", SECPKG_CRED_OUTBOUND,
			NULL, NULL, NULL, NULL, &m_sec_token, &m_sec_token_expiry );
		if ( code != SEC_E_OK ) {
			DTRACE(VSTM_PRTCL, "NTLM acq failed,code=%08x", code );
			return false;
		}
	}
	return true;
}

void  CVSTrClientProc::Authorize(VS_Container& in_cnt)
{
	int32_t type = -1;
	in_cnt.GetValue(TYPE_PARAM,type);
	DTRACE(VSTM_PRTCL, "AUTH: request type %d ", type);
	switch(type)
	{
	case LA_NTLM:
		{
			if(!RenewToken())
				return;
			SecBufferDesc out_desc, in_desc;
			SecBuffer out_buf, in_buf;

			in_desc.ulVersion = SECBUFFER_VERSION;
			in_desc.cBuffers = 1;
			in_desc.pBuffers = &in_buf;
			in_buf.BufferType = SECBUFFER_TOKEN;
			size_t sz;
			in_buf.pvBuffer=(void*)in_cnt.GetBinValueRef(DATA_PARAM, sz);
			in_buf.cbBuffer = sz;
			bool first_step=in_buf.pvBuffer==0;

			out_desc.ulVersion = SECBUFFER_VERSION;
			out_desc.cBuffers = 1;
			out_desc.pBuffers = &out_buf;
			out_buf.BufferType = SECBUFFER_TOKEN; // preping a token here
			out_buf.cbBuffer = m_sec_pack->cbMaxToken;
			out_buf.pvBuffer = new char[out_buf.cbBuffer];

			unsigned long ctx_request = ISC_REQ_REPLAY_DETECT | ISC_REQ_SEQUENCE_DETECT |
				ISC_REQ_CONFIDENTIALITY ;
			TimeStamp ts;
			unsigned long attr;

			SECURITY_STATUS code=
				(m_sec->InitializeSecurityContext)( &m_sec_token, first_step?0:&m_sec_ctxt,
				0, ctx_request, 0, SECURITY_NATIVE_DREP,
				first_step?0:&in_desc, 0,
				&m_sec_ctxt, &out_desc, &attr, &ts);

			if( code== SEC_I_COMPLETE_AND_CONTINUE || code == SEC_I_COMPLETE_NEEDED ) {
				DTRACE(VSTM_PRTCL, " CTOKEN ");
				if(m_sec->CompleteAuthToken!=NULL)
					(m_sec->CompleteAuthToken)( &m_sec_ctxt, &out_desc );
				if ( code == SEC_I_COMPLETE_NEEDED )
					code = SEC_E_OK;
				else
					code = SEC_I_CONTINUE_NEEDED;
			}

			if(code==SEC_I_CONTINUE_NEEDED || code==SEC_E_OK) {
				//continue
				DTRACE(VSTM_PRTCL, code==SEC_I_CONTINUE_NEEDED?" CONTINUE":" FINISH");
				if(out_buf.cbBuffer==0) {
					DTRACE(VSTM_PRTCL, "Auth buffer is zero?");
					break;
				}

				VS_Container out_cnt;
				out_cnt.AddValue(METHOD_PARAM,AUTHORIZE_METHOD);
				out_cnt.AddValueI32(TYPE_PARAM, LA_NTLM);
				out_cnt.AddValueI32(CLIENTTYPE_PARAM, m_client_type);
				out_cnt.AddValue(DATA_PARAM,out_buf.pvBuffer,out_buf.cbBuffer);
				ComposeSend(out_cnt, AUTH_SRV);
			}
			else {
				DTRACE(VSTM_PRTCL, " ERROR %08x",code);
			}

			if (code!=SEC_I_CONTINUE_NEEDED) {
				(m_sec->DeleteSecurityContext)(&m_sec_ctxt);
			}

			delete[] out_buf.pvBuffer;
			break;
		}
	default:
		DTRACE(VSTM_PRTCL, "AUTH: unsupported auth type %d",type);
		return;
	}
}
/**
****************************************************************************
* User Login service Messages interpreter.
*
* \param msg				- pointer to Window Message;
* \date    18-11-2002
******************************************************************************/
void CVSTrClientProc::LoginUserSrv(VS_ClientMessage &tMsg)
{
	DWORD dwRet = ERR_OK;
	DWORD dwMessId = 0;
	const char * Method = 0;

	VS_Container cnt;
	cnt.Deserialize(tMsg.Body(), tMsg.BodySize());
	Method = cnt.GetStrValueRef(METHOD_PARAM);

	auto ChatRight = [this](const char* srcs){
		if (srcs && strstr(srcs, "#vcs") != 0 && m_SrvProtocolVersion < 440)
			m_Status.MyInfo.Rights |= 0x01000000; //UR_COMM_CHAT, bug#44334
	};

	switch(tMsg.Type())
	{
	case transport::MessageType::Invalid:
		dwRet = VSTRCL_ERR_INTERNAL;
		break;
	case transport::MessageType::Request:
	case transport::MessageType::Reply:
		DTRACE(VSTM_PRTCL, "method    = %20s", Method);
		if (Method && _stricmp(Method, USERLOGGEDIN_METHOD) == 0) {
			int32_t result;
			if (cnt.GetValue(RESULT_PARAM, result))	{
				SetOnlineStatus(true);
				switch((VS_UserLoggedin_Result )result)
				{
				case USER_LOGGEDIN_OK:
					m_CleanOnSubscribe = true;
					m_LoginRetryCnt.Clear();
					m_Status.dwStatus |= STATUS_LOGGEDIN;
					m_Status.MyInfo.Clean();
					for (int i = 0; i<m_Status.MAX_CONFINFO; i++) // reset all
						m_Status.ConfInfo[i].Clean();
					m_Status.MyInfo.UserName =		cnt.GetStrValueRef(USERNAME_PARAM);
					m_Status.MyInfo.DisplayName =	cnt.GetStrValueRef(DISPLAYNAME_PARAM);
					m_Status.MyInfo.CallId =		cnt.GetStrValueRef(CALLID_PARAM);
					m_Status.MyInfo.TarifPlan =		cnt.GetStrValueRef(TARIFNAME_PARAM);
					m_Status.MyInfo.SessionKey =	cnt.GetStrValueRef(SESSION_PARAM);
					if (!!m_Status.MyInfo.SessionKey)
						SetTimerObject(TIME_SESS_RENEW);
					cnt.GetValueI32(TARIFRESTR_PARAM, m_Status.MyInfo.TarifRestrictions);
					cnt.GetValueI32(SEPARATIONGROUP_PARAM, m_Status.MyInfo.SepartionGroup);
					cnt.GetValueI32(RIGHTS_PARAM, m_Status.MyInfo.Rights);
					ChatRight(tMsg.SrcServer());
					m_AutoLoginKey =				cnt.GetStrValueRef(KEY_PARAM);
					cnt.GetValue(PROPERTY_PARAM, m_UserProp);

					VS_SetEndpointName(m_Status.MyInfo.UserName);
					VS_RemoveAcceptPorts();
					VS_StreamClient::InitUdpListener();

					if (m_DirectPort) {
						VS_UPnPInterface* unpn = VS_UPnPInterface::Instance();
						unsigned long *ports = 0;
						unsigned long num = 0;
						VS_GetAppPorts(ports, num);
						ports[0] = MAKELONG(m_DirectPort, m_DirectPort);
						char ip[256] = {0};
						long lenght = sizeof(ip);
						VS_GetEndpointSourceIP(tMsg.SrcServer(), ip, lenght);
						unpn->FullAsyncPortMapping(m_MyThreadID, ports, num, ip);
					}
					else
						UpdateConfiguration(false);

					ReadPropNetConfig();
					SetAutoLogin(m_Status.dwStatus&STATUS_LOGGEDIN ? m_AutoLogin : 0);
					dwRet = VSTRCL_LOGIN_OK;
					break;
				case USER_ALREADY_LOGGEDIN:
					m_LoginRetryCnt.Clear();
					UpdateStatus(m_StoredStatus);
					if (!(m_Status.dwStatus&STATUS_LOGGEDIN))
						AutoLogin();
					dwRet = VSTRCL_LOGIN_OK;
					{	// check ip-address change
						VS_RemoveAcceptPorts();
						if (m_DirectPort) {
							VS_UPnPInterface* unpn = VS_UPnPInterface::Instance();
							unsigned long *ports = 0;
							unsigned long num = 0;
							VS_GetAppPorts(ports, num);
							ports[0] = MAKELONG(m_DirectPort, m_DirectPort);
							char ip[256] = {0};
							long lenght = sizeof(ip);
							VS_GetEndpointSourceIP(tMsg.SrcServer(), ip, lenght);
							unpn->FullAsyncPortMapping(m_MyThreadID, ports, num, ip);
						}
						else
							UpdateConfiguration(false);
					}
					break;
				case NO_USER_LOGGEDIN:
					m_Status.dwStatus&=~STATUS_LOGGEDIN;
					AutoLogin();
					dwRet = VSTRCL_LOGIN_OK;
					break;
				case ACCESS_DENIED:
					m_LoginRetryCnt.Clear();
					m_Status.dwStatus&=~STATUS_LOGGEDIN;
					dwRet = VSTRCL_LOGIN_INCPAR;
					break;
				case SILENT_REJECT_LOGIN:
					m_LoginRetryCnt.Clear();
					m_Status.dwStatus&=~STATUS_LOGGEDIN;
					dwRet = VSTRCL_LOGIN_OK;
					break;
				case LICENSE_USER_LIMIT:
					m_LoginRetryCnt.Clear();
					m_Status.dwStatus&=~STATUS_LOGGEDIN;
					dwRet = VSTRCL_LOGIN_USERLIMIT;
					break;
				case USER_PASSWORD_EXPIRED:
					m_LoginRetryCnt.Clear();
					m_Status.dwStatus&=~STATUS_LOGGEDIN;
					dwRet = VSTRCL_LOGIN_PWDEXPIRED;
					break;
				case USER_DISABLED:
					m_LoginRetryCnt.Clear();
					m_Status.dwStatus&=~STATUS_LOGGEDIN;
					dwRet = VSTRCL_LOGIN_USDISBLD;
					break;
				case RETRY_LOGIN:
					{
						// increase time for login resending, min time ~ 30 sec
						int32_t timeout = 30000;
						cnt.GetValue(FIELD1_PARAM, timeout);
						if (timeout > 300000)
							timeout = 300000;
						if (timeout < 30000)
							timeout = 30000;
						SetTimerObject(TIME_LGIN, timeout);
					}
					dwRet = VSTRCL_LOGIN_OK;
					break;
				default:
					m_LoginRetryCnt.Clear();
					dwRet = VSTRCL_ERR_PARAM;
					break;
				}
				if (!m_LoginRetryCnt.IsValid())
					RemoveTimerObjects(TIME_LGIN);
				RemoveTimerObjects(TIME_CHUSLS);
			}
			else
				dwRet = VSTRCL_ERR_CONTEYNER;
		}
		else if (Method && _stricmp(Method, USERLOGGEDOUT_METHOD) == 0) {
			int32_t result, cause;
			if (cnt.GetValue(RESULT_PARAM, result))	{
				if (m_Status.dwStatus&STATUS_CONFERENCE)
					Hangup();
				m_Status.dwStatus&=~STATUS_LOGGEDIN;
				switch((VS_UserLoggedout_Result )result)
				{
				case USER_ALREADY_LOGGEDOUT:
					dwRet = VSTRCL_LOGIN_OK;		break;
				case USER_LOGGEDOUT_OK:
					m_Status.MyInfo.Clean();
					cnt.GetValue(CAUSE_PARAM, cause);
					switch((VS_UserLoggedout_Cause )cause)
					{
					case USER_LOGGEDOUT_BY_REQUEST:
						// autologin have been cleaned
						dwRet = VSTRCL_LOGIN_OK;
						break;
					case USER_LOGGEDIN:
						// clear user to prevent autologin, must be filled again by user on startup
						m_AutoLoginKey.Empty();
						dwRet = VSTRCL_LOGIN_LGFROMS;
						break;
					default: 						dwRet = VSTRCL_ERR_PARAM;		break;
					}
					break;
				default:
					dwRet = VSTRCL_ERR_PARAM;			break;
				}
			}
			else	dwRet = VSTRCL_ERR_CONTEYNER;
			RemoveTimerObjects(TIME_LGOUT);
			RemoveTimerObjects(TIME_SESS_RENEW);
			SetEvent(m_LogoutEvent);
		}
		else if (Method && _stricmp(Method, AUTHORIZE_METHOD) == 0) {
			Authorize(cnt);
			dwRet = VSTRCL_LOGIN_OK;
		}
		else if (Method && _stricmp(Method, UPDATEACCOUNT_METHOD) == 0) {
			cnt.GetValueI32(RIGHTS_PARAM, m_Status.MyInfo.Rights);
			ChatRight(tMsg.SrcServer());
			cnt.GetValueI32(TARIFRESTR_PARAM, m_Status.MyInfo.TarifRestrictions);
			cnt.GetValueI32(SEPARATIONGROUP_PARAM, m_Status.MyInfo.SepartionGroup);
			const char * dn = cnt.GetStrValueRef(DISPLAYNAME_PARAM);
			const char * tp = cnt.GetStrValueRef(TARIFNAME_PARAM);
			const char * ss = cnt.GetStrValueRef(SESSION_PARAM);
			cnt.GetValue(PROPERTY_PARAM, m_UserProp);
			if (dn)
				m_Status.MyInfo.DisplayName = dn;
			if (tp)
				m_Status.MyInfo.TarifPlan = tp;
			if (ss)
				m_Status.MyInfo.SessionKey = ss;
			dwRet = VSTRCL_LOGIN_UPDATEACC;
		}
		else if (Method && _stricmp(Method, AUTH_BY_ECP_METHOD) == 0) {
			dwMessId = m_AuthECPContainers.AddList(&cnt);
			DTRACE(VSTM_PRTCL, "AuthByECP MessId=%d, ticketId=%s, ticketBody=%s", dwMessId, cnt.GetStrValueRef(TICKET_ID_PARAM), cnt.GetStrValueRef(TICKET_BODY_PARAM));
			dwRet = VSTRCL_LOGIN_BY_ECP;
		}
		else		dwRet = VSTRCL_ERR_METHOD;
		break;
	case transport::MessageType::Notify:
		dwRet = VSTRCL_ERR_TIMEOUT;
		break;
	default: 						dwRet = VSTRCL_ERR_UNKMESS;		break;
	}

	PostProc(dwRet, (dwMessId)? dwMessId: m_Status.MyInfo.Rights);
}


/**
****************************************************************************
* Initiate call to specified User.
*
* \return enum eVSTrClientErr
* \param UserName				- string containing specified User Name;
* \date    18-11-2002
******************************************************************************/
DWORD CVSTrClientProc::PlaseCall(char *CallId, int type)
{
	if (m_Status.dwStatus&STATUS_INCALL_PROGRESS) return VSTRCL_ERR_CURROP;
	m_Status.ConfInfo[0].CallId = CallId;
	m_Status.ConfInfo[0].confType = 0;
	if		(type==CT_PRIVATE) {
		return CreateConference(2, 30, CT_PRIVATE);
	}
	else {
		return CreateConference(20, 30, CT_HALF_PRIVATE);
	}
}

/**
****************************************************************************
* Creates public Conference
*
* \return ERR_OK
* \date    19-05-2003
******************************************************************************/
DWORD CVSTrClientProc::CreatePublicConf(VS_Conference_Type Type, char* pass)
{
	if (m_Status.dwStatus&STATUS_INCALL_PROGRESS) return VSTRCL_ERR_CURROP;
	m_Status.ConfInfo[0].confType = 2;
	return CreateConference(99, 60*60*24*10, Type, 0, pass);
}

/**
****************************************************************************
* Creates multiconference
*
* \return ERR_OK
* \date    02-04-2003
******************************************************************************/
DWORD CVSTrClientProc::CreateMultiConf(char* name, char* pass, VS_Conference_Type Type, int maxUsers, long SubType, long scope)
{
	if (m_Status.dwStatus&STATUS_INCALL_PROGRESS) return VSTRCL_ERR_CURROP;
	m_Status.ConfInfo[0].confType = 4;
	return CreateConference(maxUsers, 60*60*10, Type, name, pass, SubType, scope);
}


/**
****************************************************************************
* Send Create Conference Message.
*
* \return ERR_OK
* \param MaxPartisipants			- Maximum Number of participant in conference;
* \param Duration					- Duration of conference in sec (used with DestroyCondition);
* \param DestroyCondition			- Conference Destroy Condition method;
* \date    18-11-2002
******************************************************************************/
DWORD CVSTrClientProc::CreateConference(long MaxPartisipants, long Duration, long type, char* name, char* pass, long SubType, long scope)
{
	DTRACE(VSTM_PRTCL, "request = CreateConf: mp=%d, durr=%d, t=%d, st=%d, sc=%d", MaxPartisipants, Duration, type, SubType, scope);
	if (m_Status.ConfInfo[0].confState != CUserDesk::CONF_STATE_NONE) {
		DTRACE(VSTM_PRTCL, "\tnot send by 0!!!!");
		return VSTRCL_ERR_CURROP;
	}
	else if (m_Status.ConfInfo[1].confState&CUserDesk::CONF_STATE_REQEND) {
		DTRACE(VSTM_PRTCL, "\tnot send by 1!!!!");
		return VSTRCL_ERR_CURROP;
	}
	else if (m_Status.CurrConfInfo->confState == CUserDesk::CONF_STATE_DONE) {
		DTRACE(VSTM_PRTCL, "\tnot send curent is gone!!!!");
		return VSTRCL_ERR_CURROP;
	}
	else {
		DTRACE(VSTM_PRTCL, "\tsend...");
	}
	m_Status.ConfInfo[0].confState = m_Status.ConfInfo[0].CONF_STATE_REQB;
	m_Status.ConfInfo[0].Cname = name;
	m_Status.ConfInfo[0].Cpass = pass;

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, CREATECONFERENCE_METHOD);
	rCnt.AddValueI32(MAXPARTISIPANTS_PARAM, MaxPartisipants);
	rCnt.AddValueI32(DURATION_PARAM, Duration);
	rCnt.AddValueI32(TYPE_PARAM, type);
	rCnt.AddValueI32(SUBTYPE_PARAM, SubType);
	rCnt.AddValueI32(SCOPE_PARAM, scope);
	rCnt.AddValue(NAME_PARAM, name);
	rCnt.AddValue(PASSWORD_PARAM, pass);
	// set caps in private conf
	void* body;
	size_t bodySize;
	if (type==CT_PRIVATE)
		PrepareCaps(false);
	else
		PrepareCaps(true);
	if (m_Status.MyInfo.ClientCaps.Get(body, bodySize)) {
		rCnt.AddValue(CLIENTCAPS_PARAM, body, bodySize);
		free(body); body = 0; bodySize = 0;
	}

	ComposeSend(rCnt, CONFERENCE_SRV);
	SetTimerObject(TIME_CRCONF);
	return ERR_OK;
}

/**
****************************************************************************
* Send Delete Conference Message.
*
* \return ERR_OK
* \date    18-11-2002
******************************************************************************/
DWORD CVSTrClientProc::DeleteConference(const char* Conf)
{
	DTRACE(VSTM_PRTCL, "request   = DELETE");
	if (m_Status.dwStatus&STATUS_CONFERENCE) return Hangup();

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, DELETECONFERENCE_METHOD);
	rCnt.AddValue(NAME_PARAM, Conf);

	ComposeSend(rCnt, CONFERENCE_SRV, ReturnBroker(Conf));
	return ERR_OK;
}

/**
***************************************************************************
* Send Invite Message (in Place Call Chain to far User).
*
* \return ERR_OK
* \date    18-11-2002
******************************************************************************/
DWORD CVSTrClientProc::Invite()
{
	DTRACE(VSTM_PRTCL, "request   = INVITE %s send...", m_Status.ConfInfo[0].Conference.m_str);

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, INVITE_METHOD);
	rCnt.AddValue(CONFERENCE_PARAM, m_Status.ConfInfo[0].Conference);
	rCnt.AddValue(CALLID_PARAM, m_Status.ConfInfo[0].CallId);
	if (m_dwSuppressDirectCon)
		m_Status.ConfInfo[0].ConnectType = NO_DIRECT_CONNECT;
	else
		m_Status.ConfInfo[0].ConnectType = DIRECT_UNKNOWN;
	rCnt.AddValueI32(DIRECTCONNECT_PARAM, m_Status.ConfInfo[0].ConnectType);
	// set caps in private conf
	void* body;
	size_t bodySize;
	PrepareCaps(false);
	if (m_Status.MyInfo.ClientCaps.Get(body, bodySize)) {
		rCnt.AddValue(CLIENTCAPS_PARAM, body, bodySize);
		free(body); body = 0; bodySize = 0;
	}

	ComposeSend(rCnt, CONFERENCE_SRV, ReturnBroker(m_Status.ConfInfo[0].Conference));
	return ERR_OK;
}

/**
****************************************************************************
* Send Invite Message to Multi Conf
*
* \return ERR_OK
* \date    02-04-2004
******************************************************************************/
DWORD CVSTrClientProc::InviteToMulti(char* user)
{
	DTRACE(VSTM_PRTCL, "request   = InviteToMulti %s send...", m_Status.ConfInfo[0].Conference.m_str);

	if (m_Status.ConfInfo[2].UserName==(const char *)user) {
		SetReqInviteStatus(false);
	}
	m_Status.ConfInfo[2].UserName=user;
	m_Status.ConfInfo[2].Conference=m_Status.ConfInfo[0].Conference;

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, INVITE_METHOD);
	rCnt.AddValue(CONFERENCE_PARAM, m_Status.ConfInfo[0].Conference);
	rCnt.AddValueI32(TYPE_PARAM, m_Status.ConfInfo[0].confType == 4 ? CT_MULTISTREAM : CT_INTERCOM);
	rCnt.AddValue(CALLID_PARAM, user);
	rCnt.AddValue(NAME_PARAM, m_Status.ConfInfo[0].Cname);
	rCnt.AddValue(PASSWORD_PARAM, m_Status.ConfInfo[0].Cpass);

	ComposeSend(rCnt, CONFERENCE_SRV, ReturnBroker(m_Status.ConfInfo[0].Conference));
	return ERR_OK;
}

/**
****************************************************************************
* Request Invite from Host Of Multi Conf
*
* \return ERR_OK
* \date    02-04-2004
******************************************************************************/
DWORD CVSTrClientProc::ReqInviteToMulti(char* host)
{
	DTRACE(VSTM_PRTCL, "request   = ReqInviteToMulti %s send...", host);
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, REQINVITE_METHOD);
	rCnt.AddValueI32(TYPE_PARAM, CT_MULTISTREAM);
	rCnt.AddValue(CALLID_PARAM, host);
	// set sender have no caps to reinit audio sender
	void* body;
	size_t bodySize;
	PrepareCaps(true);
	if (m_Status.MyInfo.ClientCaps.Get(body, bodySize)) {
		rCnt.AddValue(CLIENTCAPS_PARAM, body, bodySize);
		free(body); body = 0; bodySize = 0;
	}

	ComposeSend(rCnt, CONFERENCE_SRV);

	m_Status.ConfInfo[0].confType = 4;
	m_Status.ConfInfo[0].confState = m_Status.ConfInfo[0].CONF_STATE_REQB;
	m_Status.ConfInfo[0].CallId = host;
	return ERR_OK;
}


/**
****************************************************************************
* Send Accept Message (in Place Call Chain from far User).
*
* \return ERR_OK
* \date    18-11-2002
*****************************************************************************/
DWORD CVSTrClientProc::Accept()
{
	DTRACE(VSTM_PRTCL, "request   = ACCEPT");
	SetIncallStatus(false);
	if (!m_Status.ConfInfo[1].Conference || !m_Status.ConfInfo[1].UserName)
		return VSTRCL_ERR_CURROP;
	if (m_Status.ConfInfo[1].confType==4 || m_Status.ConfInfo[1].confType==5)
		return Join(m_Status.ConfInfo[1].Cname, m_Status.ConfInfo[1].Cpass, m_Status.ConfInfo[1].confType==4 ? CT_MULTISTREAM : CT_INTERCOM, m_Status.ConfInfo[1].Conference);

	DTRACE(VSTM_PRTCL, "\tconf = %s", m_Status.ConfInfo[1].Conference.m_str);
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, ACCEPT_METHOD);
	rCnt.AddValue(CONFERENCE_PARAM, m_Status.ConfInfo[1].Conference);
	rCnt.AddValue(NAME_PARAM, m_Status.ConfInfo[1].UserName);
	rCnt.AddValue(USERNAME_PARAM, m_Status.MyInfo.UserName);
	rCnt.AddValue(APPID_PARAM,m_AppId);

	rCnt.AddValueI32(DIRECTCONNECT_PARAM, m_Status.ConfInfo[1].ConnectType);
	// set caps in private conf
	void* body;
	size_t bodySize;
	PrepareCaps(false);
	if (m_Status.MyInfo.ClientCaps.Get(body, bodySize)) {
		rCnt.AddValue(CLIENTCAPS_PARAM, body, bodySize);
		free(body); body = 0; bodySize = 0;
	}
	// ssl stream
	if (m_Status.MyInfo.ClientCaps.GetStreamsDC() & m_Status.ConfInfo[1].ClientCaps.GetStreamsDC() & VSCC_STREAM_CAN_DECODE_SSL) {
		char key[100];
		VS_GenKeyByMD5(key);
		rCnt.AddValue(SYMKEY_PARAM, key);
	}

	ComposeSend(rCnt, CONFERENCE_SRV, ReturnBroker(m_Status.ConfInfo[1].Conference));
	m_Status.ConfInfo[1].confState = m_Status.ConfInfo[1].CONF_STATE_COMING;
	return ERR_OK;
}

/**
****************************************************************************
* Send Reject Message (in Place Call Chain from far User).
*
* \return ERR_OK
* \date    18-11-2002
******************************************************************************/
DWORD CVSTrClientProc::Reject(char* conf, char* user, long cause)
{
	DTRACE(VSTM_PRTCL, "request   = REJECT");
	char *User, *Conf;
	if (conf && user) {
		User = user;
		Conf = conf;
	}
	else {
		if (m_Status.dwStatus&STATUS_REQINVITE) {
			User = m_Status.ConfInfo[2].UserName;
			Conf = m_Status.ConfInfo[2].Conference;
			SetReqInviteStatus(false);
		}
		else { // was Invite
			User = m_Status.ConfInfo[1].UserName;
			Conf = m_Status.ConfInfo[1].Conference;
			SetIncallStatus(false);
		}
	}

	DTRACE(VSTM_PRTCL, "\tconf = %s", Conf);

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, REJECT_METHOD);
	rCnt.AddValue(CONFERENCE_PARAM, Conf);
	rCnt.AddValue(NAME_PARAM, User);
	rCnt.AddValueI32(CAUSE_PARAM, cause);
	rCnt.AddValue(USERNAME_PARAM, m_Status.MyInfo.UserName);

	ComposeSend(rCnt, CONFERENCE_SRV, ReturnBroker(Conf));
	return ERR_OK;
}

/**
****************************************************************************
* Send Hangup Message. Terminate current participation in conference.
*
* \return ERR_OK
* \date    18-11-2002
******************************************************************************/
DWORD CVSTrClientProc::Hangup(int flags_int)
{
	DTRACE(VSTM_PRTCL, "request   = HUNGUP; flags_int = %d",flags_int);
	m_Status.CurrConfInfo->CloseStreams();
	const char* Conference = 0;
	if		(!!m_Status.ConfInfo[0].Conference) {
		Conference = m_Status.ConfInfo[0].Conference;
		SetTimerObject(TIME_DLCONF0);  // clean status in no responce from server
		m_Status.ConfInfo[0].confState |= CUserDesk::CONF_STATE_REQEND;
		m_Status.ConfInfo[1].Clean();
	}
	else if (!!m_Status.ConfInfo[1].Conference) {
		Conference = m_Status.ConfInfo[1].Conference;
		SetTimerObject(TIME_DLCONF1);  // clean status in no responce from server
		m_Status.ConfInfo[1].confState |= CUserDesk::CONF_STATE_REQEND;
		m_Status.ConfInfo[0].Clean();
	}
	else {
		m_Status.ConfInfo[0].Clean();
		m_Status.ConfInfo[1].Clean();
		m_Status.dwStatus &= (STATUS_LOGGEDIN|STATUS_SERVAVAIL);
		PostProc(VSTRCL_CONF_CALL);
	}

	if (Conference) {
		DTRACE(VSTM_PRTCL, "\t%s send...", Conference);

		VS_Container rCnt;
		rCnt.AddValue(METHOD_PARAM, HANGUP_METHOD);
		rCnt.AddValue(CONFERENCE_PARAM, Conference);
		rCnt.AddValue(NAME_PARAM, m_Status.MyInfo.UserName);
		HangupFlags flags(static_cast<HangupFlags>(flags_int));
		rCnt.AddValueI32(HANGUP_FLAGS_PARAM, flags);
		if (bool(flags & HangupFlags::FOR_ALL))
			rCnt.AddValueI32(RESULT_PARAM, 1);

		ComposeSend(rCnt, CONFERENCE_SRV, ReturnBroker(Conference));
	}
	return ERR_OK;
}

/**
****************************************************************************
* Send Join Message to Public conference by CallId if Host.
* \return ERR_OK
* \date    19-05-2003
******************************************************************************/
DWORD CVSTrClientProc::Join(char* CallId, char* pass, long type, char* Conference)
{
	DTRACE(VSTM_PRTCL, "request   = JOIN TO type=%d", type);
	if (m_Status.ConfInfo[0].confState != CUserDesk::CONF_STATE_NONE) {
		DTRACE(VSTM_PRTCL, "\twarn: prev operation not complete. send");
	}
	else if (m_Status.CurrConfInfo->confState == CUserDesk::CONF_STATE_DONE) {
		DTRACE(VSTM_PRTCL, "\tnot send curent is gone!!!!");
		return VSTRCL_ERR_CURROP;
	}
	else {
		DTRACE(VSTM_PRTCL, "\tsend...");
	}


	switch(type)
	{
	case CT_BROADCAST:
	case CT_VIPPUBLIC:
	case CT_PUBLIC:
	case CT_HALF_PRIVATE:
		m_Status.ConfInfo[0].confType = 3;
		m_Status.ConfInfo[0].CallId = CallId;
		break;
	case CT_MULTISTREAM:
		m_Status.ConfInfo[0].confType = 4;
		m_Status.ConfInfo[0].Cname = CallId;
		m_Status.ConfInfo[0].Cpass = pass;
		break;
	case CT_INTERCOM:
		m_Status.ConfInfo[0].confType = 5;
		m_Status.ConfInfo[0].Cname = CallId;
		m_Status.ConfInfo[0].Cpass = pass;
		break;
	default: return VSTRCL_ERR_CURROP;
	}
	m_Status.ConfInfo[0].confState = m_Status.ConfInfo[0].CONF_STATE_REQB;

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, JOIN_METHOD);
	rCnt.AddValueI32(TYPE_PARAM, type);
	rCnt.AddValue(PASSWORD_PARAM, pass?pass:"");
	rCnt.AddValue(NAME_PARAM, CallId);
	rCnt.AddValue(APPID_PARAM,m_AppId);
	rCnt.AddValue(USERNAME_PARAM, m_Status.MyInfo.UserName);
	rCnt.AddValue(DISPLAYNAME_PARAM, m_Status.MyInfo.DisplayName.m_str);
	// set sender have no caps to reinit audio sender
	void* body;
	size_t bodySize;
	PrepareCaps(true);
	if (m_Status.MyInfo.ClientCaps.Get(body, bodySize)) {
		rCnt.AddValue(CLIENTCAPS_PARAM, body, bodySize);
		free(body); body = 0; bodySize = 0;
	}

	ComposeSend(rCnt, CONFERENCE_SRV, ReturnBroker(Conference));
	m_podium_start_t = m_podium_time = 0;
	return ERR_OK;
}

/**
****************************************************************************
* Send query in to Multistream conference to (re)connect participant
* \param name		- participant name
* \param fltr		- type of connect: audio, video...
*
* \return ERR_OK
* \date    07-10-2003
******************************************************************************/
DWORD CVSTrClientProc::ConnectReceiver(char *name, long fltr)
{
	DTRACE(VSTM_PRTCL, "request   = CONNECT Recv %s %x", name, fltr);

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, CONNECTRECEIVER_METHOD);
	rCnt.AddValue(NAME_PARAM, name);
	rCnt.AddValue(USERNAME_PARAM, m_Status.MyInfo.UserName);
	rCnt.AddValueI32(MEDIAFILTR_PARAM, fltr);

	ComposeSend(rCnt, CONFERENCE_SRV, ReturnBroker(m_Status.CurrConfInfo->Conference));
	return ERR_OK;
}

/**
****************************************************************************
* Send query in to Multistream conference to connect collaboration sevices
* \param fltr		- type of connect: SlideShow, DesktopSharing ...
*
* \return ERR_OK
* \date    21-04-2009
******************************************************************************/
DWORD	CVSTrClientProc::ConnectServices(long fltr)
{
	DTRACE(VSTM_PRTCL, "request   = CONNECT Services %x", fltr);
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, CONNECTSERVICES_METHOD);
	rCnt.AddValue(USERNAME_PARAM, m_Status.MyInfo.UserName);
	rCnt.AddValueI32(SERVICES_PARAM, fltr);

	ComposeSend(rCnt, CONFERENCE_SRV, ReturnBroker(m_Status.CurrConfInfo->Conference));
	return ERR_OK;
}

/**
****************************************************************************
* Send query in to Multistream conference to connect sender wirth poit param
* \param fltr		- type of connect: audio, video...
*
* \return ERR_OK
* \date    09-07-2007
******************************************************************************/
DWORD CVSTrClientProc::ConnectSender(long fltr)
{
	DTRACE(VSTM_PRTCL, "request   = CONNECT Sender %x", fltr);

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, CONNECTSENDER_METHOD);
	rCnt.AddValue(USERNAME_PARAM, m_Status.MyInfo.UserName);
	rCnt.AddValueI32(MEDIAFILTR_PARAM, fltr);

	ComposeSend(rCnt, CONFERENCE_SRV, ReturnBroker(m_Status.CurrConfInfo->Conference));
	return ERR_OK;
}


/**
****************************************************************************
* Kick User from Conference.
* \return ERR_OK
* \date    19-05-2003
******************************************************************************/
DWORD CVSTrClientProc::Kick(const char* user)
{
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, KICK_METHOD);
	rCnt.AddValue(CONFERENCE_PARAM, m_Status.CurrConfInfo->Conference);
	rCnt.AddValue(NAME_PARAM, user);

	ComposeSend(rCnt, CONFERENCE_SRV, ReturnBroker(m_Status.CurrConfInfo->Conference));
	return ERR_OK;
}

/**
****************************************************************************
* Send query Ignore User chat.
* \return ERR_OK
* \date    02-06-2003
******************************************************************************/
DWORD CVSTrClientProc::Ignore(char *user)
{
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, IGNORE_METHOD);
	rCnt.AddValue(CONFERENCE_PARAM, m_Status.CurrConfInfo->Conference);
	rCnt.AddValue(NAME_PARAM, user);

	ComposeSend(rCnt, CONFERENCE_SRV, ReturnBroker(m_Status.CurrConfInfo->Conference));
	return ERR_OK;
}


/**
****************************************************************************
* Send Connect info to opposite user OnConferenceInvite;
* \return ERR_OK
* \date    02-06-2003
******************************************************************************/
DWORD CVSTrClientProc::SendUserConnectInfo(const char* server, const char * user)
{
	VS_Container rCnt;
	rCnt.Clear();
	rCnt.AddValue(METHOD_PARAM, CONFIGURATIONUPDATED_METHOD);
	rCnt.AddValue(NAME_PARAM, m_Status.MyInfo.UserName);
	for (const auto& x : m_Status.MyInfo.config.connTCP)
	{
		auto data = x->Serialize();
		if (data.empty())
			continue;

		rCnt.AddValue(IPCONFIG_PARAM, static_cast<const void*>(data.data()), data.size());
	}

	ComposeSend(rCnt, CONFERENCE_SRV, server, user);
	return ERR_OK;
}

/**
****************************************************************************
* Send Ping Configuration Message.
*
* \return ERR_OK
* \date    03-02-2003
******************************************************************************/
DWORD CVSTrClientProc::PingConference()
{
	VS_AutoLock lock(this);

	if (!(m_Status.dwStatus&STATUS_CONFERENCE))
		return ERR_OK;

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, PING_METHOD);
	rCnt.AddValue(CONFERENCE_PARAM, m_Status.CurrConfInfo->Conference);
	rCnt.AddValue(USERNAME_PARAM, m_Status.MyInfo.UserName);
	rCnt.AddValueI32(SYSLOAD_PARAM, m_loadSystem);

	ComposeSend(rCnt, CONFERENCE_SRV, ReturnBroker(m_Status.CurrConfInfo->Conference), 0, SEND_TIMEOUT/2);
	return ERR_OK;
}

/**
****************************************************************************
* Send MBps users list Message.
*
* \return ERR_OK
* \date    23-01-2013
******************************************************************************/
DWORD CVSTrClientProc::UpdateMBpsUsers()
{
	VS_AutoLock lock(this);

	if (!(m_Status.dwStatus&STATUS_CONFERENCE))
		return ERR_OK;

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, MBPSLIST_METHOD);
	rCnt.AddValue(CONFERENCE_PARAM, m_Status.CurrConfInfo->Conference);
	rCnt.AddValue(USERNAME_PARAM, m_Status.MyInfo.UserName);
	m_Status.CurrConfInfo->RsvList.GetFrameSizeMBUsersList(rCnt);

	ComposeSend(rCnt, CONFERENCE_SRV, ReturnBroker(m_Status.CurrConfInfo->Conference), 0, SEND_TIMEOUT/2);
	return ERR_OK;
}

/**
****************************************************************************
* Qery Role for point user
*
* \return ERR_OK
* \date    18-04-2007
******************************************************************************/
DWORD CVSTrClientProc::QueryRole(char *name, long role)
{
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, ROLEEVENT_METHOD);
	rCnt.AddValue(CONFERENCE_PARAM, m_Status.CurrConfInfo->Conference);
	rCnt.AddValueI32(TYPE_PARAM, RET_INQUIRY);
	rCnt.AddValue(USERNAME_PARAM, name);
	rCnt.AddValueI32(ROLE_PARAM, role);

	ComposeSend(rCnt, CONFERENCE_SRV, ReturnBroker(m_Status.CurrConfInfo->Conference));
	return ERR_OK;
}


/**
****************************************************************************
* Answer for Role query for point user
*
* \return ERR_OK
* \date    18-04-2007
******************************************************************************/
DWORD CVSTrClientProc::AnswerRole(char *name, long role, long result)
{
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, ROLEEVENT_METHOD);
	rCnt.AddValue(CONFERENCE_PARAM, m_Status.CurrConfInfo->Conference);
	rCnt.AddValueI32(TYPE_PARAM, RET_ANSWER);
	rCnt.AddValue(USERNAME_PARAM, name);
	rCnt.AddValueI32(ROLE_PARAM, role);
	rCnt.AddValueI32(RESULT_PARAM, result);

	ComposeSend(rCnt, CONFERENCE_SRV, ReturnBroker(m_Status.CurrConfInfo->Conference));
	return ERR_OK;
}


/**
***************************************************************************
* Change status on server
* \date    13.06.2007
*****************************************************************************/
DWORD CVSTrClientProc::DeviceStatusChanged()
{
	if (m_Status.dwStatus&STATUS_CONFERENCE) {
		VS_Container rCnt;
		rCnt.AddValue(METHOD_PARAM, DEVICESTATUS_METHOD);
		rCnt.AddValue(USERNAME_PARAM, m_Status.MyInfo.UserName);
		rCnt.AddValueI32(DEVICESTATUS_PARAM, g_DevStatus.GetStatus());

		ComposeSend(rCnt, CONFERENCE_SRV, ReturnBroker(m_Status.CurrConfInfo->Conference));
	}
	return ERR_OK;
}


/**
***************************************************************************
* Send Reply to invite directly to owner of conference
* \date    06.10.2007
*****************************************************************************/
DWORD CVSTrClientProc::InviteReply(long result)
{
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, INVITEREPLY_METHOD);
	rCnt.AddValue(NAME_PARAM, m_Status.ConfInfo[1].UserName);
	rCnt.AddValue(CONFERENCE_PARAM, m_Status.ConfInfo[1].Conference);
	rCnt.AddValue(USERNAME_PARAM, m_Status.MyInfo.UserName);
	rCnt.AddValueI32(RESULT_PARAM, result);

	ComposeSend(rCnt, CONFERENCE_SRV, ReturnBroker(m_Status.ConfInfo[1].Conference));
	return ERR_OK;
}

DWORD CVSTrClientProc::UpdateDisplayName(const char *alias, const wchar_t *displayName, bool updateImmediately)
{
	DTRACE(VSTM_PRTCL, "request   = UpdateDisplayName of %s is %S send", m_LastLogin.m_str, displayName);

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, USERREGISTRATIONINFO_METHOD);
	rCnt.AddValue(FROM_PARAM, m_LastLogin);
	rCnt.AddValue(ALIAS_PARAM, alias);
	rCnt.AddValue(DISPLAYNAME_PARAM, displayName);
	if (updateImmediately)
	{
		rCnt.AddValueI32(CMD_PARAM, 1);
		if (!!m_Status.ConfInfo[1].UserName)
			rCnt.AddValue(NAME_PARAM, m_Status.ConfInfo[1].UserName);
	}

	ComposeSend(rCnt, CONFERENCE_SRV);

	return ERR_OK;
}

DWORD CVSTrClientProc::SetMyLStatus(long lstatus)
{
	DTRACE(VSTM_PRTCL, "request = SetMyLStatus, status %d", lstatus);
	if (!m_Status.CurrConfInfo || !m_Status.CurrConfInfo->Conference)
		return 0;
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, SETMYLSTATUS_METHOD);
	rCnt.AddValueI32(LSTATUS_PARAM, lstatus);
	rCnt.AddValue(CALLID_PARAM, m_Status.MyInfo.UserName);
	rCnt.AddValue(CONFERENCE_PARAM, m_Status.CurrConfInfo->Conference);
	return ComposeSend(rCnt, CONFERENCE_SRV, ReturnBroker(m_Status.CurrConfInfo->Conference));
}

DWORD CVSTrClientProc::ClearAllLStatuses()
{
	DTRACE(VSTM_PRTCL, "request = ClearAllLStatuses");
	if (!m_Status.CurrConfInfo || !m_Status.CurrConfInfo->Conference)
		return 0;
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, CLEARALLLSTATUSES_METHOD);
	rCnt.AddValue(CALLID_PARAM, m_Status.MyInfo.UserName);
	rCnt.AddValue(CONFERENCE_PARAM, m_Status.CurrConfInfo->Conference);
	return ComposeSend(rCnt, CONFERENCE_SRV, ReturnBroker(m_Status.CurrConfInfo->Conference));
}


/**
****************************************************************************
* Conference service Messages interpreter.
*
* \param msg				- pointer to Window Message;
* \date    18-11-2002
******************************************************************************/
void CVSTrClientProc::ConferenceSrv(VS_ClientMessage &tMsg)
{
	DWORD dwRet = ERR_OK;
	VS_SimpleStr Method;

	switch(tMsg.Type())
	{
	case transport::MessageType::Invalid:
		dwRet = VSTRCL_ERR_INTERNAL;
		break;
	case transport::MessageType::Request:
	case transport::MessageType::Reply:
		{
			VS_Container cnt;
			if (cnt.Deserialize(tMsg.Body(), tMsg.BodySize())) {
				Method = cnt.GetStrValueRef(METHOD_PARAM);
				DTRACE(VSTM_PRTCL, "method    = %20s", Method.m_str);
				if (Method==CONFERENCECREATED_METHOD)
					dwRet = Method_ConferenceCreated(cnt);
				else if (Method == CONFERENCEDELETED_METHOD)
					dwRet = Method_ConferenceDeleted(cnt);
				else if (Method == USERREGISTRATIONINFO_METHOD)
					dwRet = Method_UserRegistrationInfo(cnt);
				else if (Method == CONFIGURATIONUPDATED_METHOD)
					dwRet = Method_ConfigurationUpdated(cnt);
				else if (Method == INVITE_METHOD)
					dwRet = Method_Invite(cnt);
				else if (Method == INVITETOMULTI_METHOD)
					dwRet = Method_InviteToMulti(cnt);
				else if (Method == INVITEREPLY_METHOD)
					dwRet = Method_InviteReply(cnt);
				else if (Method == ACCEPT_METHOD)
					dwRet = Method_Accept(cnt);
				else if (Method == REJECT_METHOD)
					dwRet = Method_Reject(cnt);
				else if (Method == JOIN_METHOD)
					dwRet = Method_Join(cnt);
				else if (Method == RECEIVERCONNECTED_METHOD)
					dwRet = Method_ReceiverConnected(cnt);
				else if (Method == SENDERCONNECTED_METHOD)
					dwRet = Method_SenderConnected(cnt);
				else if (Method == REQINVITE_METHOD)
					dwRet = Method_ReqInvite(cnt);
				else if (Method == LISTENERSFLTR_METHOD)
					dwRet = Method_ListenersFltr(cnt);
				else if (Method == SENDCOMMAND_METHOD)
					dwRet = Method_SendCommand(cnt);
				else if (Method == ROLEEVENT_METHOD)
					dwRet = Method_RoleEvent(cnt);
				else if (Method == DEVICESTATUS_METHOD)
					dwRet = Method_DeviceStatus(cnt);
				else if (Method == UPDATESETTINGS_METHOD)
					dwRet = Method_ConferenceSettings(cnt);
				else if (Method == FECC_METHOD)
					dwRet = Method_FECC(cnt);
				else
					dwRet = VSTRCL_ERR_METHOD;
			}
			else 	dwRet = VSTRCL_ERR_CONTEYNER;
		}
		break;
	case transport::MessageType::Notify:
		dwRet = VSTRCL_ERR_TIMEOUT;
		break;
	default: 						dwRet = VSTRCL_ERR_UNKMESS;		break;
	}
	PostProc(dwRet);
}


/**
****************************************************************************
* Notify message about conf creating.
******************************************************************************/
DWORD CVSTrClientProc::Method_ConferenceCreated(VS_Container& cnt)
{
	DWORD dwRet = 0;
	int32_t result;
	if (cnt.GetValue(RESULT_PARAM, result))	{
		if (result==CONFERENCE_CREATED_OK) {
			const char * Conf;
			Conf = cnt.GetStrValueRef(NAME_PARAM);
			DTRACE(VSTM_PRTCL, "\tconf = %s", Conf);
			m_Status.ConfInfo[0].Conference = Conf;
			if (m_Status.ConfInfo[0].confState&CUserDesk::CONF_STATE_REQEND || m_Status.ConfInfo[0].confState==CUserDesk::CONF_STATE_NONE) {
				DTRACE(VSTM_PRTCL, "\tdeleted known");
				DeleteConference(Conf);
			}
			else {
				DTRACE(VSTM_PRTCL, "\tcontinued");
				if (m_Status.ConfInfo[0].confType == 0) Invite();
				m_Status.ConfInfo[0].confState = CUserDesk::CONF_STATE_COMING;
			}
		}
		else {
			m_Status.ConfInfo[0].Clean();
			switch(result)
			{
			case VSS_CONF_LIC_LIMITED:		dwRet = VSTRCL_CONF_ONLINELIMIT;break;
			case CREATE_ACCESS_DENIED:		dwRet = VSTRCL_CONF_ACCDEN;	break;
			case VSS_CONF_ACCESS_DENIED:	dwRet = VSTRCL_CONF_ACCDEN;	break;
			case CREATE_HAVE_NO_MONEY:		dwRet = VSTRCL_CONF_HNOMON;	break;
			case VSS_CONF_EXISTS:			dwRet = VSTRCL_CONF_NAMEUSED;break;
			default: 						dwRet = VSTRCL_CONF_NOTCR;	break;
			}
		}
		RemoveTimerObjects(TIME_CRCONF);
	}
	else		dwRet = VSTRCL_ERR_CONTEYNER;
	return dwRet;
}


/**
****************************************************************************
* Notify message about conf deleting.
******************************************************************************/
DWORD CVSTrClientProc::Method_ConferenceDeleted(VS_Container& cnt)
{
	DWORD dwRet = 0;
	int32_t result;
	int32_t del_cause(0);
	if (cnt.GetValue(RESULT_PARAM, result))	{
		const char *Conf = cnt.GetStrValueRef(NAME_PARAM);
		cnt.GetValue(CAUSE_PARAM, del_cause);
		DTRACE(VSTM_PRTCL, "\tconf = %s del_cause = %d", Conf,del_cause);
		if ( Conf && m_Status.CurrConfInfo->Conference == Conf) {
			m_Status.dwStatus&=~STATUS_CONFERENCE;
			PostProc(VSTRCL_CONF_CALL);
			DTRACE(VSTM_PRTCL, "\tcommon");
		}
		if (Conf && m_Status.ConfInfo[0].Conference == Conf) {
			DTRACE(VSTM_PRTCL, "\tcase 0");
			if (m_Status.ConfInfo[0].confState == CUserDesk::CONF_STATE_COMING) {
				m_Status.ConfInfo[0].Clean();
				dwRet = VSTRCL_CONF_ACCEPTREJ;
			}
			else if (m_Status.ConfInfo[0].confState == CUserDesk::CONF_STATE_REQB) {
				VS_SimpleStr CallId = m_Status.ConfInfo[0].CallId;
				m_Status.ConfInfo[0].Clean();
				m_Status.ConfInfo[0].confState = CUserDesk::CONF_STATE_REQB;
				m_Status.ConfInfo[0].CallId = CallId;
				dwRet = VSTRCL_CONF_CALL;
				DTRACE(VSTM_PRTCL, "\tbut continue wait newly created");
			}
			else {
				m_Status.ConfInfo[0].Clean();
				dwRet = VSTRCL_CONF_CALL;
			}
			RemoveTimerObjects(TIME_DLCONF0);
		}
		else if (Conf && m_Status.ConfInfo[1].Conference==Conf){
			if (m_Status.dwStatus&(STATUS_INCALL|STATUS_INCALL_PROGRESS)) {
				SetIncallStatus(false);
				m_Status.dwStatus&=~STATUS_INCALL_PROGRESS;
			}
			if (m_Status.ConfInfo[1].confState == CUserDesk::CONF_STATE_COMING)
				dwRet = VSTRCL_CONF_ACCEPTREJ;
			else
				dwRet = VSTRCL_CONF_CALL;
			m_Status.ConfInfo[1].Clean();
			RemoveTimerObjects(TIME_DLCONF1);
			DTRACE(VSTM_PRTCL, "\tcase 1");
		}
		else	dwRet = VSTRCL_CONF_OK;
	}
	else		dwRet = VSTRCL_ERR_CONTEYNER;
	return dwRet;
}

/**
****************************************************************************
* Invite to conf.
******************************************************************************/
DWORD CVSTrClientProc::Method_Invite(VS_Container& cnt)
{
	DWORD dwRet = 0;
	const char *Conf = cnt.GetStrValueRef(CONFERENCE_PARAM);
	const char *User = cnt.GetStrValueRef(NAME_PARAM);
	if ( m_Status.dwStatus&(STATUS_INCALL_PROGRESS|STATUS_INCALL|STATUS_CONFERENCE) ) {
		Reject((char*)Conf, (char*)User, PARTISIPANT_IS_BUSY);
		dwRet = VSTRCL_CONF_OK;
	}
	else {
		int32_t val = NO_DIRECT_CONNECT;	cnt.GetValue(DIRECTCONNECT_PARAM, val);
		bool NeedConnectInfo = false;	cnt.GetValue(NEEDCONNECTINFO_PARAM, NeedConnectInfo);
		size_t size = 0;
		const void * buff = cnt.GetBinValueRef(CLIENTCAPS_PARAM, size);
		m_Status.ConfInfo[1].Conference = Conf;
		m_Status.ConfInfo[1].UserName = User;
		m_Status.ConfInfo[1].confType = 0;
		m_Status.ConfInfo[1].ConnectType = m_dwSuppressDirectCon ? NO_DIRECT_CONNECT : (VS_DirectConnect_Type)val;
		m_Status.ConfInfo[1].ClientCaps.Set(buff, size);
		if (NeedConnectInfo)
			SendUserConnectInfo(ReturnBroker(Conf), User);
		m_Status.ConfInfo[1].invNum = m_InviteContainers.AddList(&cnt);
		m_Status.dwStatus|=STATUS_INCALL_PROGRESS;
		SetTimerObject(TIME_TSTACC);
		dwRet = VSTRCL_CONF_OK;
	}
	return dwRet;
}


/**
****************************************************************************
* Current invite sequence user info.
******************************************************************************/
DWORD CVSTrClientProc::Method_UserRegistrationInfo(VS_Container& cnt)
{
	DWORD dwRet = VSTRCL_CONF_OK;
	const char *Un = cnt.GetStrValueRef(USERNAME_PARAM);
	const char *Dn = cnt.GetStrValueRef(DISPLAYNAME_PARAM);
	const char *CId = cnt.GetStrValueRef(CALLID_PARAM);
	if (m_Status.dwStatus&STATUS_INCALL_PROGRESS) {
		if (m_Status.ConfInfo[1].UserName==Un) {
			m_Status.ConfInfo[1].DisplayName = Dn;
			m_Status.ConfInfo[1].CallId = CId;
			m_Status.dwStatus|=STATUS_USERINFO;
			dwRet = VSTRCL_CONF_CALL;
		}
	}
	else {
		if (CId && m_Status.ConfInfo[0].CallId.iCompare(CId)) {
			m_Status.ConfInfo[0].DisplayName = Dn;
			m_Status.ConfInfo[0].UserName = Un;
			m_Status.dwStatus|=STATUS_USERINFO;
			dwRet = VSTRCL_CONF_CALL;
		}
	}
	return dwRet;
}


/**
****************************************************************************
* Current invite sequence endpoint info.
******************************************************************************/
DWORD CVSTrClientProc::Method_ConfigurationUpdated(VS_Container& cnt)
{
	DWORD dwRet = 0;
	const char * UserName = cnt.GetStrValueRef(NAME_PARAM);
	CUserDesk *usd;
	if (m_Status.dwStatus&STATUS_INCALL_PROGRESS)
		usd = &m_Status.ConfInfo[1];
	else
		usd = &m_Status.ConfInfo[0];
	if (usd->UserName == UserName) {
		if (!m_dwSuppressDirectCon && usd->ConnectType!=NO_DIRECT_CONNECT) {
			if (m_Status.MyInfo.UserName!=UserName) { // use dirrect connection test
				// prepare NHP
				unsigned long NhpTime = TESTNHP_TIMEOUT;
				unsigned long NhpStartTime = timeGetTime();
				//Prepare Direct UDP
				unsigned long udp_connect_timeout = NhpTime;
				unsigned long UdpConnectStartTime = NhpStartTime;
				bool NhpResult = false, UdpResult = false;

				if (m_dwUseNhp) {
					CUserDesk::Trmtr->CloseConnection();
					ResetEvent(m_NhpEvent);
					char ip[256] = {0};
					long lenght = sizeof(ip);
					VS_GetEndpointSourceIP(m_CurrBroker, ip, lenght);
					NhpResult = CUserDesk::Trmtr->ConnectNHP(UserName, ReturnBroker(usd->Conference), usd->Conference, m_NhpEvent, NhpTime, ip);
				}
				// prepare Direct Connect
				DWORD num = 0;
				cnt.Reset();
				while (cnt.Next())
					if (_stricmp(cnt.GetName(), IPCONFIG_PARAM) == 0 || _stricmp(cnt.GetName(), IP6CONFIG_PARAM) == 0) num++;
				usd->config.Clean(); /// reset to zero
				if (num) {
					net::endpoint::ConnectTCP ProxyTCP;
					VSProxySettings::ReadProxyTCP(ProxyTCP);
					bool useProxy = !boost::iequals(ProxyTCP.protocol_name, string_view(net::endpoint::protocol_tcp));

					// This variable sets the priority of address family (ipv4 or ipv6), that will be used in DirectUDP.
					// Values:
					// 1				- try to find ipv4 address, otherwise - use ipv6
					// 2				- try to find ipv6 address, otherwise - use ipv4
					// any other number - use the first direct udp connection
					int ip_priority = 0;
					if(!(m_Status.dwStatus&STATUS_INCALL_PROGRESS))
					{
						// if we call somebody
						// we set the priority of using ipv4 or ipv6
						VS_RegistryKey key(true, REG_CurrentConfiguratuon, false, true);
						key.GetValue(&ip_priority, 4, VS_REG_INTEGER_VT, REG_DirectUDP_IPPriority);
						if(ip_priority != 2) ip_priority = 1; // Only '1' or '2' values are valid. '1' is default.
					}
					// Else, if somebody calls us, we use his priority.
					// It means, we must use the first DirectUDP connection we found.

					bool directUDPFound = false;
					std::unique_ptr<net::endpoint::ConnectTCP> directUdpConn;

					// Enumerate all configuration
					usd->config.connTCPSucs.clear();
					usd->config.connTCP.clear();
					cnt.Reset();
					while (cnt.Next())
					{
						const void *body = 0;
						size_t bodySize = 0;

						bool is_ipv6;
						// We read only IPCONFIG_PARAM, IP6CONFIG_PARAM.
						if (_stricmp(cnt.GetName(), IPCONFIG_PARAM) == 0)
							is_ipv6 = false;
						else if(_stricmp(cnt.GetName(), IP6CONFIG_PARAM) == 0)
							is_ipv6 = true;
						else
							continue;

						body = cnt.GetBinValueRef(bodySize);

						auto conn = std::make_unique<net::endpoint::ConnectTCP>();
						conn->Deserialize(body, bodySize);
						conn->is_ipv6 = is_ipv6;

						if (boost::iequals(conn->protocol_name, string_view(net::endpoint::protocol_direct_udp)))
						{
							if(!directUDPFound)
							{
								directUdpConn = std::move(conn);

								switch(ip_priority)
								{
								case 1: // IPV4 > IPV6
									if (directUdpConn->is_ipv6 == false)
										directUDPFound = true;
									break;
								case 2: // IPV6 > IPV4
									if (directUdpConn->is_ipv6 == true)
										directUDPFound = true;
									break;
								default:
									directUDPFound = true; // use the first connection
								}
							}
						}
						else
						{
							if (useProxy && VS_CheckIsInternetAddress(conn->host.c_str()))
							{
								conn->protocol_name = ProxyTCP.protocol_name;
								conn->socks_host = ProxyTCP.socks_host;
								conn->socks_port = ProxyTCP.socks_port;
								conn->socks_user = ProxyTCP.socks_user;
								conn->socks_password = ProxyTCP.socks_password;
								conn->socks_version = ProxyTCP.socks_version;
								conn->http_host = ProxyTCP.http_host;
								conn->http_port = ProxyTCP.http_port;
								conn->http_user = ProxyTCP.http_user;
								conn->http_password = ProxyTCP.http_password;
							}
							usd->config.connTCP.emplace_back(std::move(conn));
						}
					} // end while

					if (m_dwUseNhp && directUdpConn) {
						CUserDesk::DirectUdpTrmtr->CloseConnection();
						ResetEvent(m_DirectUdpConnEvent);
						char	epNm[256];
						VS_EndpointName(epNm,sizeof(epNm));

						UdpResult = CUserDesk::DirectUdpTrmtr->ConnectUDP(directUdpConn->is_ipv6 ? VS_IPPortAddress::ADDR_IPV6 : VS_IPPortAddress::ADDR_IPV4,
							directUdpConn->host.c_str(), directUdpConn->port,
							usd->Conference, epNm, UserName, m_DirectUdpConnEvent, udp_connect_timeout);
					}

					unsigned long TryTime = TESTDCONNECT_TIMEOUT;
					VS_TestAccessibility(usd->config.connTCP, usd->config.connTCPSucs, usd->UserName, TryTime);
					DTRACE(VSTM_PRTCL, "Direct TryTime = %d, sucs=%d", TESTDCONNECT_TIMEOUT - TryTime, usd->config.connTCPSucs.size());
					if (!usd->config.connTCPSucs.empty())
					{
						net::endpoint::ClearAllConnectTCP(usd->UserName.m_str); // clean to avoid accumulation
						for (const auto& x : usd->config.connTCPSucs)
							net::endpoint::AddConnectTCP(*x, usd->UserName.m_str);
					}
				}

				int UdpConnWaitTime = UdpConnectStartTime + udp_connect_timeout + 100 - timeGetTime();
				if (UdpConnWaitTime < 0) UdpConnWaitTime = 0;
				bool UdpConnOk = UdpResult && WaitForSingleObject(m_DirectUdpConnEvent,UdpConnWaitTime) == WAIT_OBJECT_0;
				DTRACE(VSTM_PRTCL, "UDP Time = %4d, try=%d, resconn=%d, ok=%d", timeGetTime()-UdpConnectStartTime, m_dwUseNhp, UdpResult, UdpConnOk);

				// calculate time for nhp is to end
				int NhpWaitTime = NhpStartTime + NhpTime+100 - timeGetTime();
				if (NhpWaitTime < 0) NhpWaitTime = 0;
				// reset wait NHP time if UDP ok
				if (UdpConnOk) NhpWaitTime = 0;
				// check if nhp alredy finished
				bool NhpOk = NhpResult && WaitForSingleObject(m_NhpEvent, NhpWaitTime)== WAIT_OBJECT_0;
				DTRACE(VSTM_PRTCL, "NHP Time = %4d, try=%d, resconn=%d, ok=%d", timeGetTime()-NhpStartTime, m_dwUseNhp, NhpResult, NhpOk);
				if (m_Status.dwStatus&STATUS_INCALL_PROGRESS) {
					if (UdpConnOk)
						usd->ConnectType = DIRECT_UDP;
					else if (m_dwForceNhp && NhpOk)
						usd->ConnectType = DIRECT_NHP;
					else if (usd->config.connTCPSucs.size())
						usd->ConnectType = DIRECT_CONNECT;
					else if (IsTimerObjectElapsed(TIME_TSTACC)) {
						if (NhpOk) // m_dwForceNhp=0
							usd->ConnectType = DIRECT_NHP;
						else
							usd->ConnectType = NO_DIRECT_CONNECT;
						RemoveTimerObjects(TIME_TSTACC);
					}
					else{} // go out to wait for timer or Direct from other user
				}
				else {
					if (usd->config.connTCPSucs.size())
						ClientTryAcceptDirect(usd->Conference, usd->UserName);
				}
			}
			// myself call, force to use direct connection without test
			else	usd->ConnectType = DIRECT_SELF;
		}
		else		usd->ConnectType = NO_DIRECT_CONNECT;

		dwRet = VSTRCL_CONF_OK;
		if (usd->ConnectType!= DIRECT_UNKNOWN)
			if (m_Status.dwStatus&STATUS_INCALL_PROGRESS)
				SetIncallStatus(true, VSTRCL_CONF_CALL, usd->invNum);
	}
	else	dwRet = VSTRCL_CONF_OK;
	return dwRet;
}



/**
****************************************************************************
* Accept for request to invite.
******************************************************************************/
DWORD CVSTrClientProc::Method_Accept(VS_Container& cnt)
{
	DWORD dwRet = 0;
	const char *Conf, *User;
	Conf = cnt.GetStrValueRef(CONFERENCE_PARAM);
	User = cnt.GetStrValueRef(NAME_PARAM);
	size_t size = 0;
	const void * buff = cnt.GetBinValueRef(CLIENTCAPS_PARAM, size);
	DTRACE(VSTM_PRTCL, "\tconf = %s", Conf);
	if (Conf) {
		bool valid = true;
		if (m_Status.ConfInfo[0].Conference == Conf) {
			m_Status.ConfInfo[0].ClientCaps.Set(buff, size);
			if (m_Status.CurrConfInfo->confState&CUserDesk::CONF_STATE_REQEND)
				valid = false;
			else
				m_Status.CurrConfInfo = &m_Status.ConfInfo[0];
		}
		else if (m_Status.ConfInfo[1].Conference == Conf) {
			m_Status.CurrConfInfo = &m_Status.ConfInfo[1];
		}
		else {
			DTRACE(VSTM_PRTCL, "\tunknown !!!");
			m_Status.CurrConfInfo = &m_Status.ConfInfo[m_Status.MAX_CONFINFO-1];
			valid = false;
		}
		if (valid) {
			int32_t val = 0;
			cnt.GetValue(DIRECTCONNECT_PARAM, val);
			if      (val==DIRECT_CONNECT)	m_Status.CurrConfInfo->ConnectType = DIRECT_ACCEPT;
			else if (val==DIRECT_ACCEPT)	m_Status.CurrConfInfo->ConnectType = DIRECT_CONNECT;
			else							m_Status.CurrConfInfo->ConnectType = (VS_DirectConnect_Type )val;
			m_Status.CurrConfInfo->ConfSimKey = cnt.GetStrValueRef(SYMKEY_PARAM);
			cnt.GetValueI32(CMR_FLAGS_PARAM, m_Status.CurrConfInfo->cmr_flags);
			m_Status.dwStatus|=STATUS_CONFERENCE;
			dwRet = VSTRCL_CONF_CALL;
		}
		else {
			DeleteConference(Conf);
			dwRet = VSTRCL_CONF_OK;
		}
	}
	else	dwRet = VSTRCL_CONF_OK;
	return dwRet;
}


/**
****************************************************************************
* reject from request to invite or to join.
******************************************************************************/
DWORD CVSTrClientProc::Method_Reject(VS_Container& cnt)
{
	DWORD dwRet = 0;
	const char *Conf, *User;
	int32_t cause;
	Conf = cnt.GetStrValueRef(CONFERENCE_PARAM);
	User = cnt.GetStrValueRef(CALLID_PARAM);
	DTRACE(VSTM_PRTCL, "\tconf = %s", Conf);
	cnt.GetValue(CAUSE_PARAM, cause);
	switch((VS_Reject_Cause )cause)
	{
	case REJECTED_BY_PARTICIPANT:		dwRet = VSTRCL_CONF_REJBYPART;	break;
	case CONFERENCE_IS_BUSY:			dwRet = VSTRCL_CONF_CONFBUSY;	break;
	case PARTISIPANT_IS_BUSY:			dwRet = VSTRCL_CONF_PBUSY;		break;
	case PARTISIPANT_NOT_AVAILABLE_NOW:	dwRet = VSTRCL_CONF_PNOTAV;		break;
	case INVALID_CONFERENCE:			dwRet = VSTRCL_CONF_INVCONF;	break;
	case INVALID_PARTISIPANT:			dwRet = VSTRCL_CONF_INVP;		break;
	case REACH_MONEY_LIMIT	:			dwRet = VSTRCL_CONF_HNOMON;		break;
	case REJECTED_BY_ACCESS_DENIED:		dwRet = VSTRCL_CONF_ACCDEN;		break;
	case REJECTED_BY_RESOURCE_LIMIT:	dwRet = VSTRCL_CONF_NOTCR;		break;
	case REJECTED_BY_NOFRIEND:			dwRet = VSTRCL_CONF_NOFRIEND;	break;
	case REJECTED_BY_BADRATING:			dwRet = VSTRCL_CONF_BADRATING;	break;
	case REJECTED_BY_TIMEOUT:			dwRet = VSTRCL_CONF_USERTIMEOUT;break;
	default:							dwRet = VSTRCL_ERR_PARAM;		break;
	}
	bool IsConfKnown = false;
	if (Conf && m_Status.ConfInfo[0].Conference == Conf && m_Status.ConfInfo[0].confType==0) {										// was Invite
		m_Status.ConfInfo[0].Clean();
		IsConfKnown = true;
	}
	if (Conf && m_Status.ConfInfo[1].Conference == Conf && m_Status.ConfInfo[1].confType==0) {										// was Accept
		m_Status.ConfInfo[1].Clean();
		IsConfKnown = true;
	}
	if (Conf && m_Status.ConfInfo[2].Conference == Conf) {	// was Inv to Multi
		m_Status.ConfInfo[2].Clean();
		IsConfKnown = true;
	}
	if (User && m_Status.ConfInfo[0].CallId.iCompare( User)
		&& (m_Status.ConfInfo[0].confType==4 || m_Status.ConfInfo[0].confType==5)
		&& m_Status.ConfInfo[0].confState != CUserDesk::CONF_STATE_DONE	)
	{														// was ReqInvite
		m_Status.ConfInfo[0].Clean();
		IsConfKnown = true;
	}
	if (!IsConfKnown)
		dwRet = VSTRCL_CONF_OK;
	return dwRet;
}


/**
****************************************************************************
* Join response.
******************************************************************************/
DWORD CVSTrClientProc::Method_Join(VS_Container& cnt)
{
	DWORD dwRet = 0;
	int32_t result = 0;	cnt.GetValue(RESULT_PARAM, result);
	size_t size = 0;
	const void * buff = cnt.GetBinValueRef(CLIENTCAPS_PARAM, size);
	switch(result)
	{
	case JOIN_OK: {
		m_Status.ConfInfo[0].Conference = cnt.GetStrValueRef(CONFERENCE_PARAM);
		m_Status.ConfInfo[0].UserName = cnt.GetStrValueRef(USERNAME_PARAM);
		m_Status.ConfInfo[0].DisplayName = cnt.GetStrValueRef(DISPLAYNAME_PARAM);
		//if (!m_Status.ConfInfo[0].DisplayName) m_Status.ConfInfo[0].DisplayName = cnt.GetStrValueRef(NAME_PARAM);
		m_Status.ConfInfo[0].Cname = cnt.GetStrValueRef(NAME_PARAM); // real conference name
		m_Status.dwStatus|=STATUS_USERINFO;
		m_Status.ConfInfo[0].ServerType = CT_MULTISTREAM; cnt.GetValueI32(TYPE_PARAM, m_Status.ConfInfo[0].ServerType);
		m_Status.ConfInfo[0].ServerSubType = GCST_FULL; cnt.GetValueI32(SUBTYPE_PARAM, m_Status.ConfInfo[0].ServerSubType);
		m_Status.ConfInfo[0].ServerScope = GS_PERSONAL; cnt.GetValueI32(SCOPE_PARAM, m_Status.ConfInfo[0].ServerScope);
		m_Status.ConfInfo[0].MaxCasts = 1; cnt.GetValueI32(MAXCAST_PARAM, m_Status.ConfInfo[0].MaxCasts);
		m_Status.ConfInfo[0].MaxParts = 1; cnt.GetValueI32(MAXPARTISIPANTS_PARAM, m_Status.ConfInfo[0].MaxParts);
		m_Status.ConfInfo[0].SVCMode = 0; cnt.GetValueI32(SVCMODE_PARAM, m_Status.ConfInfo[0].SVCMode);
		m_Status.ConfInfo[0].LStatus = 0; cnt.GetValueI32(LSTATUS_PARAM, m_Status.ConfInfo[0].LStatus);
		cnt.GetValueI32(CMR_FLAGS_PARAM, m_Status.ConfInfo[0].cmr_flags);
		//if (m_Status.ConfInfo[0].ServerType != CT_INTERCOM) {
		//	m_Status.dwStatus|=STATUS_CONFERENCE;
		//	m_Status.CurrConfInfo = &m_Status.ConfInfo[0];
		//}
		//else // intercom
		//	m_Status.ConfInfo[0].confType=5;
		if (m_Status.ConfInfo[0].ServerType == CT_INTERCOM)
			m_Status.ConfInfo[0].confType=5;
		m_Status.dwStatus|=STATUS_CONFERENCE;
		m_Status.CurrConfInfo = &m_Status.ConfInfo[0];

		m_Status.ConfInfo[0].lfltr = 0; // don't send anything at start
		m_Status.ConfInfo[0].ConnectType = NO_DIRECT_CONNECT;
		m_Status.ConfInfo[0].ClientCaps.Set(buff, size);
		m_Status.ConfInfo[0].ConfSimKey = cnt.GetStrValueRef(SYMKEY_PARAM);

		/// check if not waiting for join
		if (m_Status.ConfInfo[0].confState==CUserDesk::CONF_STATE_NONE) {
			Hangup();
			break;
		}
		else {
			m_Status.ConfInfo[0].confState = m_Status.ConfInfo[0].CONF_STATE_COMING;
			if (m_Status.ConfInfo[0].UserName == m_Status.MyInfo.UserName)
				dwRet = VSTRCL_CONF_IAM_HOST; // do not work at join in intercom
			else
				dwRet = VSTRCL_CONF_CALL;
			PostProc(dwRet, m_Status.ConfInfo[0].ServerSubType);
			dwRet = VSTRCL_CONF_OK;
		}
				  }
			break;
	case VSS_CONF_MAX_PART_NUMBER:		dwRet = VSTRCL_CONF_CONFBUSY;	break;
	case VSS_CONF_NOT_STARTED:			dwRet = VSTRCL_CONF_NOT_STARTED;break;
	case VSS_CONF_NOT_FOUND:			dwRet = VSTRCL_CONF_INVPARAM;	break;
	case VSS_CONF_ACCESS_DENIED:		dwRet = VSTRCL_CONF_INVPARAM;	break;
	case VSS_CONF_ROUTER_DENIED:		dwRet = VSTRCL_CONF_NOTCR;		break;
	case VSS_CONF_EXPIRED:				dwRet = VSTRCL_CONF_EXPIRED;	break;
	case REJECTED_BY_LOCAL_RES:			dwRet = VSTRCL_CONF_LOCALBUSY;	break;
	case VSS_BROKER_LOGIC_ERROR:		dwRet = VSTRCL_CONF_NOTCR;		break;
	case VSS_DB_ERROR:					dwRet = VSTRCL_CONF_NOTCR;		break;
	case VSS_DB_MEM_ERROR:				dwRet = VSTRCL_CONF_NOTCR;		break;
	case VSS_CONF_LIC_LIMITED:			dwRet = VSTRCL_CONF_ONLINELIMIT;break;
	case CONFERENCE_PASSWORD_REQUIRED:	dwRet = VSTRCL_CONF_PASSREQ;	break;
	case REJECTED_BY_WRONG_PASSWORD:	dwRet = VSTRCL_CONF_PASSWRONG;	break;
		// old values
	case REJECTED_BY_PARTICIPANT:		dwRet = VSTRCL_CONF_REJBYPART;	break;
	case CONFERENCE_IS_BUSY:			dwRet = VSTRCL_CONF_CONFBUSY;	break;
	case PARTISIPANT_IS_BUSY:			dwRet = VSTRCL_CONF_PBUSY;		break;
	case PARTISIPANT_NOT_AVAILABLE_NOW:	dwRet = VSTRCL_CONF_PNOTAV;		break;
	case INVALID_CONFERENCE:			dwRet = VSTRCL_CONF_INVCONF;	break;
	case INVALID_PARTISIPANT:			dwRet = VSTRCL_CONF_INVP;		break;
	case REACH_MONEY_LIMIT	:			dwRet = VSTRCL_CONF_HNOMON;		break;
	case REJECTED_BY_ACCESS_DENIED:		dwRet = VSTRCL_CONF_ACCDEN;		break;
	case REJECTED_BY_RESOURCE_LIMIT:	dwRet = VSTRCL_CONF_NOTCR;		break;

	default:							dwRet = VSTRCL_CONF_NOTCR;		break;
	}
	if (result!=JOIN_OK)
		m_Status.ConfInfo[0].confState = m_Status.ConfInfo[0].CONF_STATE_NONE;
	return dwRet;
}


/**
****************************************************************************
* Receiver connected method.
******************************************************************************/
DWORD CVSTrClientProc::Method_ReceiverConnected(VS_Container& cnt)
{
	DWORD dwRet = 0;
	int32_t result = 0; cnt.GetValue(RESULT_PARAM, result);

	VSClientRcvDesc rd;
	rd.m_name = cnt.GetStrValueRef(USERNAME_PARAM);
	rd.m_conf = m_Status.ConfInfo[0].Conference;
	if (!!rd.m_name && !!rd.m_conf) {
		if (result == SCR_CONNECTED_OK) {
			cnt.GetValueI32(TYPE_PARAM, rd.m_type);
			cnt.GetValueI32(MEDIAFILTR_PARAM, rd.m_fltr);
			cnt.GetValueI32(DEVICESTATUS_PARAM, rd.m_dvs);
			if		(rd.m_type == RCT_ROUTER) {
				rd.m_endp = ReturnBroker(rd.m_conf);
				rd.m_part = cnt.GetStrValueRef(NAME_PARAM);
				rd.m_user = m_Status.MyInfo.UserName;
			}
			else {// if (rd.m_type == RCT_INTERCOM) {
				rd.m_mcast = cnt.GetStrValueRef(MULTICAST_IP_PARAM);
				//rd.m_host = cnt.GetStrValueRef(HOST_PARAM);
				cnt.GetValueI32(PORT_PARAM, rd.m_port);
				cnt.GetValueI32(KEY_PARAM, rd.m_ckey);
				char ip[256] = {0};
				long lenght = sizeof(ip);
				VS_GetEndpointSourceIP(m_CurrBroker, ip, lenght);
				rd.m_srcip = ip;
			}
			size_t size = 0;
			const void* buff = cnt.GetBinValueRef(CLIENTCAPS_PARAM, size);
			rd.m_caps.Set(buff, size);
			rd.m_state = rd.RDSTATE_NEW;
			rd.m_symkey = cnt.GetStrValueRef(SYMKEY_PARAM);
			dwRet = VSTRCL_RCV_NEW;
		}
		else if (result == SCR_DISCONNECTED_OK) {
			rd.m_state = rd.RDSTATE_REMOVED;
			dwRet = VSTRCL_RCV_REM;
		}
		else if (result == SCR_CHANGED_OK) {
			cnt.GetValueI32(MEDIAFILTR_PARAM, rd.m_fltr);
			cnt.GetValueI32(DEVICESTATUS_PARAM, rd.m_dvs);
			rd.m_state = rd.RDSTATE_CHANGED;
			dwRet = VSTRCL_RCV_UPD;
		}
		m_Status.ConfInfo[0].RsvList.AddAction(rd);
		PostProc(dwRet, m_Status.ConfInfo[0].RsvList.GetId());
		dwRet = VSTRCL_CONF_OK;
	}
	else {
		dwRet = VSTRCL_RCV_FAIL;
	}
	return dwRet;
}

/**
****************************************************************************
* Intercom sender connected result
******************************************************************************/
DWORD CVSTrClientProc::Method_SenderConnected(VS_Container& cnt)
{
	DWORD dwRet = 0;
	int32_t result = 0;	cnt.GetValue(RESULT_PARAM, result);
	if (result == SCR_CONNECTED_OK) {
		m_Status.MyInfo.config.connTCPSucs.clear();

		int32_t port = 0; cnt.GetValue(PORT_PARAM, port);
		const char *host = cnt.GetStrValueRef(HOST_PARAM);
		const char *mcast_ip = cnt.GetStrValueRef(MULTICAST_IP_PARAM);

		if (m_Status.MyInfo.config.connTCPSucs.size() < 2)
			m_Status.MyInfo.config.connTCPSucs.resize(2);
		m_Status.MyInfo.config.connTCP.emplace_back(new net::endpoint::ConnectTCP{ (host ? host : ""), net::port(port), "bcast" });// argument host will be passed to std::string constructor and value 0 will provoke an error
		m_Status.MyInfo.config.connTCPSucs[0] = m_Status.MyInfo.config.connTCP.back().get();
		m_Status.MyInfo.config.connTCP.emplace_back(new net::endpoint::ConnectTCP{ (mcast_ip ? mcast_ip : ""), net::port(port), "mcast" });// argument mcast_ip will be passed to std::string constructor and value 0 will provoke an error
		m_Status.MyInfo.config.connTCPSucs[1] = m_Status.MyInfo.config.connTCP.back().get();

		m_Status.MyInfo.ConferenseKey = 0;
		cnt.GetValueI32(KEY_PARAM, m_Status.MyInfo.ConferenseKey);

		//m_Status.dwStatus|=STATUS_CONFERENCE;
		//m_Status.CurrConfInfo = &m_Status.ConfInfo[0];
		//dwRet = m_Status.ConfInfo[0].UserName == m_Status.MyInfo.UserName ? VSTRCL_CONF_IAM_HOST : VSTRCL_CONF_CALL;
	}
	else {
		dwRet = VSTRCL_CONF_OK;
	}
	return dwRet;
}


/**
****************************************************************************
* reguest to Query to joinn to multiconf.
******************************************************************************/
DWORD CVSTrClientProc::Method_ReqInvite(VS_Container& cnt)
{
	DWORD dwRet = 0;
	if (!!m_Status.CurrConfInfo->Conference && (m_Status.dwStatus&STATUS_CONFERENCE)) {
		const char *Conf, *User;
		Conf = m_Status.CurrConfInfo->Conference;
		User = cnt.GetStrValueRef(CALLID_PARAM);

		if (m_Status.dwStatus&(STATUS_INCALL_PROGRESS|STATUS_INCALL|STATUS_REQINVITE)) {
			Reject((char*)Conf, (char*)User, PARTISIPANT_IS_BUSY);
			dwRet = VSTRCL_CONF_OK;
		}
		else {
			m_Status.ConfInfo[2].Conference = Conf;
			m_Status.ConfInfo[2].UserName = User;
			m_Status.ConfInfo[2].DisplayName = cnt.GetStrValueRef(DISPLAYNAME_PARAM);
			SetReqInviteStatus(true);
			size_t size = 0;
			const void * buff = cnt.GetBinValueRef(CLIENTCAPS_PARAM, size);
			m_Status.ConfInfo[2].ClientCaps.Set(buff, size);
			dwRet = VSTRCL_CONF_CALL;
		}
	}
	return dwRet;
}


/**
****************************************************************************
* Query to joinn to multiconf.
******************************************************************************/
DWORD CVSTrClientProc::Method_InviteToMulti(VS_Container& cnt)
{
	DWORD dwRet = VSTRCL_CONF_OK;
	const char *Conf = cnt.GetStrValueRef(CONFERENCE_PARAM);
	const char *User = cnt.GetStrValueRef(USERNAME_PARAM);
	int32_t type = CT_MULTISTREAM;
	cnt.GetValue(TYPE_PARAM, type);
	if ((m_Status.dwStatus&(STATUS_INCALL_PROGRESS|STATUS_INCALL)) || (type!=CT_MULTISTREAM && type!=CT_INTERCOM) ) {
		if (Conf && *Conf)
		{
			VS_Container rCnt;
			rCnt.AddValue(METHOD_PARAM, REJECT_METHOD);
			rCnt.AddValue(CONFERENCE_PARAM, Conf);
			if (User && *User)
				rCnt.AddValue(NAME_PARAM, User);
			rCnt.AddValueI32(CAUSE_PARAM, PARTISIPANT_IS_BUSY);
			rCnt.AddValue(USERNAME_PARAM, m_Status.MyInfo.UserName);
			ComposeSend(rCnt, CONFERENCE_SRV, ReturnBroker(Conf));
		}
	}
	else {
		bool NeedConnectInfo = false;		cnt.GetValue(NEEDCONNECTINFO_PARAM, NeedConnectInfo);
		size_t size = 0;
		const void * buff = cnt.GetBinValueRef(CLIENTCAPS_PARAM, size);
		m_Status.ConfInfo[1].Conference =	Conf;
		m_Status.ConfInfo[1].UserName =		User ? User : cnt.GetStrValueRef(SERVER_PARAM);
		m_Status.ConfInfo[1].Cname =		cnt.GetStrValueRef(NAME_PARAM);
		m_Status.ConfInfo[1].Cpass =		cnt.GetStrValueRef(PASSWORD_PARAM);
		m_Status.ConfInfo[1].DisplayName =  cnt.GetStrValueRef(DISPLAYNAME_PARAM);
		m_Status.ConfInfo[1].CallId =		cnt.GetStrValueRef(CALLID_PARAM);
		m_Status.ConfInfo[1].confType =		type==CT_MULTISTREAM ? 4 : 5;
		m_Status.ConfInfo[1].ConnectType =	NO_DIRECT_CONNECT;
		m_Status.ConfInfo[1].ClientCaps.Set(buff, size);
		if (NeedConnectInfo)
			SendUserConnectInfo(ReturnBroker(Conf), User);
		if ( (m_Status.ConfInfo[0].confType ==4 || m_Status.ConfInfo[0].confType ==5) &&
			m_Status.ConfInfo[0].confState == m_Status.ConfInfo[0].CONF_STATE_REQB &&
			!m_Status.ConfInfo[1].CallId.IsEmpty() && m_Status.ConfInfo[0].CallId.iCompare( m_Status.ConfInfo[1].CallId))
		{
			m_Status.ConfInfo[0].Clean();
			Accept();
		}
		else {
			m_Status.ConfInfo[1].invNum = m_InviteContainers.AddList(&cnt);
			SetIncallStatus(true, VSTRCL_CONF_INVMULTI, m_Status.ConfInfo[1].invNum);
		}
	}
	return dwRet;
}

/**
****************************************************************************
* Notify about receivers listen status in multiconf.
*
* \return ERR_OK
* \date    03.11.2005
******************************************************************************/
DWORD CVSTrClientProc::Method_ListenersFltr(VS_Container& cnt)
{
	const char *Conf = cnt.GetStrValueRef(CONFERENCE_PARAM);
	if (m_Status.CurrConfInfo->Conference==Conf) {
		int32_t fltr = 0; cnt.GetValue(MEDIAFILTR_PARAM, fltr);
		if (( fltr&VS_RcvFunc::FLTR_RCV_VIDEO) && !(m_Status.CurrConfInfo->lfltr&VS_RcvFunc::FLTR_RCV_VIDEO) ) {
			stream::Command cmd;
			cmd.RequestKeyFrame();
			g_cmdProc.AddCommand(cmd, false);
		}
		m_Status.CurrConfInfo->lfltr = fltr;
		if (fltr == 0) {
			if (m_podium_start_t != 0) m_podium_time += timeGetTime() - m_podium_start_t;
			m_podium_start_t = 0;
		} else {
			m_podium_start_t = timeGetTime();
		}
		DTRACE(VSTM_PRTCL, "Listeners fltr = %x", fltr);
	}
	return VSTRCL_CONF_OK;
}

/**
****************************************************************************
* Restrict bitrate in conference
*
* \return ERR_OK
* \date    17.01.2007
******************************************************************************/
DWORD CVSTrClientProc::Method_SendCommand(VS_Container& cnt)
{
	const char *Conf = cnt.GetStrValueRef(CONFERENCE_PARAM);
	if (m_Status.CurrConfInfo->Conference==Conf) {
		int32_t bitrate = 0;
		int64_t bitrate_svc = 0;
		bool key = false;
		if (cnt.GetValue(RESTRICTBITRATE_PARAM, bitrate)) {
			stream::Command cmd;
			cmd.RestrictBitrate(bitrate);
			cmd.sub_type = stream::Command::Info;
			g_cmdProc.AddCommand(cmd, false);
			DTRACE(VSTM_PRTCL, "Restrict bitrate to %d", bitrate);
		} else if (cnt.GetValue(RESTRICTBITRATESVC_PARAM, bitrate_svc)) {
			stream::Command cmd;
			cmd.RestrictBitrateSVC(bitrate_svc & 0x00000000ffffffff, bitrate_svc >> 32);
			cmd.sub_type = stream::Command::Info;
			g_cmdProc.AddCommand(cmd, false);
			DTRACE(VSTM_PRTCL, "Restrict SVC bitrate to %d (v = %d)", *reinterpret_cast<uint32_t*>(cmd.data), *(reinterpret_cast<uint32_t*>(cmd.data) + 1));
		} else if (cnt.GetValue(REQESTKEYFRAME_PARAM, key)) {
			stream::Command cmd;
			cmd.RequestKeyFrame();
			g_cmdProc.AddCommand(cmd, false);
		}
	}
	return VSTRCL_CONF_OK;
}

/**
****************************************************************************
* Role Events
*
* \return ERR_OK
* \date    17.01.2007
******************************************************************************/
DWORD CVSTrClientProc::Method_RoleEvent(VS_Container& cnt)
{
	const char *Conf = cnt.GetStrValueRef(CONFERENCE_PARAM);
	if (m_Status.CurrConfInfo->Conference==Conf) {
		int Key = m_RoleEventContainers.AddList(&cnt);

		int32_t	type(RET_NOTIFY); cnt.GetValue(TYPE_PARAM, type);
		if (type == RET_NOTIFY) {
			const char *user = cnt.GetStrValueRef(USERNAME_PARAM);
			if (user) {
				int32_t role(0); cnt.GetValue(ROLE_PARAM, role);
				auto ui = m_Status.CurrConfInfo->PartList.find(user);
				if (ui != m_Status.CurrConfInfo->PartList.end())
					ui->second.role = role;
			}
		}
		PostProc(VSTRCL_RCV_ROLE, Key);
	}
	return VSTRCL_CONF_OK;
}

DWORD CVSTrClientProc::Method_FECC(VS_Container& cnt)
{
	int dwMessId = m_FECC_Containers.AddList(&cnt);
	int32_t type = 0;	cnt.GetValue(TYPE_PARAM, type);
	DTRACE(VSTM_PRTCL, "FECC MessId=%d, from=%s to=%s type=%d", dwMessId, cnt.GetStrValueRef(FROM_PARAM), cnt.GetStrValueRef(TO_PARAM), type);
	PostProc(VSTRCL_CONF_FECC_CNT, dwMessId);
	return VSTRCL_CONF_OK;
}

/**
***************************************************************************
* Reseive device status
* \date    04.07.2007
*****************************************************************************/
DWORD CVSTrClientProc::Method_DeviceStatus(VS_Container& cnt)
{
	if (m_Status.dwStatus&STATUS_CONFERENCE) {
		const char * name = cnt.GetStrValueRef(USERNAME_PARAM);
		int32_t DeviceStatus = 1 | 1 << 16;
		cnt.GetValue(DEVICESTATUS_PARAM, DeviceStatus);
		if (m_Status.CurrConfInfo->confType==4 || m_Status.CurrConfInfo->confType==5) {
			if (name) {
				auto ui = m_Status.CurrConfInfo->PartList.find(name);
				if (ui != m_Status.CurrConfInfo->PartList.end())
					ui->second.device_status = DeviceStatus;
			}
			VSClientRcvDesc rd;
			rd.m_name = name;
			rd.m_dvs = DeviceStatus;
			rd.m_state = rd.RDSTATE_DVS;
			m_Status.CurrConfInfo->RsvList.AddAction(rd);
			PostProc(VSTRCL_RCV_UPD, m_Status.CurrConfInfo->RsvList.GetId());
		}
		else {
			NotifyDvs(name, DeviceStatus);
		}
	}
	return VSTRCL_CONF_OK;
}


DWORD CVSTrClientProc::Method_ConferenceSettings(VS_Container& cnt)
{
	if (m_Status.dwStatus&STATUS_CONFERENCE) {
		const char * conf = cnt.GetStrValueRef(CONFERENCE_PARAM);
		if (m_Status.ConfInfo[0].Conference == conf) {
			cnt.GetValueI32(UCALL_PARAM, m_Status.ConfInfo[0].UCall);
			cnt.GetValueI32(PVIEW_PARAM, m_Status.ConfInfo[0].PView);
		}
	}
	return VSTRCL_CONF_OK;
}


/**
***************************************************************************
* Reseive invite-to-multy reply
* \date    06.10.2007
*****************************************************************************/
DWORD CVSTrClientProc::Method_InviteReply(VS_Container& cnt)
{
	long result = 0;
	result = m_InviteContainers.AddList(&cnt);
	PostProc(VSTRCL_CONF_INVITEREPLY, result);
	return VSTRCL_CONF_OK;
}

/**
***************************************************************************
* Create container and notify GUI about Device Status
* \date    17.10.2007
*****************************************************************************/
void CVSTrClientProc::NotifyDvs(const char* user, long dvs) {
	DTRACE(VSTM_PRTCL, "Device status for %s is %x", user, dvs);
	VS_Container lcnt;
	lcnt.AddValue(METHOD_PARAM, DEVICESTATUS_METHOD);
	lcnt.AddValue(USERNAME_PARAM, user);
	lcnt.AddValueI32(DEVICESTATUS_PARAM, dvs);
	int Key = m_PresenceContainers.AddList(&lcnt);
	PostProc(VSTRCL_UPCONT_OK, Key);
}

/**
****************************************************************************
* Send Update Network Configuration Message.
*
* \return ERR_OK
* \date    18-11-2002
******************************************************************************/
DWORD CVSTrClientProc::UpdateConfiguration(bool tryAccept)
{
	if (tryAccept) {
		VS_UPnPInterface* unpn = VS_UPnPInterface::Instance();
		VS_UPnPInterface::accepts_t acceptTCP;
		VS_UPnPInterface::connects_t connectTCP;
		if (unpn->GetConnects(acceptTCP, connectTCP) && acceptTCP.size() > 0)
		{
			string_view MyUserName;
			if (!m_Status.MyInfo.UserName.IsEmpty())
				MyUserName = m_Status.MyInfo.UserName.m_str;
			net::endpoint::ClearAllAcceptTCP(MyUserName);
			for (const auto& x : acceptTCP)
				net::endpoint::AddAcceptTCP(*x, MyUserName);
			if (VS_EstablishAcceptPorts()>0) {
				m_Status.MyInfo.config.Clean();
				VS_GetEstablishedAcceptPorts(m_Status.MyInfo.config.connTCP);
				if (m_Status.MyInfo.config.connTCP.size() > 0 && connectTCP.size() > 0)
					for (auto& x : connectTCP)
						m_Status.MyInfo.config.connTCP.emplace_back(std::move(x));
				net::endpoint::ClearAllConnectTCP(MyUserName);
				for (const auto& x : m_Status.MyInfo.config.connTCP)
				{
					net::endpoint::AddConnectTCP(*x, MyUserName);
					DTRACE(VSTM_PRTCL, "Upnp Connects: %s:%d", x->host.c_str(), x->port);
				}
			}
		}
	}
	char direct_udp_host[256] = {0};
	unsigned short direct_udp_port(0);
	if(VS_StreamClient::UdpIsBinded())
	{
		// This variable sets the priority of address family (ipv4 or ipv6), that will be used in DirectUDP.
		// Values:
		// 1 - try to find ipv4 address, otherwise - use ipv6
		// 2 - try to find ipv6 address, otherwise - use ipv4
		// other values are equal to '1'.
		int ip_priority = 1;
		VS_RegistryKey key(true, REG_CurrentConfiguratuon, false, true);
		key.GetValue(&ip_priority, 4, VS_REG_INTEGER_VT, REG_DirectUDP_IPPriority);
		if(ip_priority != 2) ip_priority = 1; // Only '1' or '2' values are valid. '1' is default.
		if(ip_priority == 1)
		{
			// First add ipv4 connection, next - ipv6. So, we show to another client, that our priority is ipv4.
			// trying to add udp+ipv4 lisntener
			if(VS_StreamClient::GetBindedUdpAddressV4(direct_udp_host,256,direct_udp_port))
			{
				m_Status.MyInfo.config.connTCP.emplace_back(new net::endpoint::ConnectTCP{ direct_udp_host, direct_udp_port, net::endpoint::protocol_direct_udp });
				m_Status.MyInfo.config.connTCP.back()->is_ipv6 = false;
			}
			// trying to add udp+ipv6 lisntener
			if(VS_StreamClient::GetBindedUdpAddressV6(direct_udp_host,256,direct_udp_port))
			{
				m_Status.MyInfo.config.connTCP.emplace_back(new net::endpoint::ConnectTCP{ direct_udp_host, direct_udp_port, net::endpoint::protocol_direct_udp });
				m_Status.MyInfo.config.connTCP.back()->is_ipv6 = true;
			}
		}
		else // if ip_priority == 2
		{
			// First add ipv6 connection, next - ipv4. So, we show to another client, that our priority is ipv6.
			// trying to add udp+ipv6 lisntener
			if(VS_StreamClient::GetBindedUdpAddressV6(direct_udp_host,256,direct_udp_port))
			{
				m_Status.MyInfo.config.connTCP.emplace_back(new net::endpoint::ConnectTCP{ direct_udp_host, direct_udp_port, net::endpoint::protocol_direct_udp });
				m_Status.MyInfo.config.connTCP.back()->is_ipv6 = true;
			}
			// trying to add udp+ipv4 lisntener
			if(VS_StreamClient::GetBindedUdpAddressV4(direct_udp_host,256,direct_udp_port))
			{
				m_Status.MyInfo.config.connTCP.emplace_back(new net::endpoint::ConnectTCP{ direct_udp_host, direct_udp_port, net::endpoint::protocol_direct_udp });
				m_Status.MyInfo.config.connTCP.back()->is_ipv6 = false;
			}
		}
	}


	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, UPDATECONFIGURATION_METHOD);
	for (const auto& x : m_Status.MyInfo.config.connTCP)
	{
		auto data = x->Serialize();
		if (data.empty())
			continue;

		if (x->is_ipv6)
			rCnt.AddValue(IP6CONFIG_PARAM, static_cast<const void*>(data.data()), data.size());
		else
			rCnt.AddValue(IPCONFIG_PARAM, static_cast<const void*>(data.data()), data.size());
	}

	ComposeSend(rCnt, CONFIGURATION_SRV);
	return ERR_OK;
}


/**
****************************************************************************
* Request configuration On start aplication.
*
* \return ERR_OK
* \date    16-05-2003
******************************************************************************/
DWORD CVSTrClientProc::GetAppProperties()
{
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, GETAPPPROPERTIES_METHOD);
	rCnt.AddValue(CLIENTVERSION_PARAM, APP_version);
	rCnt.AddValue(NAME_PARAM, GUI_version);

	ComposeSend(rCnt, CONFIGURATION_SRV);
	return ERR_OK;
}

/**
****************************************************************************
* Set Application Prop on Server.
*
* \return ERR_OK
* \date    26-08-2003
******************************************************************************/
DWORD CVSTrClientProc::SetEpProperties(const char* name, const char* prop, bool sendImmediate)
{
	if (name && *name && prop) {
		// fill container
		m_SendPropContainer.AddValue(NAME_PARAM, name);
		m_SendPropContainer.AddValue(PROPERTY_PARAM, prop);
	}

	if (sendImmediate && m_SendPropContainer.IsValid()) {
		m_SendPropContainer.AddValue(METHOD_PARAM, SETPROPERTIES_METHOD);
		m_SendPropContainer.AddValue(HASH_PARAM, m_AppId);
		ComposeSend(m_SendPropContainer, CONFIGURATION_SRV);
		m_SendPropContainer.Clear();
	}
	return ERR_OK;
}


/**
****************************************************************************
* Network Update Configuration service's Messages interpreter.
*
* \param msg				- pointer to Window Message;
* \date    20-02-2003
******************************************************************************/
void CVSTrClientProc::UpdateConfigurationSrv(VS_ClientMessage &tMsg)
{
	DWORD dwRet = ERR_OK;

	switch(tMsg.Type())
	{
	case transport::MessageType::Invalid:
		dwRet = VSTRCL_ERR_INTERNAL;
		break;
	case transport::MessageType::Request:
	case transport::MessageType::Reply:
		{
			VS_Container cnt;
			if (cnt.Deserialize(tMsg.Body(), tMsg.BodySize())) {
				const char *Method = cnt.GetStrValueRef(METHOD_PARAM);
				DTRACE(VSTM_PRTCL, "method    = %20s", Method);
				if (Method && _stricmp(Method, CONFIGURATIONUPDATED_METHOD)==0) {
					m_SrvFlags = 0;
					cnt.GetValueI32(RESTRICTFLAGS_PARAM, m_SrvFlags);
					dwRet = VSTRCL_UPDCNFG_SRVFLAG;
				}
				else if (Method && _stricmp(Method, UPDATECONFIGURATION_METHOD)==0) {
					const void * EpConfig = 0;
					size_t size = 0;
					const char * ep = cnt.GetStrValueRef(NAME_PARAM);
					const char * service = cnt.GetStrValueRef(SERVICES_PARAM);
					EpConfig = cnt.GetBinValueRef(EPCONFIG_PARAM, size);
					if (ep && EpConfig && size > 0 && UpdateEndpointInfo(ep, EpConfig, size))
						m_ServerList.SetEvent(ep, VS_ServerList::SRVT_SERVERUPDATE, -1, service);
					dwRet = VSTRCL_UPDCNFG_OK;
				}
				else if (Method && _stricmp(Method, SETPROPERTIES_METHOD) == 0){
					if (cnt.CopyTo(m_PropContainer)) {
						dwRet = VSTRCL_UPDCNFG_SETPROP;
						if (!(m_Status.dwStatus&STATUS_CONFERENCE)) {
							SetEvent(m_hRestrictMedia);
						}
					} else {
						dwRet = VSTRCL_ERR_CONTEYNER;
					}

					const char *version = m_PropContainer.GetStrValueRef("server_protocol_version");
					if (version)
						m_SrvProtocolVersion = atoi(version);
				}
				else if (Method && _stricmp(Method, REDIRECTTO_METHOD)==0) {
					const char * ep = NULL;
					const void * EpConfig = nullptr;
					size_t size = 0;

					ep = cnt.GetStrValueRef(NAME_PARAM);
					EpConfig = cnt.GetBinValueRef(EPCONFIG_PARAM, size);
					if (ep && EpConfig && UpdateEndpointInfo(ep, EpConfig, size)) {
						if (ConnectServer((char*)ep)!=0)
							dwRet = VSTRCL_ERR_INTERNAL;
						else
							dwRet = ERR_WARN;
					}
				}
				else {
					dwRet = VSTRCL_ERR_METHOD;
				}
			}
			else 		dwRet = VSTRCL_ERR_CONTEYNER;
		}
		break;
	case transport::MessageType::Notify:
		dwRet = VSTRCL_ERR_TIMEOUT;
		break;
	default:						dwRet = VSTRCL_ERR_UNKMESS;		break;
	}
	PostProc(dwRet, m_SrvFlags);
}

/**
****************************************************************************
* Updates enpoint connects.
* \return true if all OK
* \date    27-10-2003
******************************************************************************/
bool CVSTrClientProc::UpdateEndpointInfo(const char *ep, const void *epConfig, unsigned long size)
{
	const unsigned epConfig_n_ctcp = net::endpoint::GetCountConnectTCPFromBuffer(epConfig, size);
	if (epConfig_n_ctcp == 0)
		return false;

	const unsigned n_ctcp = net::endpoint::GetCountConnectTCP(ep);
	if (n_ctcp > 0)
	{
		// existed ep
		auto tcp = net::endpoint::ReadConnectTCP(1, ep);
		unsigned num = ~0;
		for (unsigned i = 0; i < epConfig_n_ctcp; ++i) {
			net::endpoint::ConnectTCP epConfig_tcp;
			bool res = net::endpoint::GetFromBuffer(i, epConfig_tcp, epConfig, size);
			if (res	&& !tcp->host.empty() && !epConfig_tcp.host.empty() &&
				strcasecmp(tcp->host.c_str(), epConfig_tcp.host.c_str()) == 0 && tcp->port == epConfig_tcp.port) {
				num = i;
				break;
			}
		}
		net::endpoint::Remove(ep);
		net::endpoint::Deserialize(true, epConfig, size, ep);
		if (num != ~0)
			net::endpoint::MakeFirstConnectTCP(num + 1, ep); //found in update, make it first
		else
		{
			// current first connect not founf, add it to cooncts only for current online broker
			if ((m_Status.dwStatus&STATUS_SERVAVAIL) && m_CurrBroker==ep)
				net::endpoint::AddFirstConnectTCP(*tcp, ep);
		}
	}
	else
	{
		net::endpoint::Remove(ep);
		net::endpoint::Deserialize(true, epConfig, size, ep);
	}
	m_Proxy.UpdateEnpointsProtocol(ep);
	return true;
}


/**
****************************************************************************
* Send Chat Message.
*
* \return ERR_OK
* \date    03-02-2003
******************************************************************************/
DWORD CVSTrClientProc::ChatSend(const char *Message, const char *To)
{
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, SENDMESSAGE_METHOD);
	rCnt.AddValue(FROM_PARAM, m_Status.MyInfo.UserName);
	rCnt.AddValue(DISPLAYNAME_PARAM, m_Status.MyInfo.DisplayName);
	rCnt.AddValue(MESSAGE_PARAM, Message);

	if (To) {
		rCnt.AddValue(TO_PARAM, To);
		ComposeSend(rCnt, CHAT_SRV, 0, m_SrvProtocolVersion <= 3 ? To : 0);
	}
	else if (m_Status.CurrConfInfo->Conference) {
		rCnt.AddValue(CONFERENCE_PARAM, m_Status.CurrConfInfo->Conference);
		ComposeSend(rCnt, CHAT_SRV, m_SrvProtocolVersion <= 3 ? ReturnBroker(m_Status.CurrConfInfo->Conference) : 0);
	}

	return ERR_OK;
}

/**
****************************************************************************
* Send Command Message.
*
* \return ERR_OK
* \date    01-09-2003
******************************************************************************/
DWORD CVSTrClientProc::SendCommand(const char *Command, const char *To)
{
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, SENDCOMMAND_METHOD);
	rCnt.AddValue(FROM_PARAM, m_Status.MyInfo.UserName);
	rCnt.AddValue(MESSAGE_PARAM, Command);

	if (To) {
		rCnt.AddValue(TO_PARAM, To);
		ComposeSend(rCnt, CHAT_SRV, 0, To);
	}
	else if (m_Status.CurrConfInfo->Conference) {
		rCnt.AddValue(CONFERENCE_PARAM, m_Status.CurrConfInfo->Conference);
		ComposeSend(rCnt, CHAT_SRV, ReturnBroker(m_Status.CurrConfInfo->Conference));
	}
	return ERR_OK;
}


/**
****************************************************************************
* Chat service's Messages interpreter.
*
* \param msg				- pointer to Window Message;
* \date    18-11-2002
******************************************************************************/
void CVSTrClientProc::ChatSrv(VS_ClientMessage &tMsg)
{
	VS_AutoLock lock(this);

	DWORD dwRet = ERR_OK;
	DWORD dwMessId = 0;

	switch(tMsg.Type())
	{
	case transport::MessageType::Invalid:
		dwRet = VSTRCL_ERR_INTERNAL;
		break;
	case transport::MessageType::Request:
	case transport::MessageType::Reply:
		{
			VS_Container cnt;
			if (cnt.Deserialize(tMsg.Body(), tMsg.BodySize())) {
				const char *Method = cnt.GetStrValueRef(METHOD_PARAM);
				if (Method && _stricmp(Method, SENDCOMMAND_METHOD) == 0) {
					dwMessId = m_CommandContainers.AddList(&cnt);
					m_Status.dwStatus|=STATUS_COMMAND;
					dwRet = VSTRCL_CHAT_COMMAND;
					DTRACE(VSTM_PRTCL, "CMD = %s", cnt.GetStrValueRef(MESSAGE_PARAM));
				}
				else if (Method && _stricmp(Method, SENDMESSAGE_METHOD) == 0) {
					const char *From = cnt.GetStrValueRef(FROM_PARAM);
					dwMessId = m_MessContainers.AddList(&cnt);
					m_Status.dwStatus|=STATUS_MESSAGE;
					dwRet = VSTRCL_CHAT_OK;
				}
			}
			else 	dwRet = VSTRCL_ERR_CONTEYNER;
		}
		break;
	case transport::MessageType::Notify:
		dwRet = VSTRCL_ERR_TIMEOUT;
		break;
	default: 						dwRet = VSTRCL_ERR_UNKMESS;		break;
	}
	PostProc(dwRet, dwMessId);
}

void CVSTrClientProc::TorrentSrv(VS_ClientMessage &tMsg) {
	VS_Container cnt;
	if (!cnt.Deserialize(tMsg.Body(), tMsg.BodySize())) return;
	const char *Method = cnt.GetStrValueRef(METHOD_PARAM);

    if (Method && _stricmp(Method, SHARE_METHOD) == 0) {
        Share_Method(cnt);
    }
}

void CVSTrClientProc::TrInit(int flags) {
	m_torrent = VS_FileTransfer::MakeFileTransfer(nullptr, ~0u, flags, weak_from_this());
}

void CVSTrClientProc::TrSendFile(const char *filePath, const std::vector<std::string> &user_id_list, std::string& id) {
    _TrSendFile(filePath, user_id_list, NULL, id);
}

void CVSTrClientProc::TrSendFile(const char *filePath, const char *conf_id, std::string &id) {
    std::vector<std::string> empty_id_list(0);
    _TrSendFile(filePath, empty_id_list, conf_id, id);
}

void CVSTrClientProc::_TrSendFile(const char *filePath, const std::vector<std::string> &user_id_list, const char *conf_id, std::string& id) {
    VS_AutoLock lock(this);
    id = TrGenId(filePath, user_id_list, conf_id);
    if (m_items.find(id) != m_items.end()) {
        return;
    }

    if (m_trackers.empty() || !filePath) {
        // 'trackers' property was not received or
        // empty path was passed
        return;
    }

    std::string file_name = filePath, dir;
    if (file_name[file_name.length() - 1] == '/'
        || file_name[file_name.length() - 1] == '\\') {
        file_name.erase(file_name.length() - 1, file_name.length());
    }
    const size_t last_slash_idx = file_name.find_last_of("\\/");
    if (std::string::npos != last_slash_idx) {
        dir = file_name.substr(0, last_slash_idx);
        file_name.erase(0, last_slash_idx + 1);
    }
	m_items[id] = { file_name, filePath, dir, "", "", user_id_list, conf_id ? conf_id : "", { undefined_error } };
    m_torrent->SendFile(filePath, id, m_trackers);
    // TrOnReadyToSend will be called
}

void CVSTrClientProc::TrDownloadFile(const char *magnet, const char *distPath, std::string &id) {
    UploadDownload dwn;
    dwn.stat.status = undefined_error;
    dwn.magnet = magnet;
    dwn.path = distPath;
    dwn.dir = distPath;

    id = TrGenId(magnet, std::vector<std::string>(), NULL);
    m_items[id] = dwn;
    m_torrent->StartDownload(magnet, id, distPath);
    // Connect via server's address, proxy will redirect connection to torrent peer
    if (!m_server_ip.empty()) {
		VS_FileTransfer::Endpoint peer_addr{ m_server_ip, m_server_port };
        m_torrent->GetControl(VS_FileTransfer::hash_str_from_uri(magnet)).Connect(peer_addr);
    }
    // TrOnReadyToDownload will be called
}

void CVSTrClientProc::TrSetLimits(unsigned int dw_lim, unsigned int up_lim) {
    m_dw_lim = m_up_lim = 5 * 1024 * 1024;

    unsigned int srv_up_lim = 0, srv_dw_lim = 0;

    char buf[256]{};
    if (GetProperties("torrent_upstream_limit", buf) == ERR_OK) {
        srv_up_lim = atoi(buf);
    }
    if (GetProperties("torrent_downstream_limit", buf) == ERR_OK) {
        srv_dw_lim = atoi(buf);
    }
    // Set to non-zero minimum
    dw_lim = (srv_dw_lim && srv_dw_lim < dw_lim) ? srv_dw_lim : dw_lim;
    up_lim = (srv_up_lim && srv_up_lim < up_lim) ? srv_up_lim : up_lim;
    m_dw_lim = (dw_lim && dw_lim < m_dw_lim) ? dw_lim : m_dw_lim;
    m_up_lim = (up_lim && up_lim < m_up_lim) ? up_lim : m_up_lim;

    m_torrent->SetLimits(m_dw_lim, m_up_lim);
}

std::string CVSTrClientProc::TrGenId(const char *filePath, const std::vector<std::string> &user_id_list, const char *conf_id) {
    time_t stamp = time(0);
    MD5 md5;
    md5.Update(&stamp, sizeof(time_t));
    md5.Final();
    char digest_str[33];
    md5.GetString(digest_str, false);
    return digest_str;
}

bool CVSTrClientProc::TrUpdateStatus() {
	std::vector<std::pair<std::string, TrStatus>> update_items;
	int num_active = 0;

	{
		VS_AutoLock lock(this);
		for (std::map<std::string, UploadDownload>::iterator i = m_items.begin(); i != m_items.end(); i++) {
			if (i->second.stat.status != download_complete) {
				TrStatus s = TrGetStatus(i->second.magnet);
				if (s != i->second.stat.status) {
					// Status changed
					i->second.stat.status = s;
					// calling pFileSendStartedCB while holding lock might block
					// on GUI's mutexes, so put it in separate list for later call
					update_items.emplace_back(i->first, s);
				}
				num_active++;

				if (!m_server_ip.empty()) {
					VS_FileTransfer::Endpoint peer_addr{ m_server_ip, m_server_port };
					m_torrent->GetControl(VS_FileTransfer::hash_str_from_uri(i->second.magnet)).Connect(peer_addr);
				}
			}
		}
	}

	if (pFileSendStartedCB) {
		for (unsigned int i = 0; i < update_items.size(); i++) {
			(*pFileDownloadStatusCB)(update_items[i].first.c_str(), update_items[i].second, NULL);
		}
	}

	if (num_active) {
        SetTimerObject(TIME_TR_UPDATE_STATUS, 1000);
    }
	return num_active != 0;
}

CVSTrClientProc::TrStatus CVSTrClientProc::TrGetStatus(const std::string &magnet) {
	VS_FileTransfer::Info info;
	m_torrent->GetControl(VS_FileTransfer::hash_str_from_uri(magnet)).GetStatistics(info, false);
	if (info.lastError.empty() && !info.name.empty()) {
		if (info.isFinished) {
			return download_complete;
		} else {
			return download_started;
		}
	} else {
		return undefined_error;
	}
}

void CVSTrClientProc::TrGetStat(const std::string &magnet, TrStat &stat) {
    VS_FileTransfer::Info info;
    m_torrent->GetControl(VS_FileTransfer::hash_str_from_uri(magnet)).GetStatistics(info, false);
    if (info.lastError.empty() && !info.name.empty()) {
		stat.size = info.totalWanted;
		stat.peers = info.numPeers;
        if (info.isFinished) {
            stat.status = download_complete;
			stat.downloaded = stat.size;
			stat.uploaded = m_torrent->GetServerProgressBytes(info.infoHash);
			stat.d_speed = 0;
			stat.u_speed = info.uploadRate;
        } else {
            stat.status = download_started;
			stat.downloaded = info.totalWantedDone;
			stat.uploaded = info.totalPayloadUpload;
			stat.d_speed = info.downloadRate;
			stat.u_speed = info.uploadRate;
        }
    } else {
        stat = TrStat(undefined_error);
    }
}

// called in torrent thread
bool CVSTrClientProc::TrOnAskUser(const std::string &id) {
    std::string dist_path;
    TrStatus s = download_started;
    {
        VS_AutoLock lock(this);
        dist_path = m_items[id].path;
    }

    {
        VS_AutoLock lock(this);
        if (s != download_started) {
            m_items.erase(id);
            return false;
        }
        m_items[id].stat.status = download_started;
        // Start updating download status
        SetTimerObject(TIME_TR_UPDATE_STATUS, 500);
    }
	return true;
}

// called in torrent thread
void CVSTrClientProc::TrOnReadyToSend(const std::string &magnet, const std::string &id) {
	if (magnet.empty()) {
		if (pFileSendStartedCB) {
			(*pFileSendStartedCB)(id.c_str(), false, magnet.c_str(), NULL);
		}
		return;
	}

    VS_AutoLock lock(this);
    UploadDownload &up = m_items[id];
    up.magnet = magnet;
    up.stat.status = download_complete;

    VS_Container cnt;
    cnt.AddValue(METHOD_PARAM, SHARE_METHOD);
    cnt.AddValue(MAGNET_LINK_PARAM, up.magnet);
    cnt.AddValue(FILENAME_PARAM, up.name);

    cnt.AddValue(ID_PARAM, id);
    if (!ComposeSend(cnt, TORRENT_SRV)) {
        if (pFileSendStartedCB) {
            (*pFileSendStartedCB)(id.c_str(), false, magnet.c_str(), NULL);
        }
    } else {
        // Connect via server's address, proxy will redirect connection to torrent peer
        if (!m_server_ip.empty()) {
			VS_FileTransfer::Endpoint peer_addr{ m_server_ip, m_server_port };
            m_torrent->GetControl(VS_FileTransfer::hash_str_from_uri(magnet)).Connect(peer_addr);
        }
        // Start updating upload status
        SetTimerObject(TIME_TR_UPDATE_STATUS, 500);
    }
}

bool CVSTrClientProc::Share_Method(VS_Container &cnt) {
    int32_t result;
    cnt.GetValue(RESULT_PARAM, result);
    const char *id = cnt.GetStrValueRef(ID_PARAM);
    const char *magnet = cnt.GetStrValueRef(MAGNET_LINK_PARAM);
    const char *url = cnt.GetStrValueRef(URL_PARAM);
    if ((result != TR_NO_ERROR && result != TR_P2P_ONLY) || !id) {
        if (pFileSendStartedCB) {
            (*pFileSendStartedCB)(id, false, magnet, NULL);
        }
        return true;
    }

    {
        VS_AutoLock lock(this);
        UploadDownload &up = m_items[id];
        if (url) {
            up.url = url;
        }
        TrSendUploadMsg(up);
    }
    if (pFileSendStartedCB) {
        (*pFileSendStartedCB)(id, true, magnet, url);
    }
    return true;
}

void CVSTrClientProc::TrSendUploadMsg(const UploadDownload &up) {
    std::string message;
    if (!up.url.empty()) {
        message += "<a href=\"" + up.url + "\">" + up.name + "</a>";
    } else {
        message += "<a href=\"\">" + up.name + "</a>";
    }

    VS_Container cnt;
    cnt.AddValue(METHOD_PARAM, SENDMESSAGE_METHOD);
    cnt.AddValue(FROM_PARAM, m_Status.MyInfo.UserName);
    cnt.AddValue(DISPLAYNAME_PARAM, m_Status.MyInfo.DisplayName);
    cnt.AddValue(MESSAGE_PARAM, message);
    cnt.AddValue(TYPE_PARAM, "application/x-bittorrent");
    cnt.AddValue(NAME_PARAM, up.name);
    cnt.AddValue(LINK_PARAM, up.magnet);
    cnt.AddValue(URL_PARAM, up.url);
    cnt.AddValue(ABOUT_PARAM, "---");

    union VS_FILETIME {
        struct _FILETIME winftime;
        uint_fast64_t time;
    };
    VS_FILETIME _send_time;
    _send_time.time = static_cast<uint_fast64_t> (_time64(0));
    const int_fast64_t tconv = (static_cast<int_fast64_t> (_send_time.time)* static_cast<int_fast64_t> (10000000)) + static_cast<int_fast64_t> (0x019DB1DED53E8000LL);
    _send_time.time = static_cast<uint_fast64_t> (tconv);

    cnt.AddValue(FILETIME_PARAM, &_send_time, sizeof(VS_FILETIME));

    if (up.user_id_list.empty()) {
        if (up.conf_id.empty()) {
            cnt.AddValue(CONFERENCE_PARAM, m_Status.CurrConfInfo->Conference);
            ComposeSend(cnt, CHAT_SRV, m_SrvProtocolVersion <= 3 ? ReturnBroker(m_Status.CurrConfInfo->Conference) : 0);
        } else {
            cnt.AddValue(CONFERENCE_PARAM, up.conf_id);
            ComposeSend(cnt, CHAT_SRV, m_SrvProtocolVersion <= 3 ? ReturnBroker(up.conf_id.c_str()) : 0);
        }
    } else {
        for (size_t i = 0; i < up.user_id_list.size(); i++) {
            VS_Container _cnt = cnt;
            _cnt.AddValue(TO_PARAM, up.user_id_list[i]);
			ComposeSend(_cnt, CHAT_SRV, 0, m_SrvProtocolVersion <= 3 ? up.user_id_list[i].c_str() : 0);
        }
    }
}


/**
****************************************************************************
* Send message to other client that direct connection test successful
*
* \return ERR_OK
* \date    18-11-2002
******************************************************************************/
DWORD CVSTrClientProc::ClientTryAcceptDirect(const char* conference, const char* endpoint)
{
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, TRYACCEPTDIRECT_METHOD);
	rCnt.AddValue(NAME_PARAM, conference);

	ComposeSend(rCnt, CLIENT_SRV, 0, endpoint, TESTDACCEPT_TIMEOUT);
	return ERR_OK;
}

/**
****************************************************************************
* Other client service's Messages interpreter.
*
* \param msg				- pointer to Window Message;
* \date    25-02-2003
******************************************************************************/
void CVSTrClientProc::ClientSrv(VS_ClientMessage &tMsg)
{
	DWORD dwRet = ERR_OK;

	switch(tMsg.Type())
	{
	case transport::MessageType::Invalid:
		dwRet = VSTRCL_ERR_INTERNAL;
		break;
	case transport::MessageType::Reply:
		dwRet = VSTRCL_CONF_OK;
		break;
	case transport::MessageType::Request:
		{
			VS_Container cnt;
			if (cnt.Deserialize(tMsg.Body(), tMsg.BodySize())) {
				const char *Method;
				Method = cnt.GetStrValueRef(METHOD_PARAM);
				if (Method && _stricmp(Method, TRYACCEPTDIRECT_METHOD) == 0) {
					if (m_Status.ConfInfo[1].Conference==cnt.GetStrValueRef(NAME_PARAM)) {
						if (m_Status.ConfInfo[1].ConnectType==DIRECT_UNKNOWN && (m_Status.dwStatus&STATUS_INCALL_PROGRESS)) {
							m_Status.ConfInfo[1].ConnectType = DIRECT_ACCEPT;
							RemoveTimerObjects(TIME_TSTACC);
							DTRACE(VSTM_PRTCL, "Direct Accept");
							SetIncallStatus(true, VSTRCL_ACCEPT_OK, m_Status.ConfInfo[1].invNum);
							dwRet = VSTRCL_ACCEPT_OK;
						}
						else dwRet = VSTRCL_ACCEPT_EXP;
					}
					else dwRet = VSTRCL_ACCEPT_EXP;
				}
				else 	dwRet = VSTRCL_ERR_METHOD;
			}
			else 		dwRet = VSTRCL_ERR_CONTEYNER;
		}
		break;
	case transport::MessageType::Notify:
		dwRet = VSTRCL_ACCEPT_EXP;
		break; /// nothing to do...
	default:						dwRet = VSTRCL_ERR_UNKMESS;		break;
	}
	PostProc(dwRet);
}


/**
****************************************************************************
* request status for all users in AB_COMMON
* \return message sequential number
* \date    27-05-2003
******************************************************************************/
DWORD CVSTrClientProc::GetAllUserStatus(){
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, GETALLUSERSTATUS_METHOD);
	return ComposeSend(rCnt, PRESENCE_SRV);
}


/**
****************************************************************************
* add user to address book
* \param	user		- wanted user callid
* \param	addressBook	- wanted adress book
* \param	email		- (optional) wanted user callid
* \return	message sequential number
* \date    27-05-2003
******************************************************************************/
DWORD CVSTrClientProc::AddUserToAddressBook(char* user, long addressBook, char* email){
	if (!user && !email)  return 0;
	DTRACE(VSTM_PRTCL, "request = AddUserToAddressBook, user %s", user ? user : email);
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, ADDTOADDRESSBOOK_METHOD);
	rCnt.AddValueI32(ADDRESSBOOK_PARAM, addressBook);
	if (user) rCnt.AddValue(NAME_PARAM, user);
	if (email) rCnt.AddValue(CALLID_PARAM, email);

	return ComposeSend(rCnt, ADDRESSBOOK_SRV);
}

DWORD CVSTrClientProc::BanUser(char *user, long abuse)
{
	if (!user)  return 0;
	DTRACE(VSTM_PRTCL, "request = BanUser, user %s", user);
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, ADDTOADDRESSBOOK_METHOD);
	rCnt.AddValueI32(ADDRESSBOOK_PARAM, AB_BAN_LIST);
	rCnt.AddValue(NAME_PARAM, user);
	rCnt.AddValueI32(TYPE_PARAM, abuse);

	return ComposeSend(rCnt, ADDRESSBOOK_SRV);
}


/**
****************************************************************************
* Remove user from address book
* \param	user		- wanted user callid
* \param	addressBook	- wanted adress book
* \return	message sequential number
* \date    27-05-2003
******************************************************************************/
DWORD CVSTrClientProc::RemoveFromAddressBook(char* user, long addressBook){
	if (!user)  return 0;
	DTRACE(VSTM_PRTCL, "request = RemoveFromAddressBook, user %s", user);
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, REMOVEFROMADDRESSBOOK_METHOD);
	rCnt.AddValue(NAME_PARAM, user);
	rCnt.AddValueI32(ADDRESSBOOK_PARAM, addressBook);

	return ComposeSend(rCnt, ADDRESSBOOK_SRV);
}

/**
****************************************************************************
* Search in address book, or immediately return notifay to update from GUI
* \param	query		- SQL query formed in GUI
* \param	addressBook	- wanted adress book
* \return	message sequential number
* \date    27-05-2003
******************************************************************************/
DWORD CVSTrClientProc::SearchAddressBook(const char *query, long addressBook, long hash){
	DTRACE(VSTM_PRTCL, "request = SearchAddressBook, query = '%s', ab=%d, hash=%d", query, addressBook, hash);
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, SEARCHADDRESSBOOK_METHOD);
	rCnt.AddValue(QUERY_PARAM, query);
	rCnt.AddValueI32(HASH_PARAM, hash);
	rCnt.AddValueI32(ADDRESSBOOK_PARAM, addressBook);

	return ComposeSend(rCnt, ADDRESSBOOK_SRV);
}


/**
****************************************************************************
* Get one user status
* \param	user		- wanted user callid
* \return	message sequential number
* \date    27-05-2003
******************************************************************************/
DWORD CVSTrClientProc::GetUserStatus(char* CallId)
{
	DTRACE(VSTM_PRTCL, "request = GetUserStatus of %s", CallId);

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, GETUSERSTATUS_METHOD);
	rCnt.AddValue(CALLID_PARAM, CallId);

	return ComposeSend(rCnt, PRESENCE_SRV);
}

/**
****************************************************************************
* Not Implemented now, code gone to GUI
* \param	user		- wanted user callid
* \return	-1
* \date    27-05-2003
******************************************************************************/
DWORD CVSTrClientProc::GetUserPhoto(char* user, void **photo, unsigned long *photoSize, char**Ptype)
{
	if (!user) return -1;
	DTRACE(VSTM_PRTCL, "request = GetUserPhoto of %s ", user);
	return -1;
}

/**
****************************************************************************
* Not Implemented now, code gone to GUI
* \param	user		- wanted user callid
* \return	-1
* \date    27-05-2003
******************************************************************************/
DWORD CVSTrClientProc::GetUserInformation(char* szUID, char* fs[16])
{
	if ( !szUID) return -1;
	DTRACE(VSTM_PRTCL, "request = GetUserInformation of %s", szUID);
	return -1;
}


void CVSTrClientProc::MonitorStatus()
{
	VS_UserPresence_Status status = USER_STATUS_UNDEF;
	if (m_Status.dwStatus&STATUS_CONFERENCE) {
		status =  USER_BUSY;
		if (m_Status.CurrConfInfo->confType==2) {
			if (m_Status.CurrConfInfo->UserName==m_Status.MyInfo.UserName)
				status = USER_PUBLIC;
		}
		else if (m_Status.CurrConfInfo->confType==4 || m_Status.CurrConfInfo->confType==5)
			if (m_Status.CurrConfInfo->UserName==m_Status.MyInfo.UserName)
				status = USER_MULTIHOST;
	}
	else if (m_Status.dwStatus&STATUS_LOGGEDIN)
		status = USER_AVAIL;
	else if (m_Status.dwStatus&STATUS_SERVAVAIL)
		status = USER_LOGOFF;
	else
		status = USER_STATUS_UNDEF;
	if (status!= m_StoredStatus) {
		if (status!= USER_STATUS_UNDEF)
			UpdateStatus(status);
		m_StoredStatus = status;
	}
	if (!(m_Status.dwStatus&STATUS_LOGGEDIN))
		m_UserProp.Clear();						// clear user props on logoff
}
VS_UserPresence_Status CVSTrClientProc::GetCurrUserPresenceStatus()
{
	VS_UserPresence_Status status = USER_STATUS_UNDEF;
	if (m_Status.dwStatus&STATUS_CONFERENCE) {
		status =  USER_BUSY;
		if (m_Status.CurrConfInfo->confType==2) {
			if (m_Status.CurrConfInfo->UserName==m_Status.MyInfo.UserName)
				status = USER_PUBLIC;
		}
		else if (m_Status.CurrConfInfo->confType==4 || m_Status.CurrConfInfo->confType==5)
			if (m_Status.CurrConfInfo->UserName==m_Status.MyInfo.UserName)
				status = USER_MULTIHOST;
	}
	else if (m_Status.dwStatus&STATUS_LOGGEDIN)
		status = USER_AVAIL;
	else if (m_Status.dwStatus&STATUS_SERVAVAIL)
		status = USER_LOGOFF;
	else
		status = USER_STATUS_UNDEF;
	return status;
}

void CVSTrClientProc::UpdateStatus(VS_UserPresence_Status status/*, const char * ext*/)
{
	DTRACE(VSTM_PRTCL, "Update My Status: %d ", status);
	if (!(m_Status.dwStatus&STATUS_LOGGEDIN))
		return;

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, UPDATESTATUS_METHOD);
	rCnt.AddValueI32(USERPRESSTATUS_PARAM, status);
	///if (ext) rCnt.AddValue(USEREXTSTATUS_PARAM, ext);

	ComposeSend(rCnt, PRESENCE_SRV);
}
void CVSTrClientProc::UpdateExtStatus(const char *name, const int value)
{
	/**
		create status, get container, add to status collection(?) and send to server;
	**/
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM,UPDATESTATUS_METHOD);
	cnt.AddValueI32(USERPRESSTATUS_PARAM, GetCurrUserPresenceStatus());
	VS_Container extStatus;
	if (name)
		extStatus.AddValueI32(name, value);
	if (cnt.AddValue(EXTSTATUS_PARAM, extStatus))
		ComposeSend(cnt,PRESENCE_SRV);
	if(!!m_Status.MyInfo.UserName)
		m_ext_statuses[(const char*)m_Status.MyInfo.UserName].UpdateStatus(extStatus);
}
void CVSTrClientProc::UpdateExtStatus(const char *name, const wchar_t *value)
{
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM,UPDATESTATUS_METHOD);
	cnt.AddValueI32(USERPRESSTATUS_PARAM, GetCurrUserPresenceStatus());
	VS_Container extStatus;
	if (name)
		extStatus.AddValue(name, value);
	if (cnt.AddValue(EXTSTATUS_PARAM, extStatus))
		ComposeSend(cnt,PRESENCE_SRV);
	if(!!m_Status.MyInfo.UserName)
		m_ext_statuses[(const char*)m_Status.MyInfo.UserName].UpdateStatus(extStatus);
}
void CVSTrClientProc::UpdateExtStatus(const int fwd_type, const wchar_t *fwd_call_id, const int fwd_timeout, const wchar_t *timeout_call_id)
{
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM,UPDATESTATUS_METHOD);
	cnt.AddValueI32(USERPRESSTATUS_PARAM, GetCurrUserPresenceStatus());
	VS_Container extStatus;
	extStatus.AddValueI32(EXTSTATUS_NAME_FWD_TYPE, fwd_type);
	extStatus.AddValue(EXTSTATUS_NAME_FWD_CALL_ID,fwd_call_id);
	extStatus.AddValueI32(EXTSTATUS_NAME_FWD_TIMEOUT, fwd_timeout);
	extStatus.AddValue(EXTSTATUS_NAME_FWD_TIMEOUT_CALL_ID,timeout_call_id);
	if (cnt.AddValue(EXTSTATUS_PARAM, extStatus))
		ComposeSend(cnt,PRESENCE_SRV);
	if(!!m_Status.MyInfo.UserName)
		m_ext_statuses[(const char*)m_Status.MyInfo.UserName].UpdateStatus(extStatus);
}
void CVSTrClientProc::Subscribe(char** user, int num, bool plus)
{
	if (!user || !*user || !num)
		return;
	const char * method = plus ? SUBSCRIBE_METHOD : UNSUBSCRIBE_METHOD;
	DTRACE(VSTM_PRTCL, "%s of: %s..., num=%d", method, user[0], num);

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, method);
	if (m_CleanOnSubscribe && plus) {
		rCnt.AddValueI32(CAUSE_PARAM, 1);
		m_CleanOnSubscribe = false;
	}
	for (int i = 0; i<num; i++)
		rCnt.AddValue(CALLID_PARAM, user[i]);

	ComposeSend(rCnt, PRESENCE_SRV);
}


/**
****************************************************************************
* UserPresence service's Messages interpreter.
*
* \param msg				- pointer to Window Message;
* \date    18-11-2002
******************************************************************************/
void CVSTrClientProc::UserPresenceSrv(VS_ClientMessage &tMsg)
{
	VS_AutoLock lock(this);
	DWORD dwRet = VSTRCL_USERS_NORET;
	DWORD dwMessId = 0;

	switch(tMsg.Type())
	{
	case transport::MessageType::Invalid:
		dwRet = VSTRCL_ERR_INTERNAL;
		break;
	case transport::MessageType::Request:
	case transport::MessageType::Reply:
		{
			VS_Container cnt;
			if (cnt.Deserialize(tMsg.Body(), tMsg.BodySize())) {
				VS_SimpleStr Method(cnt.GetStrValueRef(METHOD_PARAM));
				DTRACE(VSTM_PRTCL, "method    = %20s", Method.m_str);
				if (Method == GETALLUSERSTATUS_METHOD) {
					int32_t seq_id = -1;
					cnt.GetValue(SEQUENCE_ID_PARAM, seq_id);
					m_OnlineUsers.TrackSequence(seq_id, true);
					PostProc(VSTRCL_UPCONT_RSTATUS);
				}
				else if (Method == UPDATESTATUS_METHOD) {
					int32_t seq_id = -1;
					cnt.GetValue(SEQUENCE_ID_PARAM,seq_id);
					int32_t cause = 0;
					cnt.GetValue(CAUSE_PARAM,cause);
					if(cause==1)
						PostProc(VSTRCL_UPCONT_RSTATUS);
					if(m_OnlineUsers.TrackSequence(seq_id,cause==1))
						GetAllUserStatus();

					GetExtendedStatuses(cnt);
				}
				else if ( Method == SENDPARTSLIST_METHOD) {
					const char* Conf = cnt.GetStrValueRef(CONFERENCE_PARAM);
					VS_ParticipantListType type = PLT_OLD; cnt.GetValueI32(TYPE_PARAM, type);
					const char* User = nullptr;
					decltype(m_Status.CurrConfInfo->PartList) &PartList = m_Status.CurrConfInfo->PartList;
					size_t old_num = PartList.size();

					if (m_Status.CurrConfInfo->Conference == Conf) {
						int32_t seq_id = -1;
						cnt.GetValue(SEQUENCE_ID_PARAM, seq_id);
						bool req = m_PartsTrack.TrackSequence(seq_id, type == PLT_OLD || type == PLT_ALL);
						if (req)
							GetAllPartList(Conf);

						bool fill = false;
						if (type == PLT_OLD || type == PLT_ALL) {
							PartList.clear();
							fill = true;
						}
						else if (type == PLT_ADD || type == PLT_UPD) {
							fill = true;
							User = cnt.GetStrValueRef(USERNAME_PARAM);
						}
						else if (type == PLT_DEL) {
							User = cnt.GetStrValueRef(USERNAME_PARAM);
							PartList.erase(User);
						}

						cnt.Reset();
						decltype(m_Status.CurrConfInfo->PartList)::iterator it = PartList.end();

						if (fill)
							while (cnt.Next()) {
								if (strcasecmp(cnt.GetName(), USERNAME_PARAM) == 0) {
									VS_SimpleStr user(cnt.GetStrValueRef());
									it = type == PLT_UPD ? PartList.find(user) : PartList.end();
									if (it == PartList.end())
										it = PartList.emplace(user, CUserDesk::PartInfo()).first;
								}
								else if (it == PartList.end()) {
									continue;
								}
								else if (strcasecmp(cnt.GetName(), DISPLAYNAME_PARAM) == 0) {
									it->second.dn = cnt.GetStrValueRef();
								}
								else if (strcasecmp(cnt.GetName(), ROLE_PARAM) == 0) {
									cnt.GetValueI32(it->second.role);
									//it->second.role &= 0xff;
								}
								else if (strcasecmp(cnt.GetName(), DEVICESTATUS_PARAM) == 0) {
									cnt.GetValueI32(it->second.device_status);
								}
								else if (strcasecmp(cnt.GetName(), IS_OPERATOR_PARAM) == 0) {
									cnt.GetValue(it->second.is_operator);
								}
							}
						if (type == PLT_ADD || old_num < PartList.size()) {
							stream::Command cmd;
							cmd.RequestKeyFrame();
							g_cmdProc.AddCommand(cmd, false);
						}

						DTRACE(VSTM_PRTCL, "SENDPARTSLIST parts = %d, type = %d", m_Status.CurrConfInfo->PartList.size(), type);
						if (IsTimerObjectElapsed(TIME_NOTIFYPARTLIST))
							SetTimerObject(TIME_NOTIFYPARTLIST, m_Status.CurrConfInfo->PartList.size() < 3 ? 100 : 1000);
					}
					return;
				}
				dwRet = VSTRCL_UPCONT_OK;
				dwMessId = m_PresenceContainers.AddList(&cnt);
			}
			else 	dwRet = VSTRCL_ERR_CONTEYNER;
		}
		break;
	case transport::MessageType::Notify:
		dwRet = VSTRCL_ERR_TIMEOUT;
		break;
	default: 						dwRet = VSTRCL_ERR_UNKMESS;		break;
	}
	PostProc(dwRet, dwMessId);
}

void CVSTrClientProc::GetAllPartList(const char * conf)
{
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, SEARCHADDRESSBOOK_METHOD);
	cnt.AddValueI32(ADDRESSBOOK_PARAM, AB_PUBCONF);
	cnt.AddValue(CONFERENCE_PARAM, conf);

	ComposeSend(cnt, CONFERENCE_SRV, ReturnBroker(conf));
}


void CVSTrClientProc::GetExtendedStatuses(VS_Container &cnt)
{
	cnt.Reset();
	const char *curr_name(0);
	while(cnt.Next())
	{
		if(_stricmp(cnt.GetName(),CALLID_PARAM) == 0)
			m_ext_statuses[curr_name = cnt.GetStrValueRef()].Clear();
		else if(_stricmp(cnt.GetName(),EXTSTATUS_PARAM) == 0)
		{
			VS_Container ext_st_cnt;
			if (cnt.GetValue(ext_st_cnt))
				m_ext_statuses[curr_name].UpdateStatus(ext_st_cnt);
		}
	}
}


/**
****************************************************************************
* Set timer object time according specified type.
*
* \param type				- type of tymer object;
* \date    21-03-2003
******************************************************************************/
void CVSTrClientProc::SetTimerObject(int type, DWORD msec)
{
	if (type <= TIME_NONE || type >= TIME_MAX_TIMER)
		return;
	int CurrTime = timeGetTime(), DueTime = 0;
	switch(type)
	{
	case TIME_LGIN:
	case TIME_LGOUT:
	case TIME_CHUSLS:
	case TIME_JOIN:
	case TIME_CRCONF:
	case TIME_ACCEPT:
	case TIME_DLCONF0:
	case TIME_DLCONF1:		DueTime = 20000; break;
	case TIME_TSTACC:		DueTime =  7000; break;
	case TIME_INCALL:		DueTime = 100000; break;
	case TIME_CHECKSRV:		DueTime = 60000 + (VS_GenKeyByMD5()%180000); break; // 1 min + 0..3 min
	case TIME_SND_PINGCONF:	DueTime = 10000; break;
	case TIME_CONNECT:		DueTime = 25000; break;
	case TIME_SESS_RENEW:	DueTime = 23*60*60*1000; break;
	case TIME_SND_MBPSLIST: DueTime = 30000; break;
	case TIME_NOTIFYPARTLIST: DueTime = 1000; break;
	}
	if (msec!=-1)
		DueTime = msec;
	m_TimerObjects[type] = CurrTime + DueTime + rand()%100-50;
}

/**
****************************************************************************
* Test timer objects and performs action if one elapsed.
*
* \date    21-03-2003
******************************************************************************/
void CVSTrClientProc::TestTimerObjects()
{
	int CurrTime = timeGetTime(), i;
	for (i = 0; i<TIME_MAX_TIMER; i++) {
		if (m_TimerObjects[i]!=0 && CurrTime - m_TimerObjects[i] > 0) {
			switch(i)
			{
			case TIME_CHUSLS:
				SetOnlineStatus(false);
				DTRACE(VSTM_PRTCL, "### timeout LOGIN Service");
				break;
			case TIME_LGIN:
				if (m_LoginRetryCnt.IsValid()) {
					ComposeSend(m_LoginRetryCnt, AUTH_SRV);
					SetTimerObject(TIME_LGIN);
					DTRACE(VSTM_PRTCL, "### RETRY Service LOGIN resend");
					return;
				}
				SetOnlineStatus(false);
				DTRACE(VSTM_PRTCL, "### timeout LOGIN Service");
				break;
			case TIME_LGOUT:
				DTRACE(VSTM_PRTCL, "### timeout LOGIN Service");
				break;
			case TIME_JOIN:
				break;
			case TIME_ACCEPT:
				DTRACE(VSTM_PRTCL, "### timeout TIME_ACCEPT");
				break;
			case TIME_CRCONF:
				DTRACE(VSTM_PRTCL, "### timeout TIME_CRCONF");
				if (m_Status.ConfInfo[0].confState == CUserDesk::CONF_STATE_REQB)
					PostProc(VSTRCL_CONF_NOTCR);
				m_Status.ConfInfo[0].confState = m_Status.ConfInfo[0].CONF_STATE_NONE;
				break;
			case TIME_DLCONF0:
				DTRACE(VSTM_PRTCL, "### timeout TIME_DLCONF0, state = %x - %x", m_Status.ConfInfo[0].confState, m_Status.ConfInfo[1].confState);
				if (m_Status.dwStatus&STATUS_CONFERENCE
					&& m_Status.ConfInfo[0].confState&CUserDesk::CONF_STATE_REQEND
					&& m_Status.CurrConfInfo->confState != CUserDesk::CONF_STATE_DONE)
				{
					m_Status.dwStatus&=~STATUS_CONFERENCE;
					m_Status.ConfInfo[0].Clean();
					PostProc(VSTRCL_CONF_CALL);
				}
				else
					m_Status.ConfInfo[0].confState = m_Status.ConfInfo[0].CONF_STATE_NONE;
				break;
			case TIME_DLCONF1:
				DTRACE(VSTM_PRTCL, "### timeout TIME_DLCONF1, states: = %x - %x", m_Status.ConfInfo[1].confState, m_Status.ConfInfo[0].confState);
				if (m_Status.dwStatus&STATUS_CONFERENCE
					&& m_Status.ConfInfo[1].confState&CUserDesk::CONF_STATE_REQEND
					&& m_Status.CurrConfInfo->confState != CUserDesk::CONF_STATE_DONE)
				{
					m_Status.dwStatus&=~STATUS_CONFERENCE;
					m_Status.ConfInfo[1].Clean();
					PostProc(VSTRCL_CONF_CALL);
				}
				else
					m_Status.ConfInfo[1].confState = m_Status.ConfInfo[1].CONF_STATE_NONE;
				break;
			case TIME_TSTACC:
				DTRACE(VSTM_PRTCL, "### timeout TIME_TSTACC");
				if (m_Status.dwStatus&STATUS_INCALL_PROGRESS) {
					bool NhpOk = WaitForSingleObject(m_NhpEvent, 0)== WAIT_OBJECT_0;
					if (NhpOk)
						m_Status.ConfInfo[1].ConnectType = DIRECT_NHP;
					else
						m_Status.ConfInfo[1].ConnectType = NO_DIRECT_CONNECT;
					SetIncallStatus(true, VSTRCL_CONF_CALL, m_Status.ConfInfo[1].invNum);
				}
				break;
			case TIME_INCALL:
				DTRACE(VSTM_PRTCL, "### timeout TIME_INCALL");
				if (m_Status.dwStatus&STATUS_INCALL) {
					SetIncallStatus(false);
				}
				if (m_Status.dwStatus&STATUS_REQINVITE) {
					SetReqInviteStatus(false);
				}
				break;
				//---------------------------periodic --------------------------------//
			case TIME_CHECKSRV:
				if (m_Status.dwStatus&STATUS_SERVAVAIL)
					if (!(m_Status.dwStatus&(STATUS_INCALL|STATUS_CONFERENCE|STATUS_REQINVITE|STATUS_INCALL_PROGRESS)))
						m_ServerList.CheckServers(m_CurrBroker);
				break;
			case TIME_SND_PINGCONF:
				PingConference();
				break;
			case TIME_CONNECT:
				PostProc(DoConnect(true));
				break;
			case TIME_SESS_RENEW:
				if (m_Status.dwStatus&STATUS_LOGGEDIN)
				{
					VS_Container cnt;
					cnt.AddValue(METHOD_PARAM, REQUPDATEACCOUNT_METHOD);
					cnt.AddValue(CALLID_PARAM, m_Status.MyInfo.UserName);
					ComposeSend(cnt, AUTH_SRV);
					SetTimerObject(TIME_SESS_RENEW);
					DTRACE(VSTM_PRTCL, "### REFRESH Login Session Key");
					return ;
				}
				break;
			case TIME_SND_MBPSLIST:
				if (m_Status.dwStatus&STATUS_CONFERENCE) {
					UpdateMBpsUsers();
					SetTimerObject(TIME_SND_MBPSLIST);
					return;
				}
				break;
            case TIME_TR_UPDATE_STATUS:
                if (TrUpdateStatus()) {
                    return;
                }
                break;
			case TIME_NOTIFYPARTLIST:
				PostProc(VSTRCL_UPCONT_OK, -1);
				break;
			default:
				break;
			}
			RemoveTimerObjects(i);
		}
	}
}

/**
****************************************************************************
* Test timer object if elapsed.
*
* \param type					- type of tymer object;
* \date    21-03-2003
******************************************************************************/
bool CVSTrClientProc::IsTimerObjectElapsed(int type)
{
	if (type <= TIME_NONE || type >= TIME_MAX_TIMER)
		return true;
	else if (m_TimerObjects[type]==0)
		return true;
	else {
		int CurrTime = timeGetTime();
		return CurrTime - m_TimerObjects[type] > 0;
	}
}

/**
****************************************************************************
* Remove all timer objects of specified type.
*
* \param type					- type of tymer object;
* \date    21-03-2003
******************************************************************************/
void CVSTrClientProc::RemoveTimerObjects(int type)
{
	if (type <= TIME_NONE || type >= TIME_MAX_TIMER)
		return;
	else
		m_TimerObjects[type] = 0;
}

DWORD CVSTrClientProc::ManageGroups_CreateGroup(VS_WideStr gname)
{
	if (!gname)  return 0;
	DTRACE(VSTM_PRTCL, "request = ManageGroups_CreateGroup, name %S", gname.m_str);
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, MANAGEGROUPS_METHOD);
	rCnt.AddValueI32(CMD_PARAM, MGC_CREATE_GROUP);
	rCnt.AddValue(GNAME_PARAM, gname.m_str);
	return ComposeSend(rCnt, ADDRESSBOOK_SRV);
}

DWORD CVSTrClientProc::ManageGroups_DeleteGroup(long gid)
{
	if (!gid)  return 0;
	DTRACE(VSTM_PRTCL, "request = ManageGroups_DeleteGroup, gid %d", gid);
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, MANAGEGROUPS_METHOD);
	rCnt.AddValueI32(CMD_PARAM, MGC_DELETE_GROUP);
	rCnt.AddValueI32(GID_PARAM, gid);
	return ComposeSend(rCnt, ADDRESSBOOK_SRV);
}

DWORD CVSTrClientProc::ManageGroups_RenameGroup(long gid, VS_WideStr gname)
{
	if (!gname)  return 0;
	DTRACE(VSTM_PRTCL, "request = ManageGroups_RenameGroup, gid %d, gname %S", gid, gname.m_str);
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, MANAGEGROUPS_METHOD);
	rCnt.AddValueI32(CMD_PARAM, MGC_RENAME_GROUP);
	rCnt.AddValueI32(GID_PARAM, gid);
	rCnt.AddValue(GNAME_PARAM, gname.m_str);
	return ComposeSend(rCnt, ADDRESSBOOK_SRV);
}

DWORD CVSTrClientProc::ManageGroups_AddUser(long gid, VS_SimpleStr user)
{
	if (!gid || !user)  return 0;
	DTRACE(VSTM_PRTCL, "request = ManageGroups_AddUser, gid %d, user %s", gid, user.m_str);
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, MANAGEGROUPS_METHOD);
	rCnt.AddValueI32(CMD_PARAM, MGC_ADD_USER);
	rCnt.AddValueI32(GID_PARAM, gid);
	rCnt.AddValue(CALLID_PARAM, user.m_str);
	return ComposeSend(rCnt, ADDRESSBOOK_SRV);
}

DWORD CVSTrClientProc::ManageGroups_DeleteUser(long gid, VS_SimpleStr user)
{
	if (!gid || !user)  return 0;
	DTRACE(VSTM_PRTCL, "request = ManageGroups_DeleteUser, gid %d, user %s", gid, user.m_str);
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, MANAGEGROUPS_METHOD);
	rCnt.AddValueI32(CMD_PARAM, MGC_DELETE_USER);
	rCnt.AddValueI32(GID_PARAM, gid);
	rCnt.AddValue(CALLID_PARAM, user.m_str);
	return ComposeSend(rCnt, ADDRESSBOOK_SRV);
}


void CVSTrClientProc::SetClientType(const VS_ClientType ct)
{
	m_client_type = ct;
	m_Status.MyInfo.ClientCaps.SetClientType((long)ct);
}

/********************************************/
int CVSTrClientProc::ProcessFunction(VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar)
{
	_variant_t* var=(_variant_t*)pVar;

	std::unique_ptr<VARIANT[]> vars;
	int count = 0;
	{
		VARIANT *_vars = 0;
		count = ExtractVars(_vars, pVar);
		vars.reset(_vars);
	}

    if (VS_OPERATION == RUN_COMMAND && _stricmp(pSection, "SetFileTransferCB") == 0) {
        if (count != 2) return VS_INTERFACE_INTERNAL_ERROR;
        pFileSendStartedCB = reinterpret_cast<decltype(pFileSendStartedCB)>(vars[0].ulVal);
        pFileDownloadStatusCB = reinterpret_cast<decltype(pFileDownloadStatusCB)>(vars[1].ulVal);
        return VS_INTERFACE_OK;
    } else if (VS_OPERATION == RUN_COMMAND && _stricmp(pSection, "SendFileByChat") == 0) {
        if (count != 3) return VS_INTERFACE_INTERNAL_ERROR;
        char *filePath = VS_UCSToUTF8(vars[0].bstrVal);

        std::vector<std::string> user_id_list;

        long l, u;
        SAFEARRAY *psa = vars[1].parray;
        SafeArrayGetLBound(psa, 1, &l);
        SafeArrayGetUBound(psa, 1, &u);
        int num = u - l + 1;
        if (num >= 1) {
            int num = u - l + 1;
            VARIANT v;
            for (long i = 0; i < num; ++i) {
                SafeArrayGetElement(psa, &i, &v);
                char *user_id = VS_UCSToUTF8(v.bstrVal);
                if (user_id) {
                    user_id_list.emplace_back(user_id);
                    free(user_id);
                }
            }
        }

        std::string id;
        TrSendFile(filePath, user_id_list, id);

        VARIANT v = _variant_t(id.c_str());
        psa = var->parray;
        long index = 2;
        SafeArrayPutElement(psa, &index, &v);

        free(filePath);
        return VS_INTERFACE_OK;
    } else if (VS_OPERATION == RUN_COMMAND && _stricmp(pSection, "SendFileByChatToConf") == 0) {
        if (count != 3) return VS_INTERFACE_INTERNAL_ERROR;
        char *filePath = VS_UCSToUTF8(vars[0].bstrVal);
        char *conf_id = VS_UCSToUTF8(vars[1].bstrVal);

        std::string id;
        TrSendFile(filePath, conf_id, id);

        VARIANT v = _variant_t(id.c_str());
        SAFEARRAY *psa = var->parray;
        long index = 2;
        SafeArrayPutElement(psa, &index, &v);

        free(filePath);
        free(conf_id);
        return VS_INTERFACE_OK;
    } else if (VS_OPERATION == RUN_COMMAND && _stricmp(pSection, "DownloadFile") == 0) {
        if (count != 3) return VS_INTERFACE_INTERNAL_ERROR;
        char *magnet = VS_UCSToUTF8(vars[0].bstrVal);
        char *distPath = VS_UCSToUTF8(vars[1].bstrVal);

        std::string id;
        std::string m = magnet;
        // [magnet] -> magnet
        if (!m.empty() && m[0] == '[') {
            m = m.substr(1, m.length());
        }
        if (!m.empty() && m.back() == ']') {
            m.pop_back();
        }
        TrDownloadFile(m.c_str(), distPath, id);

        VARIANT v = _variant_t(id.c_str());
        SAFEARRAY *psa = var->parray;
        long index = 2;
        SafeArrayPutElement(psa, &index, &v);

        free(magnet);
        free(distPath);
        return VS_INTERFACE_OK;
    } else if (VS_OPERATION == RUN_COMMAND && _stricmp(pSection, "SetLimits") == 0) {
        if (count != 2) return VS_INTERFACE_INTERNAL_ERROR;

        unsigned int dw_lim = vars[0].ulVal,
                     up_lim = vars[1].ulVal;

        TrSetLimits(dw_lim, up_lim);

        return VS_INTERFACE_OK;
	} else if (VS_OPERATION == GET_PARAM && _stricmp(pSection, "GetTorrentStats") == 0) {
		if (count != 6 && count != 7) return VS_INTERFACE_INTERNAL_ERROR;

		char *id = VS_UCSToUTF8(vars[0].bstrVal);
		if (!id) return VS_INTERFACE_INTERNAL_ERROR;
		std::unique_ptr<char, free_deleter> _(id);

		TrStat stat;
		{
			VS_AutoLock lock(this);
			auto it = m_items.find(id);
			if (it == m_items.end()) {
				return VS_INTERFACE_INTERNAL_ERROR;
			}
			TrGetStat(it->second.magnet, stat);
		}

		SAFEARRAY *psa = var->parray;
		VARIANT v_out[] = { _variant_t((ULONGLONG)stat.size),
							_variant_t((ULONGLONG)stat.downloaded),
							_variant_t((ULONGLONG)stat.uploaded),
							_variant_t((ULONG)stat.u_speed),
							_variant_t((ULONG)stat.d_speed),
							_variant_t((ULONG)stat.peers) };
		for (int i = 0; i < count - 1; i++) {
			long index = i + 1;
			if (SafeArrayPutElement(psa, &index, &v_out[i]) != S_OK) {
				return VS_INTERFACE_INTERNAL_ERROR;
			}
		}
		return VS_INTERFACE_OK;
	}
	/*if (_stricmp(pSection, "TorrentFileTransfer")==0)
	{
		VARIANT *vars = 0;
		int count = ExtractVars(vars, pVar);
		if (count < 1) return 0;*/

		/*if ( wcscmp(name, L"Init") == 0 )
		{
			m_torrentGUICallback = (GUICallback)vars[1].ulVal;
		} else
		if (wcscmp(name, L"SendFile") == 0)
		{
			char *file = VS_UCSToUTF8(vars[1].bstrVal);
			char *from = VS_UCSToUTF8(vars[2].bstrVal);
			m_torrent.SendFile(file, from);
			free(file);
			free(from);
		}
		else
		if (wcscmp(name, L"ConfirmDownload") == 0)
		{
			char *infoHash = VS_UCSToUTF8(vars[1].bstrVal);
			char *path = VS_UCSToUTF8(vars[3].bstrVal);
			m_torrent.GetTorrent(infoHash).ConfirmDownload(!!vars[2].boolVal, path);
			free(infoHash);
			free(path);
		}
		else
		if (wcscmp(name, L"GetStatistics") == 0)
		{
			std::string res;
			m_torrent.GetStatisticsAsJSON( res );
			wchar_t *data = VS_UTF8ToUCS( res.c_str() );
			*var = data;
			free( data );
			return true;
		}
		else
		if (wcscmp(name, L"RemoveTorrent") == 0)
		{
			char *infoHash = VS_UCSToUTF8(vars[1].bstrVal);
			bool with_files = vars[2].boolVal;
			m_torrent.GetTorrent(infoHash).RemoveTorrent(with_files);
			free(infoHash);
		}
		if (wcscmp(name, L"Pause") == 0)
		{
			char *infoHash = VS_UCSToUTF8(vars[1].bstrVal);
			m_torrent.GetTorrent(infoHash).Pause();
			free(infoHash);
		}
		if (wcscmp(name, L"Resume") == 0)
		{
			char *infoHash = VS_UCSToUTF8(vars[1].bstrVal);
			m_torrent.GetTorrent(infoHash).Resume();
			free(infoHash);
		}
		var->intVal = 1;
		return true;
	} */
    else
	if (_stricmp(pSection, "RoleEvents")==0) {
		if (VS_OPERATION==GET_PARAM) {
			VS_Container *cnt = m_RoleEventContainers.GetList((int)*var);
			if (cnt) {
				m_result_cnt = std::make_unique<VS_ContainerData>(*cnt);
				*var = (int)m_result_cnt.get();
			}
			else
				*var = (int)0;
			return VS_INTERFACE_OK;
		}
		else if (VS_OPERATION==RUN_COMMAND) {
			if (var->vt==(VT_ARRAY|VT_VARIANT)) {
				long l, u;
				DWORD retval;
				SAFEARRAY *psa=var->parray;
				SafeArrayGetLBound(psa,1,&l);
				SafeArrayGetUBound(psa,1,&u);
				int num = u-l + 1;
				if (num > 2){
					long i;
					VARIANT *vars = new VARIANT[num];
					for(i = 0; i < num; ++i)
						VariantInit(&vars[i]);
					for(i = 0; i < num; ++i)
						SafeArrayGetElement(psa, &i, &vars[i]);
					if (vars[0].lVal==0)
						retval = QueryRole(vars[1].pcVal, vars[2].lVal);
					else if (vars[0].lVal==1)
						retval = AnswerRole(vars[1].pcVal, vars[2].lVal, vars[3].lVal);
					delete [] vars;
					return retval;
				}
			}
		}
	}
	else if (_stricmp(pSection, "GetAbList")==0) {
		if (VS_OPERATION==RUN_COMMAND) {
			*var = (int)0;
			return VS_INTERFACE_OK;
		}
		else if (VS_OPERATION == GET_PARAM) {
			int key = (int)*var;
			if (key != -1) {
				VS_Container *cnt = m_PresenceContainers.GetList(key);
				if (cnt) {
					m_result_cnt = std::make_unique<VS_ContainerData>(*cnt);
					*var = (int)m_result_cnt.get();
				}
				else
					*var = (int)0;
			}
			else {
				{
					m_PartList.Clear();
					m_PartList.AddValue(METHOD_PARAM, SENDPARTSLIST_METHOD);
					m_PartList.AddValue(CONFERENCE_PARAM, m_Status.CurrConfInfo->Conference);
					VS_AutoLock lock(this); // lock PartList
					for (auto &i : m_Status.CurrConfInfo->PartList) {
						m_PartList.AddValue(USERNAME_PARAM, i.first);
						m_PartList.AddValue(DISPLAYNAME_PARAM, i.second.dn);
						m_PartList.AddValueI32(ROLE_PARAM, i.second.role);
						m_PartList.AddValue(IS_OPERATOR_PARAM, i.second.is_operator);
						m_PartList.AddValueI32(DEVICESTATUS_PARAM, i.second.device_status);
					}
				}
				m_result_cnt = std::make_unique<VS_ContainerData>(m_PartList);
				*var = (int)m_result_cnt.get();
			}
			return VS_INTERFACE_OK;
		}
	}
	else if	(_stricmp(pSection, "EditAb")==0) {
		if (var->vt == (VT_ARRAY|VT_VARIANT)) {
			long l, u;
			SAFEARRAY *psa=var->parray;
			SafeArrayGetLBound(psa,1,&l);
			SafeArrayGetUBound(psa,1,&u);
			if((u-l)>=1){
				int num = u-l + 1; long i;
				VARIANT *vars = new VARIANT[num];
				for(i = 0; i < num; ++i) {
					VariantInit(&vars[i]);
					SafeArrayGetElement(psa, &i, &vars[i]);
				}
				if	(VS_OPERATION == RUN_COMMAND) {
					if (vars[0].lVal==0) {
						m_UpdateABContainer.Clear();
						m_UpdateABContainer.AddValue(METHOD_PARAM, UPDATEADDRESSBOOK_METHOD);
						m_UpdateABContainer.AddValueI32(ADDRESSBOOK_PARAM, vars[1].lVal);
						m_UpdateABContainer.AddValue(CALLID_PARAM, vars[2].pcVal);
					}
					else {
						if (m_UpdateABContainer.IsValid()) {
							ComposeSend(m_UpdateABContainer, ADDRESSBOOK_SRV);
							m_UpdateABContainer.Clear();
						}
					}
				}
				else if (VS_OPERATION == SET_PARAM) {
					if (m_UpdateABContainer.IsValid() && static_cast<const char*>(vars[1].pcVal)) {
						if		(vars[0].ulVal==UTYPE_BOOL)
							m_UpdateABContainer.AddValue(vars[1].pcVal, vars[2].lVal!=0);
						else if (vars[0].ulVal==UTYPE_LONG)
							m_UpdateABContainer.AddValueI32(vars[1].pcVal, vars[2].lVal);
						else if (vars[0].ulVal==UTYPE_DBL)
							m_UpdateABContainer.AddValue(vars[1].pcVal, vars[2].dblVal);
						else if (vars[0].ulVal==UTYPE_CHAR)
							m_UpdateABContainer.AddValue(vars[1].pcVal, vars[2].pcVal);
						else if (vars[0].ulVal==UTYPE_VOID)
							m_UpdateABContainer.AddValue(vars[1].pcVal, vars[2].byref, vars[3].lVal);
					}
				}
				delete [] vars;
				return VS_INTERFACE_OK;
			}
		}
		return VS_INTERFACE_INTERNAL_ERROR;
	}
	else if	(_stricmp(pSection, "UserPhoto")==0){
		if (VS_OPERATION==RUN_COMMAND) {
			if(var->vt==(VT_ARRAY|VT_VARIANT)){
				long l, u;
				SAFEARRAY *psa=var->parray;
				SafeArrayGetLBound(psa,1,&l);
				SafeArrayGetUBound(psa,1,&u);
				if((u-l)>=1){
					int num = u-l + 1; long i;
					VARIANT *vars = new VARIANT[num];
					for(i = 0; i < num; ++i)
						VariantInit(&vars[i]);
					for(i = 0; i < num; ++i)
						SafeArrayGetElement(psa, &i, &vars[i]);
					DWORD retval = GetUserPhoto(vars[0].pcVal, (void**)vars[1].byref, vars[2].pulVal, (char**)vars[3].byref);
					delete [] vars;
					return retval;
				}
			}
		}
		return VS_INTERFACE_INTERNAL_ERROR;
	}
	else if (_stricmp(pSection, "UserStatus")==0) {
		switch(VS_OPERATION)
		{
		case RUN_COMMAND:
			GetUserStatus((char*)(_bstr_t)*var);
			return VS_INTERFACE_OK;
		}
	}
	else if(_stricmp(pSection, "Property")==0) {
		switch(VS_OPERATION)
		{
		case SET_PARAM:
			if(var->vt==(VT_ARRAY|VT_VARIANT)){
				long l,u;
				SAFEARRAY *psa=var->parray;
				SafeArrayGetLBound(psa,1,&l);
				SafeArrayGetUBound(psa,1,&u);
				if((u-l)>=1){
					_variant_t prName,prValue;
					SafeArrayGetElement(psa,&l,&prName.GetVARIANT());
					l++;
					SafeArrayGetElement(psa,&l,&prValue.GetVARIANT());
					l++;
					SetEpProperties((_bstr_t)prName,(_bstr_t)prValue);
					return VS_INTERFACE_OK;
				}
			}
		}
	}
	else if(_stricmp(pSection, "Invite")==0) {
		switch(VS_OPERATION)
		{
		case RUN_COMMAND:
			InviteToMulti((char*)(_bstr_t)*var);
			return VS_INTERFACE_OK;
		case GET_PARAM:
			VS_Container *cnt = m_InviteContainers.GetList((int)*var);
			*var = int(0);
			if (cnt) {
				const char* method = cnt->GetStrValueRef(METHOD_PARAM);
				if (_stricmp(method, INVITE_METHOD)==0)
					*var = (int)cnt->GetStrValueRef(NAME_PARAM);
				else if (_stricmp(method, INVITETOMULTI_METHOD)==0)
					*var = (int)cnt->GetStrValueRef(USERNAME_PARAM);
			}
			return VS_INTERFACE_OK;
		}
	}
	else if (strcmp(pSection, INVITETOMULTI_METHOD)==0) {
		if (VS_OPERATION==GET_PARAM) {
			VS_Container *cnt = m_InviteContainers.GetList((int)*var);
			if (cnt) {
				m_result_cnt = std::make_unique<VS_ContainerData>(*cnt);
				*var = (int)m_result_cnt.get();
			}
			else
				*var = (int)0;
			return VS_INTERFACE_OK;
		}
	}
	else if(_stricmp(pSection, "InputBandwidth")==0) {
		switch(VS_OPERATION)
		{
		case GET_PARAM:
			*var=m_Status.MyInfo.ClientCaps.GetBandWRcv();
			return VS_INTERFACE_OK;
		case SET_PARAM:
			if (int(*var)<32 || int(*var)>10240)
				*var=1024;
			m_Status.MyInfo.ClientCaps.SetBandWRcv(*var);
			return VS_INTERFACE_OK;
		}
	}
	else if(_stricmp(pSection, "Call")==0) {
		switch(VS_OPERATION)
		{
		case RUN_COMMAND:
			if(var->vt==(VT_ARRAY|VT_VARIANT)){
				long l,u;
				SAFEARRAY *psa=var->parray;
				SafeArrayGetLBound(psa,1,&l);
				SafeArrayGetUBound(psa,1,&u);
				if((u-l)>=1){
					int Result;
					_variant_t conf_type,called,password(""),users(6), scope(0);
					SafeArrayGetElement(psa,&l,&conf_type.GetVARIANT());
					l++;
					SafeArrayGetElement(psa,&l,&called.GetVARIANT());
					l++;
					if((u-l)>=0)
						SafeArrayGetElement(psa,&l,&password.GetVARIANT());
					l++;
					if((u-l)>=0)
						SafeArrayGetElement(psa,&l,&users.GetVARIANT());
					l++;
					if((u-l)>=0)
						SafeArrayGetElement(psa,&l,&scope.GetVARIANT());

					switch(int(conf_type))
					{
					case MCT_PRIVATE_CALL:
						Result= PlaseCall((char*)(_bstr_t)called);
						break;
					case MCT_BROADCAST_START:
						Result= CreatePublicConf(CT_BROADCAST);
						break;
					case MCT_BROADCAST_JOIN:
						Result= Join((char*)(_bstr_t)called,(char*)(_bstr_t)password,CT_BROADCAST);
						break;
					case MCT_MULTI_JOIN:
						Result= Join((char*)(_bstr_t)called,(char*)(_bstr_t)password,CT_MULTISTREAM);
						break;
					case MCT_INTERCOM_JOIN:
						Result= Join((char*)(_bstr_t)called,(char*)(_bstr_t)password,CT_INTERCOM);
						break;
					case MCT_FREE_START:
						Result= CreatePublicConf(CT_PUBLIC);
						break;
					case MCT_VIP_START:
						Result= CreatePublicConf(CT_VIPPUBLIC, (char*)(_bstr_t)password);
						break;
					case MCT_HALFPRIVATE_CALL:
						Result= PlaseCall((char*)(_bstr_t)called, CT_HALF_PRIVATE);
						break;
					case MCT_FREE_JOIN:
						Result= Join((char*)(_bstr_t)called,(char*)(_bstr_t)password,CT_PUBLIC);
						break;
					case MCT_VIP_JOIN:
						Result= Join((char*)(_bstr_t)called,(char*)(_bstr_t)password,CT_VIPPUBLIC);
						break;
					case MCT_HALFPRIVATE_JOIN:
						Result= Join((char*)(_bstr_t)called,(char*)(_bstr_t)password,CT_HALF_PRIVATE);
						break;
					case MCT_CREATEMULTI:
						Result= CreateMultiConf((_bstr_t)called, (_bstr_t)password, CT_MULTISTREAM, users, GCST_FULL, scope);
						break;
					case MCT_REQINVITE:
						Result= ReqInviteToMulti((char*)(_bstr_t)called);
						break;
					case MCT_ONE2MANY:
						Result= CreateMultiConf((_bstr_t)called, (_bstr_t)password, CT_MULTISTREAM, users, GCST_ALL_TO_OWNER, scope);
						break;
					case MCT_ONE2MANY_PEND:
						Result= CreateMultiConf((_bstr_t)called, (_bstr_t)password, CT_MULTISTREAM, users, GCST_PENDING_ALL_TO_OWNER, scope);
						break;
					case MCT_CREATE_INTERCOM:
						Result= CreateMultiConf((_bstr_t)called, (_bstr_t)password, CT_INTERCOM, users, GCST_FULL, scope);
						break;
					case MCT_INTERCOM_ONE2MANY:
						Result= CreateMultiConf((_bstr_t)called, (_bstr_t)password, CT_INTERCOM, users, GCST_ALL_TO_OWNER, scope);
						break;
					case MCT_INTERCOM_ONE2MANY_PEND:
						Result= CreateMultiConf((_bstr_t)called, (_bstr_t)password, CT_INTERCOM, users, GCST_PENDING_ALL_TO_OWNER, scope);
						break;
					case MCT_ROLE:
						Result= CreateMultiConf((_bstr_t)called, (_bstr_t)password, CT_MULTISTREAM, users, GCST_ROLE, scope);
						break;
					case MCT_INTERCOM_ROLE:
						Result= CreateMultiConf((_bstr_t)called, (_bstr_t)password, CT_INTERCOM, users, GCST_ROLE, scope);
						break;

					}
					if(Result==ERR_OK)
						return VS_INTERFACE_OK;
				}
			}
			return VS_INTERFACE_INTERNAL_ERROR;
		}
	}
	else if(_stricmp(pSection, "SSLInfo")==0) {
		VS_WideStr par[5];
		VS_SimpleStr par2[2];
		if (!VS_GetCertificateData(par[0], par[1], par[2], par[3], par[4], par2[0], par2[1]))
			return VS_INTERFACE_INTERNAL_ERROR;
		SAFEARRAYBOUND rgsabound[1];
		SAFEARRAY * psa;
		rgsabound[0].lLbound = 0;
		rgsabound[0].cElements = 7;
		psa = SafeArrayCreate(VT_VARIANT, 1, rgsabound);
		if (psa==0)
			return VS_INTERFACE_INTERNAL_ERROR;

		var->parray=psa;
		var->vt= VT_ARRAY | VT_VARIANT;
		_variant_t var_;
		long i(0);
		for ( i = 0; i<5; i++) {
			var_= (wchar_t*)par[i];
			SafeArrayPutElement(psa, &i, &var_.GetVARIANT());
		}
		for(;i<7;i++){
			var_= (char*)par2[i-5];
			SafeArrayPutElement(psa, &i, &var_.GetVARIANT());
		}
		return VS_INTERFACE_OK;
	}
	else if (_stricmp(pSection, "WinMessage")==0) {
		if (VS_OPERATION==RUN_COMMAND) {
			struct Tmessage
			{
				int msg;
				int wParam;
				int lParam;
				int result;
			};
			Tmessage *tmsg = (Tmessage *)var->intVal;
			if (tmsg->msg==WM_POWERBROADCAST) {
				DTRACE(VSTM_PRTCL, "POWER event %d", tmsg->wParam);
				if (tmsg->wParam==PBT_APMSUSPEND) {
					DisconnectProcedure();
				}
				else if (tmsg->wParam==PBT_APMRESUMECRITICAL || tmsg->wParam==PBT_APMRESUMESUSPEND) {
					// restore connections
					ConnectServer(m_CurrBroker);
				}
			}
			return VS_INTERFACE_OK;
		}
	}
	else if (_stricmp(pSection, CONNECTSENDER_METHOD)==0) {
		if (VS_OPERATION==RUN_COMMAND) {
			if (var->intVal==1) var->intVal = VS_RcvFunc::FLTR_DEFAULT_MULTIS;
			ConnectSender(var->intVal);
			return VS_INTERFACE_OK;
		}
	}
	else if (_stricmp(pSection, CONNECTSERVICES_METHOD)==0) {
		if (VS_OPERATION==RUN_COMMAND) {
			//if (var->intVal==1) var->intVal = VS_RcvFunc::FLTR_DEFAULT_MULTIS;
			ConnectServices(var->intVal);
			return VS_INTERFACE_OK;
		}
	}
	else if (_stricmp(pSection, "ConferenceInfo")==0) {
		if (VS_OPERATION==GET_PARAM) {
			_variant_t vars[10];
			vars[0] = (char*)m_Status.CurrConfInfo->UserName;
			vars[1] = (char*)m_Status.CurrConfInfo->Cname;
			vars[2] = m_Status.CurrConfInfo->ServerType;
			vars[3] = m_Status.CurrConfInfo->ServerSubType;
			vars[4] = m_Status.CurrConfInfo->ServerScope;
			vars[5] = m_Status.CurrConfInfo->MaxParts;
			vars[6] = m_Status.CurrConfInfo->MaxCasts;
			vars[7] = m_Status.CurrConfInfo->LStatus;
			vars[8] = (char*)m_Status.CurrConfInfo->Conference;
			vars[9] = (int32_t)m_Status.CurrConfInfo->cmr_flags;
			if (CombineVars(var, vars, 10))
				return VS_INTERFACE_OK;
			else
				return VS_INTERFACE_INTERNAL_ERROR;
		}
	}
	else if (_stricmp(pSection, "ActionUser")==0) {
		if (VS_OPERATION==GET_PARAM) {
			char user[256] = {0};
			m_Status.CurrConfInfo->RsvList.GetActionUser(var->lVal, user);
			*var = (char*)user;
			return VS_INTERFACE_OK;
		}
	}
	else if (_stricmp(pSection, INVITEREPLY_METHOD)==0) {
		switch(VS_OPERATION)
		{
		case RUN_COMMAND:
			InviteReply(*var);
			return VS_INTERFACE_OK;
		case GET_PARAM:
			VS_Container *cnt = m_InviteContainers.GetList(*var);
			if (cnt) {
				SAFEARRAYBOUND rgsabound[1];
				SAFEARRAY * psa;
				rgsabound[0].lLbound = 0;
				rgsabound[0].cElements = 2;
				psa = SafeArrayCreate(VT_VARIANT, 1, rgsabound);
				if (psa!=0) {
					var->parray=psa;
					var->vt= VT_ARRAY | VT_VARIANT;
					_variant_t var_;
					long i = 0;
					var_= (char*)cnt->GetStrValueRef(USERNAME_PARAM);
					SafeArrayPutElement(psa, &i, &var_.GetVARIANT());
					i++;
					int32_t result = IRT_ACCEPT; cnt->GetValue(RESULT_PARAM, result);
					var_= result;
					SafeArrayPutElement(psa, &i, &var_.GetVARIANT());
					return VS_INTERFACE_OK;
				}
			}
			return VS_INTERFACE_INTERNAL_ERROR;
		}
	}
	else if (_stricmp(pSection, REJECT_METHOD)==0) {
		switch(VS_OPERATION)
		{
		case RUN_COMMAND:
			Reject(0, 0, *var);
			return VS_INTERFACE_OK;
		}
	}
	else if (_stricmp(pSection, "FailedEndpoint")==0) {
		switch(VS_OPERATION)
		{
		case GET_PARAM:
			if (!!m_FailedEndpoint) {
				const unsigned num = net::endpoint::GetCountConnectTCP(m_FailedEndpoint.m_str);
				if (num) {
					SAFEARRAYBOUND rgsabound[1];
					SAFEARRAY * psa;
					rgsabound[0].lLbound = 0;
					rgsabound[0].cElements = num*2;
					psa = SafeArrayCreate(VT_VARIANT, 1, rgsabound);
					if (psa!=0) {
						var->parray=psa;
						var->vt= VT_ARRAY | VT_VARIANT;
						_variant_t var_;
						long i = 0;
						for (unsigned j = 0; j < num; ++j)
						{
							auto tcp = net::endpoint::ReadConnectTCP(j + 1, m_FailedEndpoint.m_str);
							if (tcp) {
								var_= tcp->host.c_str();
								SafeArrayPutElement(psa, &i, &var_.GetVARIANT());
								i++;
								var_= tcp->port;
								SafeArrayPutElement(psa, &i, &var_.GetVARIANT());
								i++;
							}
						}
						return VS_INTERFACE_OK;
					}
				}
			}
			return VS_INTERFACE_INTERNAL_ERROR;
		}
	}
	else if (_stricmp(pSection, "Subscribe")==0) {
		if (var->vt==(VT_ARRAY|VT_VARIANT)) {
			long l, u;
			SAFEARRAY *psa = var->parray;
			SafeArrayGetLBound(psa, 1, &l);
			SafeArrayGetUBound(psa, 1, &u);
			int num = u-l + 1;
			if (num > 0) {
				VS_SimpleStr *p = new VS_SimpleStr[num];
				for (long i = 0; i < num; ++i) {
					_variant_t name;
					SafeArrayGetElement(psa, &i, &name.GetVARIANT());
					p[i] = vs::StrFromVariantT(name);
				}
				Subscribe((char**)p, num, VS_OPERATION!=SET_PARAM);
				delete [] p;
				return VS_INTERFACE_OK;
			}
		}
		return VS_INTERFACE_INTERNAL_ERROR;
	}
	else if (_stricmp(pSection, "ConferencePic")==0) {
		if (var->vt==(VT_ARRAY|VT_VARIANT)) {
			long l, u;
			SAFEARRAY *psa = var->parray;
			SafeArrayGetLBound(psa,1,&l);
			SafeArrayGetUBound(psa,1,&u);
			if (u-l >= 1) {
				_variant_t size(0), buff(0);
				SafeArrayGetElement(psa, &l, &buff.GetVARIANT());
				l++;
				SafeArrayGetElement(psa, &l, &size.GetVARIANT());
				SendConferncePicture(buff.byref, size);
				return VS_INTERFACE_OK;
			}
		}
		return VS_INTERFACE_INTERNAL_ERROR;
	}
	else if (_stricmp(pSection, "MyInfo")==0) {
		_variant_t vars[7];
		vars[0] = (char*)m_Status.MyInfo.UserName;
		VS_WideStr dn; dn.AssignUTF8(m_Status.MyInfo.DisplayName);
		vars[1] = (wchar_t*)dn;
		vars[2] = m_Status.MyInfo.Rights;
		vars[3] = (char*)m_Status.MyInfo.TarifPlan;
		vars[4] = (char*)m_Status.MyInfo.SessionKey;
		vars[5] = m_Status.MyInfo.TarifRestrictions;
		vars[6] = m_Status.MyInfo.SepartionGroup;
		if (CombineVars(var, vars, 7))
			return VS_INTERFACE_OK;
		else
			return VS_INTERFACE_INTERNAL_ERROR;
	}
	else if (_stricmp(pSection, "SearchAB")==0) {
		bool res = false;
		VARIANT *vars = 0;
		int num = ExtractVars(vars, var);
		if (num > 2) {
			VS_Container rCnt;
			rCnt.AddValue(METHOD_PARAM, SEARCHADDRESSBOOK_METHOD);
			rCnt.AddValueI32(ADDRESSBOOK_PARAM, AB_PERSONS);
			rCnt.AddValue(EMAIL_PARAM, (char*)(_bstr_t)vars[0]);
			rCnt.AddValue(CALLID_PARAM, (char*)(_bstr_t)vars[1]);
			rCnt.AddValue(NAME_PARAM, (wchar_t*)(_bstr_t)vars[2]);

			res = ComposeSend(rCnt, ADDRESSBOOK_SRV) > 0;
		}
		if (num > 0) delete[] vars;
		return res ? VS_INTERFACE_OK : VS_INTERFACE_INTERNAL_ERROR;
	}
	else if (_stricmp(pSection, "VideoQuality")==0) {
		VS_MediaFormat mf;
		if (GetMediaFormat((char*)(_bstr_t)*var, mf) >= 0) {
			*var = mf.GetModeResolution();
			return VS_INTERFACE_OK;
		}
		else
			return VS_INTERFACE_INTERNAL_ERROR;
	}
	else if (_stricmp(pSection, "ManageGroups")==0) {
		if (VS_OPERATION==RUN_COMMAND)
		{
			if(var->vt==(VT_ARRAY|VT_VARIANT))
			{
				long l,u;
				SAFEARRAY *psa=var->parray;
				SafeArrayGetLBound(psa,1,&l);
				SafeArrayGetUBound(psa,1,&u);
				if((u-l)>=1){
					_variant_t _cmd, _gid, _gname, _call_id;
					if((u-l)>=0)
						SafeArrayGetElement(psa,&l,&_cmd.GetVARIANT());
					l++;

					long cmd = (long) _cmd;
					long gid(0);
					VS_WideStr gname;
					VS_SimpleStr call_id;

					bool res = false;
					switch(cmd)
					{
					case MGC_CREATE_GROUP:
						if((u-l)>=0)
							SafeArrayGetElement(psa,&l,&_gname.GetVARIANT());
						l++;
						gname = (wchar_t*)(_bstr_t) _gname;
						if (!!gname)
							res = ManageGroups_CreateGroup(gname)!=0;
						break;
					case MGC_DELETE_GROUP:
						if((u-l)>=0)
							SafeArrayGetElement(psa,&l,&_gid.GetVARIANT());
						l++;
						gid = (long) _gid;
						if (gid)
							res = ManageGroups_DeleteGroup(gid)!=0;
						break;
					case MGC_RENAME_GROUP:
						if((u-l)>=0)
							SafeArrayGetElement(psa,&l,&_gid.GetVARIANT());
						l++;
						if((u-l)>=0)
							SafeArrayGetElement(psa,&l,&_gname.GetVARIANT());
						l++;
						gid = (long) _gid;
						gname = (wchar_t*)(_bstr_t) _gname;
						if (gid && !!gname)
							res = ManageGroups_RenameGroup(gid, gname)!=0;
						break;
					case MGC_ADD_USER:
						if((u-l)>=0)
							SafeArrayGetElement(psa,&l,&_gid.GetVARIANT());
						l++;
						if((u-l)>=0)
							SafeArrayGetElement(psa,&l,&_call_id.GetVARIANT());
						l++;
						gid = (long) _gid;
						call_id = (char*)(_bstr_t) _call_id;
						if (gid && !!call_id)
							res = ManageGroups_AddUser(gid, call_id)!=0;
						break;
					case MGC_DELETE_USER:
						if((u-l)>=0)
							SafeArrayGetElement(psa,&l,&_gid.GetVARIANT());
						l++;
						if((u-l)>=0)
							SafeArrayGetElement(psa,&l,&_call_id.GetVARIANT());
						l++;
						gid = (long) _gid;
						call_id = (char*)(_bstr_t) _call_id;
						if (gid && !!call_id)
							res = ManageGroups_DeleteUser(gid, call_id)!=0;
						break;
					default:
						return VS_INTERFACE_INTERNAL_ERROR;
						break;
					}
					return (res)? VS_INTERFACE_OK: VS_INTERFACE_INTERNAL_ERROR;
				}
			}
		} // VS_OPERATION==RUN_COMMAND
	} else if (_stricmp(pSection, "SetMyLStatus")==0) {
		if (VS_OPERATION==SET_PARAM)
		{
			SetMyLStatus((long)(_variant_t)var);
		}
	} else if (_stricmp(pSection, "ClearAllLStatuses")==0) {
		if (VS_OPERATION==SET_PARAM)
		{
			ClearAllLStatuses();
		}
	} else if (_stricmp(pSection, "ManageABPhones")==0) {
		bool res(false);
		VARIANT *vars = 0;
		int num = ExtractVars(vars, var);
		if (num > 4) {
			VS_Container rCnt;
			rCnt.AddValue(METHOD_PARAM, vars[0].pcVal);
			rCnt.AddValueI32(ADDRESSBOOK_PARAM, AB_PHONES);
			rCnt.AddValue(ID_PARAM, vars[1].pcVal);
			rCnt.AddValue(CALLID_PARAM, vars[2].pcVal);
			rCnt.AddValue(USERPHONE_PARAM, vars[3].pcVal);
			rCnt.AddValueI32(TYPE_PARAM, vars[4].lVal);

			res = ComposeSend(rCnt, ADDRESSBOOK_SRV) > 0;
		}
		if (num > 0)
			delete[] vars;
		return res ? VS_INTERFACE_OK :VS_INTERFACE_INTERNAL_ERROR;
	} else if (_stricmp(pSection, "AddContact")==0) {
		bool res(false);
		VARIANT *vars = 0;
		int num = ExtractVars(vars, var);
		if (num > 1) {
			VS_SimpleStr contact((char*)(_bstr_t)vars[0]);
			VS_WideStr dn((wchar_t*)(_bstr_t)vars[1]);
			DTRACE(VSTM_PRTCL, "AddContact, user %s, DN = %S", contact.m_str, dn.m_str);
			VS_Container rCnt;
			rCnt.AddValue(METHOD_PARAM, ADDTOADDRESSBOOK_METHOD);
			rCnt.AddValueI32(ADDRESSBOOK_PARAM, AB_COMMON);
			rCnt.AddValue(CALLID_PARAM, contact);
			rCnt.AddValue(DISPLAYNAME_PARAM, dn);

			res = ComposeSend(rCnt, ADDRESSBOOK_SRV) > 0;
		}
		if (num > 0)
			delete[] vars;
		return res ? VS_INTERFACE_OK :VS_INTERFACE_INTERNAL_ERROR;
	}
	else if (_stricmp(pSection, "GetChatMessage")==0) {
		if (VS_OPERATION==GET_PARAM) {

			VS_Container *cnt = m_MessContainers.GetList(*var);
			if (cnt) {
				_variant_t vars[10];

				int32_t type = MSG_NORMAL; cnt->GetValue(TYPE_PARAM, type);
				vars[0] = type;

				vars[1] = (char*)cnt->GetStrValueRef(FROM_PARAM);
				vars[2] = (char*)cnt->GetStrValueRef(MESSAGE_PARAM);
				vars[3] = (char*)cnt->GetStrValueRef(TO_PARAM);
				vars[4] = (char*)cnt->GetStrValueRef(DISPLAYNAME_PARAM);

				size_t size = 0;
				long long tt = 0;
				FILETIME *ft = (FILETIME *)cnt->GetBinValueRef(FILETIME_PARAM, size);
				if (ft && sizeof(FILETIME) == size) {
					LARGE_INTEGER date;
					date.HighPart = ft->dwHighDateTime;
					date.LowPart = ft->dwLowDateTime;
					date.QuadPart -= 11644473600000UL * 10000;
					date.QuadPart /= 10000000;
					tt = date.LowPart;
				} else tt = std::time(0);
				vars[5] = tt;

                vars[8] = (char*)cnt->GetStrValueRef(LINK_PARAM);
                vars[9] = (char*)cnt->GetStrValueRef(NAME_PARAM);

				if (CombineVars(var, vars, 10))
					return VS_INTERFACE_OK;
			}
		}
		return VS_INTERFACE_INTERNAL_ERROR;
	}
	else if (_stricmp(pSection, AUTH_BY_ECP_METHOD)==0) {
		bool res(false);
		if (VS_OPERATION==RUN_COMMAND) {		// start auth
			DTRACE(VSTM_PRTCL, "AuthByECP RUN_COMMAND");
			VS_Container rCnt;
			rCnt.AddValue(METHOD_PARAM, AUTH_BY_ECP_METHOD);
			res = ComposeSend(rCnt, AUTH_SRV) > 0;
		}else if (VS_OPERATION==GET_PARAM) {	// todo(kt): return ticketId, ticketBody
			DTRACE(VSTM_PRTCL, "AuthByECP GET_PARAM index=%d", (int) *var);
			VS_Container *cnt = m_AuthECPContainers.GetList(*var);
			if (cnt) {
				_variant_t vars[2];
				vars[0] = (char*)cnt->GetStrValueRef(TICKET_ID_PARAM);
				vars[1] = (char*)cnt->GetStrValueRef(TICKET_BODY_PARAM);
				res = CombineVars(var, vars, 2);
			}
		}else if (VS_OPERATION==SET_PARAM) {
			VARIANT *vars = 0;
			int num = ExtractVars(vars, var);
			if (num > 1) {
				sudis::ticketId ticketId((char*)(_bstr_t)vars[0]);
				sudis::signedTicketBody signedTicketBody((char*)(_bstr_t)vars[1]);
				DTRACE(VSTM_PRTCL, "AuthByECP SET_PARAM, ticketId=%s, signedTicketBody=%s", ticketId.c_str(), signedTicketBody.c_str());
				VS_Container rCnt;
				rCnt.AddValue(METHOD_PARAM, AUTH_BY_ECP_METHOD);
				rCnt.AddValue(TICKET_ID_PARAM, ticketId);
				rCnt.AddValue(SIGNED_TICKET_BODY_PARAM, signedTicketBody);
				rCnt.AddValue(APPID_PARAM, m_AppId);
				res = ComposeSend(rCnt, AUTH_SRV) > 0;
			}
			if (num > 0)
				delete[] vars;
		}
		return res ? VS_INTERFACE_OK :VS_INTERFACE_INTERNAL_ERROR;
	}
	else if (_stricmp(pSection, "ConferenceSettings") == 0) {
		bool res = false;
		if (VS_OPERATION == RUN_COMMAND) {
			_variant_t vars[2];
			if (m_Status.dwStatus&STATUS_CONFERENCE) {
				vars[0] = long(m_Status.CurrConfInfo->UCall != -1); // unconditional call to Podium in role conf
				vars[1] = long(m_Status.CurrConfInfo->PView != -1); // view mode in role conf
			}
			else {
				vars[0] = 0;
				vars[1] = 0;
			}
			res = CombineVars(var, vars, 2);
		}
		else if (VS_OPERATION == GET_PARAM) {
			_variant_t vars[2];
			if (m_Status.dwStatus&STATUS_CONFERENCE) {
				vars[0] = m_Status.CurrConfInfo->UCall;
				vars[1] = m_Status.CurrConfInfo->PView;
			}
			else {
				vars[0] = 0;
				vars[1] = 0;
			}
			res = CombineVars(var, vars, 2);
		}
		else if (VS_OPERATION == SET_PARAM) {
			VARIANT *vars = 0;
			int num = ExtractVars(vars, var);
			if (num > 1) {
				if (m_Status.dwStatus&STATUS_CONFERENCE) {

					if (m_Status.CurrConfInfo->UCall != -1)
						m_Status.CurrConfInfo->UCall = vars[0].lVal;
					if (m_Status.CurrConfInfo->PView != -1)
						m_Status.CurrConfInfo->PView = vars[1].lVal;

					VS_Container rCnt;
					rCnt.AddValue(METHOD_PARAM, UPDATESETTINGS_METHOD);
					rCnt.AddValue(CONFERENCE_PARAM, m_Status.CurrConfInfo->Conference);
					rCnt.AddValue(USERNAME_PARAM, m_Status.MyInfo.UserName);
					rCnt.AddValueI32(UCALL_PARAM, m_Status.CurrConfInfo->UCall);
					rCnt.AddValueI32(PVIEW_PARAM, m_Status.CurrConfInfo->PView);
					res = ComposeSend(rCnt, CONFERENCE_SRV, ReturnBroker(m_Status.CurrConfInfo->Conference)) > 0;
				}
			}
			if (num > 0)
				delete[] vars;
		}
		return res ? VS_INTERFACE_OK : VS_INTERFACE_INTERNAL_ERROR;
	}
	else if (_stricmp(pSection, "GetProperties") == 0) {
		if (VS_OPERATION == GET_PARAM) {
			VS_AutoLock lock(this);
			VS_SimpleStr Name = vs::StrFromVariantT(var);
			const char *prop = m_UserProp.GetStrValueRef(Name.m_str);
			if (!prop || !*prop)
				prop = m_PropContainer.GetStrValueRef(Name.m_str);
			if (prop) {
				*var = prop;
				return VS_INTERFACE_OK;
			}
		}
		return VS_INTERFACE_INTERNAL_ERROR;
	}
	else if (_stricmp(pSection, "FECC") == 0) {
		if (VS_OPERATION == GET_PARAM) {
			VS_Container *cnt = m_FECC_Containers.GetList((int)*var);
			if (cnt) {
				m_result_cnt = std::make_unique<VS_ContainerData>(*cnt);
				*var = (int)m_result_cnt.get();
			}
			else
				*var = (int)0;
			return VS_INTERFACE_OK;
		}
		else if (VS_OPERATION == RUN_COMMAND) {
			bool res = false;
			VARIANT *vars = 0;
			int num = ExtractVars(vars, var);
			if (num > 3) {
				long type = vars[0].lVal;
				VS_Container rCnt;
				rCnt.AddValue(METHOD_PARAM, FECC_METHOD);
				rCnt.AddValueI32(TYPE_PARAM, type);
				rCnt.AddValue(FROM_PARAM, (char*)(_bstr_t)vars[1]);
				rCnt.AddValue(TO_PARAM, (char*)(_bstr_t)vars[2]);
				if (num > 4) {
					if (type == (long)eFeccRequestType::SAVE_PRESET ||
						type == (long)eFeccRequestType::USE_PRESET)
						rCnt.AddValueI32(PRESET_NUM_PARAM, vars[3].lVal);
					else if (type == (long)eFeccRequestType::SET_STATE ||
						type == (long)eFeccRequestType::MY_STATE)
						rCnt.AddValueI32(FECC_STATE_PARAM, vars[3].lVal);
				}
				res = ComposeSend(rCnt, CONFERENCE_SRV, ReturnBroker(m_Status.CurrConfInfo->Conference)) > 0;
			}
			if (num > 0)
				delete[] vars;
			return res ? VS_INTERFACE_OK : VS_INTERFACE_INTERNAL_ERROR;
		}
	}

	return VS_INTERFACE_NO_FUNCTION;
}
/********************************************/

