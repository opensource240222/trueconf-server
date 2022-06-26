/*
* $Revision: 75 $
* $History: VS_ConferenceService.cpp $
 *
 * *****************  Version 75  *****************
 * User: Sanufriev    Date: 31.07.12   Time: 15:14
 * Updated in $/VSNA/Servers/AppServer/Services
 * - fix svc spatial for old clients
 *
 * *****************  Version 74  *****************
 * User: Mushakov     Date: 19.07.12   Time: 18:42
 * Updated in $/VSNA/Servers/AppServer/Services
 * - Reading appProp from regestry VS_REG_STRING_VT (not
 * VS_REG_WSTRING_VT) #12701
 *
 * *****************  Version 73  *****************
 * User: Mushakov     Date: 17.07.12   Time: 23:27
 * Updated in $/VSNA/Servers/AppServer/Services
 * - put checkSession to confRestriction
 *
 * *****************  Version 72  *****************
 * User: Mushakov     Date: 17.07.12   Time: 23:09
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - LoginConfigurator() was removed
 * - messages from configurator are handled by SessionID
 * - fix TransportMessage::IsFromServer()
 *
 * *****************  Version 71  *****************
 * User: Ktrushnikov  Date: 14.05.12   Time: 18:24
 * Updated in $/VSNA/Servers/AppServer/Services
 * - dont ban CT_TRANSCODER when kicked from multiconf (cause need to ban
 * #sip, not #trans)
 *
 * *****************  Version 70  *****************
 * User: Ktrushnikov  Date: 9.04.12    Time: 16:59
 * Updated in $/VSNA/Servers/AppServer/Services
 * #11423: reject at invite to UDP conf
 *
 * *****************  Version 69  *****************
 * User: Smirnov      Date: 20.03.12   Time: 18:20
 * Updated in $/VSNA/Servers/AppServer/Services
 * - fix for bug#10455
 *
 * *****************  Version 68  *****************
 * User: Ktrushnikov  Date: 5.02.12    Time: 15:58
 * Updated in $/VSNA/Servers/AppServer/Services
 * - IsKickUser(): check not only steam_id, but named_conf or VCS regsitry
 * conf_id
 * - if VCS user kicked from named_conf, send REJECT response directly to
 * user (not to BS)
 *
 * *****************  Version 67  *****************
 * User: Smirnov      Date: 11.11.11   Time: 17:32
 * Updated in $/VSNA/Servers/AppServer/Services
 * - added stream crypter in stream router
 * - bugfix#9470
 *
 * *****************  Version 66  *****************
 * User: Smirnov      Date: 17.10.11   Time: 21:57
 * Updated in $/VSNA/Servers/AppServer/Services
 * - add service name in config
 *
 * *****************  Version 65  *****************
 * User: Ktrushnikov  Date: 10.08.11   Time: 14:51
 * Updated in $/VSNA/Servers/AppServer/Services
 * #9601 use GetAnyBSByDomain() instead of GetFirstBS()
 *
 * *****************  Version 64  *****************
 * User: Ktrushnikov  Date: 20.06.11   Time: 18:45
 * Updated in $/VSNA/Servers/AppServer/Services
 * #8222
 * - email notifications fix
 *
 * *****************  Version 63  *****************
 * User: Mushakov     Date: 20.04.11   Time: 19:46
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - DeleteConf from manager
 *
 * *****************  Version 62  *****************
 * User: Ktrushnikov  Date: 16.09.10   Time: 15:17
 * Updated in $/VSNA/Servers/AppServer/Services
 * Arch 3.1 Conf Loggin duplicates remove
 * - RS store BS server of user
 * - ConfLog: uses BS got from RS to send logs
 * - ConfLog: dprint loggin added
 *
 * *****************  Version 61  *****************
 * User: Ktrushnikov  Date: 12.08.10   Time: 18:36
 * Updated in $/VSNA/Servers/AppServer/Services
 * [#7592]
 * - VS_ConfKick interface for store kicked users and deny them to connect
 * to conference in server
 *
 * *****************  Version 60  *****************
 * User: Ktrushnikov  Date: 5.08.10    Time: 15:01
 * Updated in $/VSNA/Servers/AppServer/Services
 * - Logging in p2p conferences
 *
 * *****************  Version 59  *****************
 * User: Smirnov      Date: 30.06.10   Time: 16:50
 * Updated in $/VSNA/Servers/AppServer/Services
 * delete conference on owner hungup (#7493)
 *
 * *****************  Version 58  *****************
 * User: Ktrushnikov  Date: 24.06.10   Time: 16:58
 * Updated in $/VSNA/Servers/AppServer/Services
 * [#7250]
 * - support rights: CREATEMULTI, CALL
 *
 * *****************  Version 57  *****************
 * User: Mushakov     Date: 21.06.10   Time: 14:39
 * Updated in $/VSNA/Servers/AppServer/Services
 * - VS_ConfLog
 *
 * *****************  Version 56  *****************
 * User: Mushakov     Date: 15.06.10   Time: 15:32
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - sharding
 *
 * *****************  Version 55  *****************
 * User: Ktrushnikov  Date: 14.04.10   Time: 17:57
 * Updated in $/VSNA/Servers/AppServer/Services
 * - dprint3 log added
 *
 * *****************  Version 54  *****************
 * User: Ktrushnikov  Date: 23.02.10   Time: 14:40
 * Updated in $/VSNA/Servers/AppServer/Services
 * VCS:
 * - Services added: SMTP_MAILER, LOCATOR, LOG (two last to make work
 * smtp_mailer)
 * - OnUserChange(): changes come from configurator vs_bc.dll
 * vs_bc.dll:
 * - fix params (broker) where to send updates
 *
 * *****************  Version 53  *****************
 * User: Smirnov      Date: 15.02.10   Time: 20:02
 * Updated in $/VSNA/Servers/AppServer/Services
 * - enh #6964
 *
 * *****************  Version 52  *****************
 * User: Smirnov      Date: 15.02.10   Time: 18:50
 * Updated in $/VSNA/Servers/AppServer/Services
 * - licence restrictions reorganization
 * - SECUREBEGIN_A temporally removed
 *
 * *****************  Version 51  *****************
 * User: Smirnov      Date: 9.02.10    Time: 19:14
 * Updated in $/VSNA/Servers/AppServer/Services
 * - ssl streams for private conferences
 *
 * *****************  Version 50  *****************
 * User: Mushakov     Date: 8.02.10    Time: 18:04
 * Updated in $/VSNA/Servers/AppServer/Services
 * - some commemts removed
 * - readingtrial minutes added
 *
 * *****************  Version 49  *****************
 * User: Mushakov     Date: 11.12.09   Time: 17:50
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - remive VCS_BUILD from VS_Storage
 *
 * *****************  Version 48  *****************
 * User: Mushakov     Date: 8.12.09    Time: 20:23
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - ConfRestriction Interface added
 *
 * *****************  Version 47  *****************
 * User: Mushakov     Date: 4.11.09    Time: 21:13
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - new names
 *
 * *****************  Version 46  *****************
 * User: Mushakov     Date: 4.11.09    Time: 20:07
 * Updated in $/VSNA/Servers/AppServer/Services
 * - quick update license
 * - #regs suffix added
 *
 * *****************  Version 45  *****************
 * User: Smirnov      Date: 28.10.09   Time: 15:27
 * Updated in $/VSNA/Servers/AppServer/Services
 * - repaired public conf for old users
 * - rating threashold from registry
 *
 * *****************  Version 44  *****************
 * User: Smirnov      Date: 1.10.09    Time: 15:34
 * Updated in $/VSNA/Servers/AppServer/Services
 * - autoblock users with bad rating (bugfix#6354)
 *
 * *****************  Version 43  *****************
 * User: Smirnov      Date: 11.08.09   Time: 17:42
 * Updated in $/VSNA/Servers/AppServer/Services
 * - fix reject problems
 *
 * *****************  Version 42  *****************
 * User: Smirnov      Date: 10.08.09   Time: 12:37
 * Updated in $/VSNA/Servers/AppServer/Services
 * - added reject causes
 *
 * *****************  Version 41  *****************
 * User: Mushakov     Date: 20.02.09   Time: 19:26
 * Updated in $/VSNA/Servers/AppServer/Services
 * - AppId added in ParticipantDescr (invite, join log)
 * - 2 DBs supported
 *
 * *****************  Version 40  *****************
 * User: Smirnov      Date: 24.10.08   Time: 21:12
 * Updated in $/VSNA/Servers/AppServer/Services
 * - transport ping increased
 * - loging in conference corrected
 *
 * *****************  Version 39  *****************
 * User: Smirnov      Date: 16.10.08   Time: 20:26
 * Updated in $/VSNA/Servers/AppServer/Services
 * - conference logging improved
 *
 * *****************  Version 38  *****************
 * User: Smirnov      Date: 13.10.08   Time: 21:37
 * Updated in $/VSNA/Servers/AppServer/Services
 * - some cleaning
 *
 * *****************  Version 37  *****************
 * User: Smirnov      Date: 3.10.08    Time: 18:40
 * Updated in $/VSNA/Servers/AppServer/Services
 * - removed invite mails sent at end of conference
 *
 * *****************  Version 36  *****************
 * User: Smirnov      Date: 3.10.08    Time: 15:09
 * Updated in $/VSNA/Servers/AppServer/Services
 * - conference messages
 *
 * *****************  Version 35  *****************
 * User: Smirnov      Date: 27.09.08   Time: 21:43
 * Updated in $/VSNA/Servers/AppServer/Services
 * - removed groupconference storage
 * - removed unnecessary conference logging
 * - create conference and join rewrited
 *
 * *****************  Version 34  *****************
 * User: Mushakov     Date: 22.08.08   Time: 12:18
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - chat message about closing multi conf by owner
 *  - mailing about missing of invate to multiconf
 *  - Calls logging fixed
 *
 * *****************  Version 33  *****************
 * User: Mushakov     Date: 25.07.08   Time: 16:18
 * Updated in $/VSNA/Servers/AppServer/Services
 * - Incomming calls logging added in point-to-point confs (2 join loggs)
 *
 * *****************  Version 32  *****************
 * User: Mushakov     Date: 25.07.08   Time: 14:51
 * Updated in $/VSNA/Servers/AppServer/Services
 * - logging app_ID added
 * - logging multi_conf
 * - bug 4602 fixed
 *
 * *****************  Version 31  *****************
 * User: Smirnov      Date: 25.04.08   Time: 14:36
 * Updated in $/VSNA/Servers/AppServer/Services
 * - removing mail message for group conference
 *
 * *****************  Version 30  *****************
 * User: Smirnov      Date: 7.04.08    Time: 16:55
 * Updated in $/VSNA/Servers/AppServer/Services
 * - bugfix with direct connect
 *
 * *****************  Version 29  *****************
 * User: Smirnov      Date: 27.03.08   Time: 18:18
 * Updated in $/VSNA/Servers/AppServer/Services
 * - alias problem fix
 *
 * *****************  Version 28  *****************
 * User: Ktrushnikov  Date: 15.03.08   Time: 17:11
 * Updated in $/VSNA/Servers/AppServer/Services
 * - send mail changed: AS just send request to BS::LOG_SRV from 2 places
 *
 * *****************  Version 27  *****************
 * User: Ktrushnikov  Date: 29.02.08   Time: 21:27
 * Updated in $/VSNA/Servers/AppServer/Services
 * - Same deadlock fix as in VS_MultiConfService
 *
 * *****************  Version 26  *****************
 * User: Mushakov     Date: 19.02.08   Time: 16:31
 * Updated in $/VSNA/Servers/AppServer/Services
 * - Conferences Logging added
 *
 * *****************  Version 25  *****************
 * User: Smirnov      Date: 18.02.08   Time: 20:41
 * Updated in $/VSNA/Servers/AppServer/Services
 * - repaired PostReply for tasks
 *
 * *****************  Version 24  *****************
 * User: Smirnov      Date: 18.02.08   Time: 16:43
 * Updated in $/VSNA/Servers/AppServer/Services
 * - added net config for servers
 *
 * *****************  Version 23  *****************
 * User: Mushakov     Date: 15.02.08   Time: 20:21
 * Updated in $/VSNA/Servers/AppServer/Services
 * sending missed call mail and invate mail added
 *
 * *****************  Version 22  *****************
 * User: Mushakov     Date: 12.02.08   Time: 19:20
 * Updated in $/VSNA/Servers/AppServer/Services
 * SeApptProperties realized
 *
 * *****************  Version 21  *****************
 * User: Smirnov      Date: 11.02.08   Time: 21:02
 * Updated in $/VSNA/Servers/AppServer/Services
 * - add allias processing
 *
 * *****************  Version 20  *****************
 * User: Smirnov      Date: 11.02.08   Time: 18:12
 * Updated in $/VSNA/Servers/AppServer/Services
 * - calls between servers
 *
 * *****************  Version 19  *****************
 * User: Stass        Date: 8.02.08    Time: 20:19
 * Updated in $/VSNA/Servers/AppServer/Services
 * rename
 *
 * *****************  Version 18  *****************
 * User: Mushakov     Date: 7.02.08    Time: 20:22
 * Updated in $/VSNA/Servers/AppServer/Services
 * Missed call handle changed
 *
 * *****************  Version 17  *****************
 * User: Mushakov     Date: 6.02.08    Time: 19:04
 * Updated in $/VSNA/Servers/AppServer/Services
 * sending meaage to logservice (baseserver) added
 *
 * *****************  Version 16  *****************
 * User: Smirnov      Date: 5.02.08    Time: 16:42
 * Updated in $/VSNA/Servers/AppServer/Services
 * - direct conf half-repaired
 *
 * *****************  Version 15  *****************
 * User: Smirnov      Date: 1.02.08    Time: 20:06
 * Updated in $/VSNA/Servers/AppServer/Services
 * - MULTI_CONF srv removed
 * - making message in client simplified
 * - default constants for dicsovery
 * - read onlly discovered endpoints for servers
 *
 * *****************  Version 14  *****************
 * User: Dront78      Date: 23.01.08   Time: 17:51
 * Updated in $/VSNA/Servers/AppServer/Services
 * _malloca redefinition warning fixed.
 *
 * *****************  Version 13  *****************
 * User: Smirnov      Date: 10.01.08   Time: 19:51
 * Updated in $/VSNA/Servers/AppServer/Services
 * - Subscribe users in client
 * - many fixes
 *
 * *****************  Version 12  *****************
 * User: Dront78      Date: 28.12.07   Time: 20:08
 * Updated in $/VSNA/Servers/AppServer/Services
 * bool Init( const char *our_endpoint, const char *our_service, const
 * bool permittedAll = false ); added.
 *
 * *****************  Version 11  *****************
 * User: Stass        Date: 12.12.07   Time: 20:40
 * Updated in $/VSNA/Servers/AppServer/Services
 * fixed init
 *
 * *****************  Version 10  *****************
 * User: Stass        Date: 30.11.07   Time: 20:36
 * Updated in $/VSNA/Servers/AppServer/Services
 *
 * *****************  Version 9  *****************
 * User: Stass        Date: 28.11.07   Time: 18:34
 * Updated in $/VSNA/Servers/AppServer/Services
 * removed warning
 *
 * *****************  Version 8  *****************
 * User: Stass        Date: 27.11.07   Time: 18:54
 * Updated in $/VSNA/Servers/AppServer/Services
 *
 * *****************  Version 7  *****************
 * User: Stass        Date: 26.11.07   Time: 22:02
 * Updated in $/VSNA/Servers/AppServer/Services
 * app server replacements done
 *
 * *****************  Version 6  *****************
 * User: Stass        Date: 26.11.07   Time: 17:51
 * Updated in $/VSNA/Servers/AppServer/Services
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 17.10.07   Time: 17:43
 * Updated in $/VS2005/Servers/MediaBroker/BrokerServices
 * - bugfix #3432
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 9.07.07    Time: 20:06
 * Updated in $/VS2005/Servers/MediaBroker/BrokerServices
 * - broadcast status added
 * - some wishes for gui for device status
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 5.07.07    Time: 20:05
 * Updated in $/VS2005/Servers/MediaBroker/BrokerServices
 * - write role Leader in participant list for host in not rolled group
 * conference
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 4.07.07    Time: 19:25
 * Updated in $/VS2005/Servers/MediaBroker/BrokerServices
 * - device statuses added
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 4.05.07    Time: 14:20
 * Updated in $/VS2005/Servers/MediaBroker/BrokerServices
 * - bugfix#2198
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 23.04.07   Time: 17:05
 * Updated in $/VS2005/Servers/MediaBroker/BrokerServices
 * - roled conference added and checked
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 20.04.07   Time: 20:50
 * Updated in $/VS2005/Servers/MediaBroker/BrokerServices
 * - UMC - added roles
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 27.03.07   Time: 18:22
 * Updated in $/VS2005/Servers/MediaBroker/BrokerServices
 * - restrict bitrate value = -1 is supported
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 5.02.07    Time: 21:31
 * Updated in $/VS2005/Servers/MediaBroker/BrokerServices
 * - project configuration
 * - depricated functions warnings suppressed
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Servers/MediaBroker/BrokerServices
 *
 * ************************************************/
#include "VS_ConferenceService.h"

#include "ServerServices/Common.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_RcvFunc.h"
#include "std/cpplib/VS_ClientCaps.h"
#include "net/EndpointRegistry.h"

#include "VS_AppServerData.h"
#include "ServerServices/VS_ReadLicense.h"
#include "std/cpplib/VS_IntConv.h"
#include "std/cpplib/VS_CallIDUtils.h"
#include "tools/Server/VS_ServerComponentsInterface.h"
#include "streams/Router/Router.h"
#include "streams/Router/DefaultBuffer.h"
#include "streams/Relay/VS_StreamsRelay.h"
#include "TransceiverLib/VS_ConfRecorderModuleCtrl.h"
#include "TransceiverLib/VS_RTSPBroadcastModuleCtrl.h"
#include "TransceiverLib/VS_ConfControlModule.h"
#include "std/cpplib/curl_deleters.h"
#include "std/cpplib/VS_Utils.h"
#include "TransceiverLib/TransceiversPoolInterface.h"
#include "TransceiverLib/VS_TransceiverProxy.h"
#include "TrueGateway/TransportTools.h"

#include <curl/curl.h>

#include <stdio.h>
#include <thread>
#include <boost/make_shared.hpp>

#define DEBUG_CURRENT_MODULE VS_DM_CONFS

typedef stream::DefaultBuffer vs_public_conf_buf;
//typedef VS_StreamPublicBuffer vs_public_conf_buf;

const int VS_ConferenceService::TIME_EXP_CREATE = 100;
const int VS_ConferenceService::TIME_EXP_INVITE = 100;
const int VS_ConferenceService::TIME_EXP_ACCEPT = 60 * 60 * 24 * 10; // 10 days
const int VS_ConferenceService::TIME_EXP_END    = 30;

////////////////////////////////////////////////////////////////////////////////
// Global
////////////////////////////////////////////////////////////////////////////////
VS_ConferenceService *g_conferenceService = 0;

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
int VS_CheckFmt(VS_BinBuff &to, VS_BinBuff &from, int fltr) {
	VS_MediaFormat fmt;
	VS_ClientCaps toCaps(to.Buffer(), to.Size());
	VS_ClientCaps fromCaps(from.Buffer(), from.Size());
	fmt = toCaps.GetFmtFromToMe(fromCaps);
	if (!fmt.IsAudioValid())
		fltr&=~VS_RcvFunc::FLTR_RCV_AUDIO;
	if (!fmt.IsVideoValid())
		fltr&=~VS_RcvFunc::FLTR_RCV_VIDEO;
	return fltr;
}

int VS_SetRcvDevStatus(long &dvs, int s, int d, int type) {
	int abs1 = VS_RcvFunc::FLTR_RCV_AUDIO & s;
	int abs2 = VS_RcvFunc::FLTR_RCV_AUDIO & d;
	int vbs1 = VS_RcvFunc::FLTR_RCV_VIDEO & s;
	int vbs2 = VS_RcvFunc::FLTR_RCV_VIDEO & d;
	if		(type==0) {
		if (abs1 && !abs2)
			dvs|=DVS_RCV_CONFRULE_NOTALOW;
		if (vbs1 && !vbs2)
			dvs|=(DVS_RCV_CONFRULE_NOTALOW<<16);
	}
	else if (type==1) {
		if (abs1 && !abs2)
			dvs|=DVS_RCV_DECODER_NOTPRESENT;
		if (vbs1 && !vbs2)
			dvs|=(DVS_RCV_DECODER_NOTPRESENT<<16);
	}
	else if (type==2) {
		dvs|=(DVS_RCV_TURNEDOF_BYBAND<<16);
	}
	if (abs2)
		dvs&=~DVS_MASK_RCV;
	else if ((dvs&DVS_MASK_RCV)==0)
		dvs|=DVS_RCV_PAUSED;
	if (vbs2)
		dvs&=~(DVS_MASK_RCV<<16);
	else if ((dvs&DVS_MASK_RCV<<16)==0)
		dvs|=(DVS_RCV_PAUSED<<16);
	return 0;
}

bool VS_CheckDynChange(VS_BinBuff &to)
{
	VS_ClientCaps toCaps(to.Buffer(), to.Size());
	return !!(toCaps.GetVideoRcv() & VSCC_VIDEO_DYNCHANGE);
}

unsigned char VS_GetLevelPerformance(VS_BinBuff &to)
{
	VS_ClientCaps toCaps(to.Buffer(), to.Size());
	return (unsigned char)(toCaps.GetLevelGroup() & 0x000000ff);
}

int VS_GetSndMBps(VS_BinBuff &from)
{
	VS_MediaFormat fmt;
	VS_ClientCaps fromCaps(from.Buffer(), from.Size());
	fromCaps.GetMediaFormat(fmt);
	return fmt.GetMBps();
}

int VS_GetSndFrameSizeMB(VS_BinBuff &from)
{
	VS_MediaFormat fmt;
	VS_ClientCaps fromCaps(from.Buffer(), from.Size());
	fromCaps.GetMediaFormat(fmt);
	return fmt.GetFrameSizeMB();
}

////////////////////////////////////////////////////////////////////////////////
// Constructor/Destructor
////////////////////////////////////////////////////////////////////////////////
int g_rating_th = 200;

VS_ConferenceService::VS_ConferenceService()
	: m_conf_check_time(0)
	, m_part_check_time(0)
	, m_invite_check_time(0)
	, m_sr(0)
	, m_pios(0)
{
	m_TimeInterval = std::chrono::seconds(1);
	VS_RegistryKey    cfg(false, CONFIGURATION_KEY);
	cfg.GetValue(&g_rating_th, 4, VS_REG_INTEGER_VT, "RatingTh");
	if (g_rating_th < 0)
		g_rating_th = 200;

}

VS_ConferenceService::~VS_ConferenceService(void)
{
}

bool VS_ConferenceService::Init( const char *our_endpoint, const char *our_service, const bool permittedAll )
{
	if (g_conferenceService != 0)	return false;	// No more than one copy
	if (m_sr == 0)					return false;	// Can't run without Stream Router

	assert(m_transceiversPool.lock() != nullptr);
	if (m_transceiversPool.lock() == nullptr)	// tranceivers pool must be set for this moment
		return false;

	g_conferenceService = this;
	m_streamsRelay = std::make_shared<vs_relaymodule::VS_StreamsRelay>(m_sr);
	assert(m_pios != nullptr);
	m_confWriterModule = std::make_shared<VS_ConfRecorderModuleCtrl>(*m_pios);
	m_signal_RecordStartInfo = m_confWriterModule->ConnectRecordStartInfo(VS_ConfRecorderModuleCtrl::signalRecordStartInfo::slot_type(&VS_ConferenceService::onRecordStartInfo, this, _1, _2, _3));
	m_signal_RecordStopInfo = m_confWriterModule->ConnectRecordStopInfo(VS_ConfRecorderModuleCtrl::signalRecordStopInfo::slot_type(&VS_ConferenceService::onRecordStopInfo, this, _1, _2, _3));
	m_RTSPBroadcastModule = std::make_shared<VS_RTSPBroadcastModuleCtrl>();
	m_presenceService->AddListener(VS_AutoInviteService::AUTOINVITE_PRESENCE_PARAM, this);

	return true;
}
void VS_ConferenceService::SetConfRestrict(const boost::shared_ptr<VS_ConfRestrictInterface>& confRestrict)
{
	m_confRestriction = confRestrict;
}
void VS_ConferenceService::SetIOservice(boost::asio::io_service & ios)
{
	m_pios = &ios;
}

void VS_ConferenceService::SetTransceiversPool(const std::shared_ptr<ts::IPool>& pool)
{
	m_transceiversPool = pool;
}

void VS_ConferenceService::AsyncDestroy()
{
	m_signal_RecordStartInfo.disconnect();
	m_signal_RecordStopInfo.disconnect();
	m_keyReqConn.disconnect();
	m_bitrateRestricConn.disconnect();
	m_sr->Stop();
	std::this_thread::yield();
	VS_ConferenceDescription *cd;
	int NumOnConf = 0, j;
	if ( (NumOnConf = g_storage->FindConferences(cd, "ALL"))>0) {
		for (j = 0; j< NumOnConf; j++){
			RemoveConference_Event(cd[j], cd[j].TERMINATED_BY_SERVER_RESTART);
		}
		delete[] cd;
	}
	std::this_thread::yield();
}

////////////////////////////////////////////////////////////////////////////////
// Main service
////////////////////////////////////////////////////////////////////////////////
bool VS_ConferenceService::Processing( std::unique_ptr<VS_RouterMessage>&&/*recvMess*/)
{
	return true;
}


////////////////////////////////////////////////////////////////////////////////
// CREATECONFERENCE_METHOD(DURATION_PARAM, MAXPARTISIPANTS_PARAM, TYPE_PARAM)
////////////////////////////////////////////////////////////////////////////////
void VS_ConferenceService::CreateConference_Method(long duration, long maxParticipants, long type, VS_BinBuff &caps, const char* password, const std::string& transcReserveToken)
{
	const auto user_id = m_recvMess->SrcUser_sv();
	VS_UserData ud;
	if (!g_storage->FindUser(user_id, ud))
		return;
	if ((ud.m_rights & VS_UserData::UR_COMM_CALL) == 0)
	{
		VS_Container rCnt;
		rCnt.AddValue(METHOD_PARAM, CONFERENCECREATED_METHOD);
		rCnt.AddValueI32(RESULT_PARAM, CREATE_ACCESS_DENIED);
		PostReply(rCnt);
		return;
	}
	//VS_SimpleStr	home_BS = ud.m_homeServer;

	int32_t result = CREATE_ACCESS_DENIED;
	VS_ParticipantDescription pd;
	VS_ConferenceDescription cd;
	// set common params and clear variables
	cd.SetTimeExp(TIME_EXP_CREATE);
	cd.m_state = cd.CONFERENCE_CREATED;
	cd.m_MaxParticipants = maxParticipants;
	cd.m_type = type;
	cd.m_password = password;
	cd.m_appID = ud.m_appID;
	// ------------------------------------------------------------------------------------
	if (type==CT_HALF_PRIVATE)
		cd.m_party = StringViewToSimpleStr(user_id);
	else
		cd.m_owner = StringViewToSimpleStr(user_id);

	cd.m_need_record = m_confRestriction->DoWriteConference(cd);

	int error_code = g_storage->InsertConference(cd);
	if (error_code ==0 ) {
		if (!transcReserveToken.empty()) {
			assert(cd.m_name.m_str != nullptr);
			gw::ConnectReservedProxyToConf(m_transceiversPool, transcReserveToken, cd.m_name.m_str);
		}
		if (type==CT_PUBLIC || type==CT_VIPPUBLIC || type==CT_BROADCAST) {
			{ // remove fully Access Denied
				VS_ConferenceDescription cd2;
				if (g_storage->FindConferenceByUser(user_id, cd2))
					RemoveConference_Event(cd2, cd2.TERMINATED_BY_CREATE_NEW);
			}
			// Now we have Conference (cd) and 1 Participant
			pd.m_conf_id = cd.m_name;
			pd.m_user_id = cd.m_owner;
			pd.m_type = pd.PUBLIC_HOST;
			pd.m_version = ud.m_protocolVersion;
			pd.m_appID = ud.m_appID;
			pd.m_caps = caps;
			pd.m_IsOperatorByGroups = m_confRestriction->IsOperator(pd.m_user_id);
			error_code = g_storage->AddParticipant(pd);

			if (error_code==0) {
				if (m_sr->CreateConference(cd.m_name.m_str, (VS_Conference_Type)cd.m_type, 0, cd.m_MaxParticipants)) {
					vs_public_conf_buf* strBuff = new vs_public_conf_buf;
					if (m_sr->AddParticipant(cd.m_name.m_str, cd.m_owner.m_str, strBuff)) {

						if(caps.IsValid())
						{
							m_sr->SetParticipantCaps(cd.m_name.m_str, cd.m_owner.m_str, caps.Buffer(), caps.Size());
						}

						cd.m_state = cd.PARTICIPANT_ACCEPTED;
						cd.SetTimeExp(TIME_EXP_ACCEPT);
						m_confRestriction->UpdateConfDuration(cd);

						g_storage->UpdateConference(cd);
						result = CONFERENCE_CREATED_OK;

						VS_UserPresence_Status status=USER_PUBLIC;
						if(type==CT_VIPPUBLIC || !!cd.m_password)
							status=USER_VIPPUBLIC;
					}
					else {
						if (strBuff) delete strBuff;
						result = NO_ENOUGH_RESOURCES_FOR_CONFERENCE;
					}
				} else result = NO_ENOUGH_RESOURCES_FOR_CONFERENCE;
			}
			else {
				switch(error_code)
				{
				case VSS_CONF_NOT_VALID:		result = CREATE_ACCESS_DENIED; break;
				case VSS_CONF_ACCESS_DENIED:	result = CREATE_ACCESS_DENIED; break;
				case VSS_CONF_NO_MONEY:			result = CREATE_HAVE_NO_MONEY;	break;
				default:						result = NO_ENOUGH_RESOURCES_FOR_CONFERENCE;	break;
				}
				cd.m_logCause = cd.TERMINATED_BY_PART_ACCDEN;
				g_storage->DeleteConference(cd);
			}
		}
		else
		{
			result = CONFERENCE_CREATED_OK;
		}
	}
	else {
		if (error_code == VSS_CONF_LIC_LIMITED)
			result = VSS_CONF_LIC_LIMITED;
		else
			result = NO_ENOUGH_RESOURCES_FOR_CONFERENCE;
	}

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, CONFERENCECREATED_METHOD);
	rCnt.AddValueI32(RESULT_PARAM, result);
	if (result == CONFERENCE_CREATED_OK)
		rCnt.AddValue(NAME_PARAM, cd.m_name);
	PostReply(rCnt);

	dstream3 << "Create Conference: user " << user_id << ", res=" << result << ", name=<" << cd.m_name.m_str << ">";

	if (result == CONFERENCE_CREATED_OK) {
		// log start
		LogConferenceStart(cd);

		if (type==CT_PUBLIC || type==CT_VIPPUBLIC || type==CT_BROADCAST) {
			// LOG
			LogParticipantJoin(pd);

			VS_Container rCnt;
			rCnt.AddValue(METHOD_PARAM, ACCEPT_METHOD);
			rCnt.AddValue(CONFERENCE_PARAM, cd.m_name);
			rCnt.AddValueI32(DIRECTCONNECT_PARAM, NO_DIRECT_CONNECT);
			PostReply(rCnt);
			SendPartsList(cd);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// DELETECONFERENCE_METHOD(NAME_PARAM)
////////////////////////////////////////////////////////////////////////////////
void VS_ConferenceService::DeleteConference_Method(const char *name, long cause, const char *pass)
{
	if (!name || !*name)
		return;

	VS_ConferenceDescription cd;
	bool sndNotEx = false;

	bool is_from_cfg(false);

	if(m_confRestriction->CheckSessionID(pass))
		is_from_cfg = true;


	if (cause!=0 &&
		((m_recvMess->IsFromServer() && strcmp(OurEndpoint(), m_recvMess->SrcServer())==0)||
		(is_from_cfg))){
		// week cheking, potentially may be  a hole ^)
			VS_Container cnt;
		if (g_storage->FindConference(name, cd) || !m_confRestriction->FindMultiConference(name, cd, cnt, nullptr, true))
			RemoveConference_Event(cd, cause);
		return;
	}

	vs_user_id user_id = m_recvMess->SrcUser();

	if (!user_id) return;

	if (g_storage->FindConference(name, cd)) {
		switch(cd.m_state)
		{
		case cd.CONFERENCE_CREATED:
		case cd.PARTICIPANT_INVITED:
			RemoveConference_Event(cd, cd.TERMINATED_BY_DELETE);
			break;
		case cd.PARTICIPANT_ACCEPTED:
			RemoveParticipant_Event(user_id, VS_ParticipantDescription::DISCONNECT_HANGUP);
			break;
		case cd.CONFERENCE_ENDED:
			break;
		case cd.CONFERENCE_HOLD:
			break;
		}
	}
	else sndNotEx = true;

	if (sndNotEx) {
		VS_Container rCnt;
		rCnt.AddValue(METHOD_PARAM, CONFERENCEDELETED_METHOD);
		rCnt.AddValueI32(RESULT_PARAM, CONFERENCE_DOESNT_EXIST);
		rCnt.AddValue(NAME_PARAM, name);
		PostReply(rCnt);
	}
}

////////////////////////////////////////////////////////////////////////////////
// INVITE_METHOD
////////////////////////////////////////////////////////////////////////////////
class Invite_Task: public VS_PoolThreadsTask, public VS_ConfLog
{
	boost::shared_ptr<VS_PresenceService> m_presenceService;
	bool					m_cancel_and_redirect;
	InviteInfo				v;
	boost::weak_ptr<VS_ConfRestrictInterface> m_confRestr;

public:
	Invite_Task(const boost::shared_ptr<VS_PresenceService>& PresSRV, InviteInfo &i, bool cancel_and_redirect, const boost::weak_ptr<VS_ConfRestrictInterface>& confRestr)
		: m_presenceService(PresSRV), m_cancel_and_redirect(cancel_and_redirect), v(i), m_confRestr(confRestr)
	{}

	void Run() {
		bool result = false;
		bool local = false;
		VS_UserPresence_Status status = USER_INVALID;
		VS_ConferenceDescription	cd;
		// 1- caller, 2 - other
		VS_UserData		ud1, ud2;
		vs_user_id		user_id2=v.user;
		VS_Reject_Cause cause = PARTISIPANT_NOT_AVAILABLE_NOW;
		VS_ConferenceDescription::TerminationCause confCause = VS_ConferenceDescription::TERMINATED_BY_INVITE_REJECT;
		VS_CallIDInfo ci2;
		auto time = std::chrono::system_clock::now();
		unsigned fwd_timeout(0);

		// Start Invitaton process
		do {
			if (!v.src_user || !g_storage->FindUser(SimpleStrToStringView(v.src_user), ud1))
			{ cause = INVALID_PARTISIPANT; break;}

			result = g_storage->FindConference(SimpleStrToStringView(v.conf), cd) &&
				(cd.m_state == cd.CONFERENCE_CREATED || cd.m_state == cd.PARTICIPANT_INVITED)
				&& (cd.m_type == CT_PRIVATE ? cd.m_owner==v.src_user : cd.m_type == CT_HALF_PRIVATE ? cd.m_party==v.src_user : false);
			if (!result) {
				cause = INVALID_CONFERENCE; break;
			}
			{
				// store caps
				VS_ParticipantDescription pd;
				auto confRestr = m_confRestr.lock();

				pd.m_conf_id = cd.m_name;
				pd.m_user_id = ud1.m_name;
				pd.m_type = cd.m_type == CT_HALF_PRIVATE ? pd.HPRIVATE_MEMBER : pd.PRIVATE_HOST;
				pd.m_version = ud1.m_protocolVersion;
				pd.m_server_id = OurEndpoint();
				pd.m_appID = ud1.m_appID;
				pd.m_displayName = ud1.m_displayName;
				pd.m_caps = v.m_caps;
				pd.m_IsOperatorByGroups = confRestr ? confRestr->IsOperator(pd.m_user_id) : false;
				int error = g_storage->AddParticipant(pd);
				result = error == 0;
				if (!result) {
					cause = REJECTED_BY_RESOURCE_LIMIT; break;
				}
			}

			if (m_cancel_and_redirect)
			{
				VS_Container rCnt;
				rCnt.AddValue(METHOD_PARAM, CONFERENCEDELETED_METHOD);
				rCnt.AddValue(NAME_PARAM, v.conf);
				rCnt.AddValueI32(RESULT_PARAM, CONFERENCE_DELETED_OK);
				rCnt.AddValueI32(CAUSE_PARAM, VS_ConferenceDescription::UNKNOWN);
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

			const auto our_endpoint = OurEndpoint();
			if (strstr(user_id2, "/") != 0 && m_presenceService->IsRegisteredTransId(user_id2))		// todo(kt): it is full trans_id -> call directly to it
			{
				ci2.m_serverID = !!our_endpoint ? our_endpoint : std::string{};
				local = true;
				LogParticipantInvite(v.conf, v.src_user, ud1.m_appID, user_id2, time, VS_ParticipantDescription::PRIVATE_HOST);
				cd.m_state = cd.PARTICIPANT_INVITED;
				cd.SetTimeExp(VS_ConferenceService::TIME_EXP_INVITE);
				g_storage->UpdateConference(cd);
				break;
			}

			status = m_presenceService->ResolveWithForwarding(v.src_user, user_id2, &v.fwd_limit,
				m_cancel_and_redirect, &ci2, &ud1, &fwd_timeout);
			if (!ci2.m_displayName.empty())
				v.user_display_name = ci2.m_displayName;

			dprint3("Invite: Resolving %s(%s): status %d at %s\n", v.user.m_str, user_id2.m_str, status, ci2.m_serverID.c_str());

			if ((our_endpoint == nullptr || *our_endpoint == 0 ? ci2.m_serverID.empty() : ci2.m_serverID == our_endpoint)) {
				local = true;
				g_storage->FindUser(SimpleStrToStringView(user_id2), ud2);
			}

			const char* other_bs = (!local) ? ci2.m_homeServer.c_str() : nullptr;

			LogParticipantInvite(v.conf, v.src_user, ud1.m_appID, user_id2, time, VS_ParticipantDescription::PRIVATE_HOST, other_bs);

			result = false; // reset
			if (status==USER_STATUS_UNDEF) {
				cause = REJECTED_BY_ACCESS_DENIED;
				confCause = VS_ConferenceDescription::TERMINATED_BY_PART_ACCDEN;
				break;
			}
			if (status==USER_INVALID) {
				cause = INVALID_PARTISIPANT;
				confCause = VS_ConferenceDescription::TERMINATED_BY_INVITE_NOT_FOUND;
				break;
			}
			else if (status==USER_LOGOFF) {
				cause = PARTISIPANT_NOT_AVAILABLE_NOW;
				confCause = VS_ConferenceDescription::TERMINATED_BY_INVITE_OFFLINE;
				break;
			}
			else if (status>=USER_BUSY) {
				cause = PARTISIPANT_IS_BUSY;
				confCause = VS_ConferenceDescription::TERMINATED_BY_INVITE_BUSY;
				break;
			}
			if (ud1.m_rating >= g_rating_th) {
				std::lock_guard<std::mutex> lock(g_storage->m_BookLock);
				VS_Storage::VS_UserBook* ib = g_storage->GetUserBook(ud1.m_name, AB_INVERSE);
				if (ib && !ib->IsInBook(user_id2)) {
					cause = ud1.m_protocolVersion > 33 ? REJECTED_BY_BADRATING : REJECTED_BY_PARTICIPANT;
					break;
				}
			}

			int error_code(0);
			if (cd.m_type==CT_PRIVATE) {
				cd.m_party = user_id2;
				if(local) {
					error_code=g_storage->CheckParticipant(user_id2, VS_ParticipantDescription::PRIVATE_MEMBER, v.src_user, v.src_user);
					result=error_code==0;
				}
				else
					result = true;

				if (result && v.src_user!= user_id2)
					result &= g_storage->CheckParticipant(v.src_user, VS_ParticipantDescription::PRIVATE_HOST, v.src_user, user_id2)==0;
			}
			else {
				cd.m_owner = user_id2;
				if (local) {
					error_code = g_storage->CheckParticipant(user_id2, VS_ParticipantDescription::HPRIVATE_HOST, v.src_user, v.src_user)==0;
					result=error_code==0;
				}
				else
					result = true;

				if (result && v.src_user!= user_id2)
					result &= g_storage->CheckParticipant(v.src_user, VS_ParticipantDescription::HPRIVATE_MEMBER, v.src_user, user_id2)==0;
			}
			if (!result) {
				switch(error_code)
				{
				case VSS_USER_NOT_FOUND:		cause = INVALID_PARTISIPANT; break;
				case VSS_CONF_ACCESS_DENIED:	cause = REJECTED_BY_ACCESS_DENIED; break;
				case VSS_CONF_NO_MONEY:			cause = REACH_MONEY_LIMIT; break;
				default:						cause = REJECTED_BY_RESOURCE_LIMIT; break;
				}
				break;
			}

			VS_ParticipantDescription cpd;
			if (g_storage->FindParticipant(SimpleStrToStringView(ud1.m_name), cpd)) {
				cpd.m_addledTick += std::chrono::seconds(VS_ConferenceService::TIME_EXP_INVITE); // 100 sec timeout
				g_storage->UpdateParticipant(cpd);
			}

			cd.m_state = cd.PARTICIPANT_INVITED;
			cd.SetTimeExp(VS_ConferenceService::TIME_EXP_INVITE);
			g_storage->UpdateConference(cd);

		} while (false);

		if (!result) {
			VS_Container rCnt;
			rCnt.AddValue(METHOD_PARAM, REJECT_METHOD);
			rCnt.AddValue(CONFERENCE_PARAM, v.conf);
			rCnt.AddValue(CALLID_PARAM, v.user);
			rCnt.AddValueI32(CAUSE_PARAM, cause);

			// ������� � BS::LOCATE_SRV->LOG_SRV
			auto confRestr = m_confRestr.lock();
			v.server = confRestr ? confRestr->GetAnyBSbyDomain(SimpleStrToStringView(v.user)) : VS_SimpleStr();
			if (!local && v.server.IsEmpty())
				v.server = OurEndpoint();
			if ( (status < USER_AVAIL) && !!v.server ) {
				VS_Container cnt;
				cnt.AddValue(METHOD_PARAM, SENDMAIL_METHOD);
				cnt.AddValue(CALLID_PARAM, v.src_user);
				cnt.AddValue(DISPLAYNAME_PARAM, ud1.m_displayName);
				cnt.AddValue(CALLID2_PARAM, user_id2);
				cnt.AddValue(USERNAME_PARAM, user_id2);		// for BS::LOCATION_SRV
				cnt.AddValue("app_name", ud1.m_appName);
				cnt.AddValue(TIME_PARAM, time);

				VS_TransportRouterServiceHelper::PostRequest(v.server, 0, cnt, LOG_SRV, LOCATE_SRV);
			}

			if (cause != INVALID_PARTISIPANT) {
				rCnt.AddValue(USERNAME_PARAM, user_id2);
			}
			PostRequest(0, v.src_user, rCnt);
			if (!!cd.m_name || (cause != INVALID_CONFERENCE && g_storage->FindConference(SimpleStrToStringView(v.conf), cd))) {
				VS_Container rCnt;
				rCnt.AddValue(METHOD_PARAM, DELETECONFERENCE_METHOD);
				rCnt.AddValue(NAME_PARAM, cd.m_name);
				rCnt.AddValueI32(CAUSE_PARAM, confCause);
				PostRequest(OurEndpoint(), 0, rCnt);
			}
		}
		else {
			// To Callee
			const char* to_user = (ci2.m_ml_cap == VS_MultiLoginCapability::UNKNOWN ||
				ci2.m_ml_cap == VS_MultiLoginCapability::SINGLE_USER)? user_id2.m_str: nullptr;
			{	// INVITE_METHOD
				VS_Container rCnt;
				const bool NeedInfo = true;
				rCnt.AddValue(METHOD_PARAM, INVITE_METHOD);
				rCnt.AddValue(CONFERENCE_PARAM, cd.m_name);
				rCnt.AddValue(NAME_PARAM, v.src_user);
				rCnt.AddValueI32(SEPARATIONGROUP_PARAM, ud1.m_SeparationGroup);
				rCnt.AddValue(CALLID_PARAM, user_id2);
				rCnt.AddValue(DISPLAYNAME_PARAM, ud1.m_displayName);
				rCnt.AddValueI32(DIRECTCONNECT_PARAM, v.m_type);
				if (v.m_caps.IsValid())
					rCnt.AddValue(CLIENTCAPS_PARAM, v.m_caps.Buffer(), v.m_caps.Size());
				if (!local)
					rCnt.AddValue(NEEDCONNECTINFO_PARAM, NeedInfo);
				PostRequest(ci2.m_serverID.c_str(), to_user, rCnt);
			}
			if (!local) {
				VS_Container rCnt;
				if (g_appServer->GetNetInfo(rCnt))
				{
					VS_TransportRouterServiceHelper::PostRequest(ci2.m_serverID.c_str(), to_user, rCnt, 0, CONFIGURATION_SRV);
				}
			}
			{
				VS_Container rCnt;
				rCnt.AddValue(METHOD_PARAM, USERREGISTRATIONINFO_METHOD);
				rCnt.AddValue(USERNAME_PARAM, ud1.m_name);
				rCnt.AddValue(DISPLAYNAME_PARAM, ud1.m_displayName);
				if (!ud1.m_callId.empty())
					rCnt.AddValue(CALLID_PARAM, ud1.m_callId.c_str());
				else
					rCnt.AddValue(CALLID_PARAM, ud1.m_name);
				PostRequest(ci2.m_serverID.c_str(), to_user, rCnt);

				rCnt.Clear();
				if (ud1.m_IPconfig.IsValid())
					rCnt.Deserialize(ud1.m_IPconfig.Buffer(), ud1.m_IPconfig.Size());
				rCnt.AddValue(METHOD_PARAM, CONFIGURATIONUPDATED_METHOD);
				rCnt.AddValue(NAME_PARAM, v.src_user);
				PostRequest(ci2.m_serverID.c_str(), to_user, rCnt);
			}
			// To Caller
			{
				VS_Container rCnt;
				rCnt.AddValue(METHOD_PARAM, USERREGISTRATIONINFO_METHOD);
				rCnt.AddValue(USERNAME_PARAM, user_id2);
				if (!ud2.m_displayName.empty())
					rCnt.AddValue(DISPLAYNAME_PARAM, ud2.m_displayName);
				else if (!ci2.m_displayName.empty())
					rCnt.AddValue(DISPLAYNAME_PARAM, ci2.m_displayName);
				rCnt.AddValue(CALLID_PARAM, v.user);
				PostRequest(0, v.src_user, rCnt);
			}
			if (local) {
				VS_Container rCnt;
				if (ud2.m_IPconfig.IsValid())
					rCnt.Deserialize(ud2.m_IPconfig.Buffer(), ud2.m_IPconfig.Size());
				rCnt.AddValue(METHOD_PARAM, CONFIGURATIONUPDATED_METHOD);
				rCnt.AddValue(NAME_PARAM, user_id2);
				PostRequest(0, v.src_user, rCnt);
			}

			v.timeout = std::chrono::steady_clock::now() + (fwd_timeout ? std::chrono::seconds(fwd_timeout) : std::chrono::minutes(2));
			v.fwd_limit =  fwd_timeout ? v.fwd_limit : 0;
			v.user = user_id2;
			g_storage->StartInvitationProcess( v );
		}
	}

	void PostRequest(const char *server, const char *user, VS_Container &cnt, unsigned long time = default_timeout)
	{
		VS_TransportRouterServiceHelper::PostRequest(server, user, cnt,0, CONFERENCE_SRV,time,CONFERENCE_SRV);
	}
};

void VS_ConferenceService::Invite_Method(VS_Container &cnt)
{
	if (cnt.GetStrValueRef(CALLID_PARAM))
	{
		size_t size;
		const void * buff = cnt.GetBinValueRef(CLIENTCAPS_PARAM, size);
		int32_t directConnect = NO_DIRECT_CONNECT;
		cnt.GetValue(DIRECTCONNECT_PARAM, directConnect);

		InviteInfo i;
		i.conf = cnt.GetStrValueRef(CONFERENCE_PARAM);
		i.fwd_limit = 2;
		i.m_caps.Set(buff, size);
		i.m_type = (VS_DirectConnect_Type)directConnect;
		i.src_user = m_recvMess->SrcUser();
		i.timeout = std::chrono::steady_clock::time_point();
		i.user = cnt.GetStrValueRef(CALLID_PARAM);
		i.conf_type = InviteInfo::CONF_1_TO_1;
		PutTask(new Invite_Task(m_presenceService, i, false, m_confRestriction) , "Invt", 30);
	}
	else {
		dprint2("In INvite Bad Params\n");
	}
}


const char* GetConnectTypeName(long type)
{
	switch(type)
	{
	case DIRECT_ACCEPT:
	case DIRECT_CONNECT:
		return "direct TCP";
	case DIRECT_NHP:
		return "direct NHP";
	case DIRECT_UDP:
		return "direct UDP";
	default:
		return "server TCP";
	}
}

bool ExtractIP(VS_SimpleStr &ip, VS_BinBuff &ipcfg) {
	VS_Container cnt;
	void *IP;
	size_t size;
	std::string host;
	std::map<std::string, int> flt;
	std::map<std::string, int>::iterator it;

	ip = "";
	if (ipcfg.IsValid()) {
		cnt.Deserialize(ipcfg.Buffer(), ipcfg.Size());
		cnt.Reset();
		while (cnt.Next()) {
			if (strcasecmp(cnt.GetName(), IPCONFIG_PARAM) == 0) {
				IP = (void *)cnt.GetBinValueRef(size);
				net::endpoint::ConnectTCP tcp;
				tcp.Deserialize(IP, size);
				if (!tcp.host.empty()) {
					host = tcp.host;
					flt[host] = 0;
				}
			}
		}
		it = flt.begin();
		while (it != flt.end()) {
			if (!!ip)
				ip+=", ";
			ip+= it->first.c_str();
			++it;
		}
	}
	return !!ip;
}


////////////////////////////////////////////////////////////////////////////////
// ACCEPT_METHOD(CONFERENCE_PARAM, NAME_PARAM, DIRECTCONNECT_PARAM)
////////////////////////////////////////////////////////////////////////////////
void VS_ConferenceService::Accept_Method(VS_Container &cnt)
{
	int32_t directConnect = NO_DIRECT_CONNECT;
	cnt.GetValue(DIRECTCONNECT_PARAM, directConnect);
	const auto conferenceName = cnt.GetStrValueView(CONFERENCE_PARAM);
	const auto name = cnt.GetStrValueView(NAME_PARAM);
	const char *src_user = m_recvMess->SrcUser();
	const char *user_id2 = cnt.GetStrValueRef(USERNAME_PARAM);
	if (!user_id2)
		user_id2 = src_user;
	const char *app_id = cnt.GetStrValueRef(APPID_PARAM);
	// ssl stream
	VS_SimpleStr sym_key;
	if (cnt.GetStrValueRef(SYMKEY_PARAM))
		m_confRestriction->GetSSL(sym_key);

	if (conferenceName.empty() || name.empty())
		return;

	dstream3 << "Accept from " << user_id2 << " to <" << conferenceName << ">";

	size_t size;
	const void *buff = cnt.GetBinValueRef(CLIENTCAPS_PARAM, size);
	VS_BinBuff caps(buff, size);

	VS_ConferenceDescription	cd;
	VS_UserData					ud1,ud2;
	VS_Reject_Cause cause = REJECTED_BY_RESOURCE_LIMIT;
	bool result = false;
	bool writeConf(false);
	g_storage->FindUser(user_id2, ud2);

	if(!user_id2) {
		dprint1("could not determine an accepting user\n");
		return;
	};

	VS_ParticipantDescription pd2, pd1;						// 1 - caller(inviting user), 2 - accepting user
	InviteInfo inv_info;
	if (g_storage->EndInvitationProcess(StringViewToSimpleStr(conferenceName), user_id2, inv_info) &&
		g_storage->FindConference(conferenceName, cd)
		&& cd.m_state==cd.PARTICIPANT_INVITED
		&& ((cd.m_type == CT_PRIVATE && SimpleStrToStringView(cd.m_owner) == name)
		 || (cd.m_type == CT_HALF_PRIVATE && SimpleStrToStringView(cd.m_party) == name))
		)
	{
		writeConf = cd.m_need_record;
		if (writeConf)
			directConnect = NO_DIRECT_CONNECT;

		if (g_storage->FindUser(name, ud1)) {	// Found destination endpoint
			if (g_storage->FindParticipant(SimpleStrToStringView(ud1.m_name), pd1)) {
				if (pd1.m_user_id != user_id2) {	//not self call

					pd2.m_conf_id = cd.m_name;
					pd2.m_user_id = user_id2;
					pd2.m_type = cd.m_type == CT_HALF_PRIVATE ? pd2.HPRIVATE_HOST : pd2.PRIVATE_MEMBER;
					pd2.m_version = atou_s(m_recvMess->AddString());
					pd2.m_caps = caps;
					pd2.m_server_id = m_recvMess->SrcServer();
					pd2.m_appID = app_id;
					pd2.m_displayName = inv_info.user_display_name;
					if (pd2.m_displayName.empty() && !ud2.m_displayName.empty())
						pd2.m_displayName = ud2.m_displayName;
					pd2.m_IsOperatorByGroups = m_confRestriction->IsOperator(pd2.m_user_id);

					if (g_storage->AddParticipant(pd2) == 0) {
						// Now we have Conference (cd) and 2 Participants

						if (directConnect == NO_DIRECT_CONNECT) {
							if (m_sr->CreateConference(cd.m_name.m_str, (VS_Conference_Type)cd.m_type, sym_key, cd.m_MaxParticipants)) {
								if (m_sr->AddParticipant(cd.m_name.m_str, pd2.m_user_id.m_str, 0, false, std::chrono::seconds(25)) &&
									m_sr->AddParticipant(cd.m_name.m_str, pd1.m_user_id.m_str, pd2.m_user_id.m_str, 0, false, std::chrono::seconds(25)))
								{
									if (pd1.m_caps.IsValid())
									{
										m_sr->SetParticipantCaps(cd.m_name.m_str, pd1.m_user_id.m_str, pd1.m_caps.Buffer(), pd1.m_caps.Size());
									}
									if (pd2.m_caps.IsValid())
									{
										m_sr->SetParticipantCaps(cd.m_name.m_str, pd2.m_user_id.m_str, pd2.m_caps.Buffer(), pd2.m_caps.Size());
									}
									result = true;
								}
								else {
									dprint1("AddParticipant FAILED, cnf=%s, p1=%s, p2=%s\n", cd.m_name.m_str, pd1.m_user_id.m_str, pd2.m_user_id.m_str);
								}
							}
							else {
								dprint1("CreateConference FAILED, cnf=%s, mp=%u\n", cd.m_name.m_str, cd.m_MaxParticipants);
							}
						}
						else {
							cd.m_type = CT_PRIVATE_DIRECTLY;
							cd.m_SubType = directConnect;
							result = true;
						}
					}
					else {
						dprint1("AddParticipant pd2 FAILED, cnf=%s, pd2=%s\n", cd.m_name.m_str, user_id2);
					}
				}
				else {
					pd2 = pd1;
					if (directConnect == NO_DIRECT_CONNECT) {
						if (m_sr->CreateConference(cd.m_name.m_str, (VS_Conference_Type)cd.m_type, sym_key, cd.m_MaxParticipants)) {
							if (m_sr->AddParticipant(cd.m_name.m_str, pd1.m_user_id.m_str, pd1.m_user_id.m_str)) {
								if (pd1.m_caps.IsValid())
								{
									m_sr->SetParticipantCaps(cd.m_name.m_str, pd1.m_user_id.m_str, pd1.m_caps.Buffer(), pd1.m_caps.Size());
								}
								result = true;
							}
						}
					}
					else {
						cd.m_type = CT_PRIVATE_DIRECTLY;
						cd.m_SubType = directConnect;
						result = true;
					}
				}
			}
		}
		else cause = PARTISIPANT_NOT_AVAILABLE_NOW;
	}
	else  cause = REJECTED_BY_ACCESS_DENIED;

	if (result) {
		auto pool = m_transceiversPool.lock();
		assert(pool != nullptr);
		if (pool) {
			auto streamsCircuit = pool->GetTransceiverProxy(std::string(conferenceName));
			if (streamsCircuit) {
				if (auto confCtrl = streamsCircuit->ConfControl())
				{
					confCtrl->SetDisplayName(cd.m_name, pd1.m_user_id, pd1.m_displayName.c_str());
				}
			}
		}

		if (writeConf)
		{
			m_confWriterModule->StartRecordConference(cd.m_name);
			m_confRestriction->SetRecordState(cd, RS_RECORDING);
		}

		if (cd.m_type == CT_HALF_PRIVATE ) SendPartsList(cd);

		cd.m_state = cd.PARTICIPANT_ACCEPTED;
		cd.SetTimeExp(TIME_EXP_ACCEPT);
		m_confRestriction->UpdateConfDuration(cd);

		cd.m_symKey = sym_key;
		g_storage->UpdateConference(cd);

		{
			VS_Container rCnt;
			rCnt.AddValue(METHOD_PARAM, ACCEPT_METHOD);
			rCnt.AddValue(CONFERENCE_PARAM, conferenceName);
			rCnt.AddValue(NAME_PARAM, name);
			rCnt.AddValueI32(DIRECTCONNECT_PARAM, directConnect);
			if (caps.IsValid())
				rCnt.AddValue(CLIENTCAPS_PARAM, caps.Buffer(), caps.Size());
			if (!!sym_key)
				rCnt.AddValue(SYMKEY_PARAM, sym_key);
			rCnt.AddValueI32(CMR_FLAGS_PARAM, m_confRestriction->GetCMRFlagsByLicense());
			PostRequest(OurEndpoint() , ud1.m_name, rCnt);
		}
		{
			VS_SimpleStr ip1, ip2;
			ExtractIP(ip1, ud1.m_IPconfig);
			ExtractIP(ip2, ud2.m_IPconfig);
			// date_time|debug_level|debug_module|thread_id|type_message|user_source|user_destination|user_source_ip|user_destination_ip|type
			dprint3("connect type|%s|%s|%s|%s|%s\n", pd1.m_user_id.m_str, pd2.m_user_id.m_str, ip1.m_str, ip2.m_str, GetConnectTypeName(directConnect));
		}
		{
			VS_Container rCnt;
			rCnt.AddValue(METHOD_PARAM, ACCEPT_METHOD);
			rCnt.AddValue(CONFERENCE_PARAM, conferenceName);
			rCnt.AddValue(NAME_PARAM, src_user);
			directConnect = directConnect==DIRECT_CONNECT ? DIRECT_ACCEPT : directConnect==DIRECT_ACCEPT ? DIRECT_CONNECT : directConnect;
			rCnt.AddValue(DIRECTCONNECT_PARAM, directConnect);
			if (!!sym_key)
				rCnt.AddValue(SYMKEY_PARAM, sym_key);
			rCnt.AddValueI32(CMR_FLAGS_PARAM, m_confRestriction->GetCMRFlagsByLicense());
			PostReply(rCnt);
		}
		LogParticipantJoin(pd1);
		LogParticipantJoin(pd2);
	}
	else
	{
		VS_Container rCnt;
		rCnt.AddValue(METHOD_PARAM, REJECT_METHOD);
		rCnt.AddValue(CONFERENCE_PARAM, conferenceName);
		rCnt.AddValue(NAME_PARAM, name);
		rCnt.AddValueI32(CAUSE_PARAM, cause);
		PostReply(rCnt);
	}
}

////////////////////////////////////////////////////////////////////////////////
// REJECT_METHOD(CONFERENCE_PARAM, NAME_PARAM)
////////////////////////////////////////////////////////////////////////////////
void VS_ConferenceService::Reject_Method(VS_Container &cnt)
{
	const char *conferenceName = cnt.GetStrValueRef(CONFERENCE_PARAM);
	const char *invited_from = cnt.GetStrValueRef(NAME_PARAM);
	const char *from = cnt.GetStrValueRef(USERNAME_PARAM);
	const char *alias = cnt.GetStrValueRef(ALIAS_PARAM);
	int32_t cause = REJECTED_BY_PARTICIPANT; cnt.GetValue(CAUSE_PARAM, cause);
	if ( !conferenceName || !invited_from)	return;
	if (from)
		g_storage->EndInvitationProcess(conferenceName, from); // EndInvitationProcess(...) can return false, if user sent ReqInviteTuMulty (user wasn't invited to conference)

	dprint3("Reject from %s to <%s>\n", from, conferenceName);

	VS_ConferenceDescription cd;
	auto conf_is_found = g_storage->FindConference(conferenceName, cd);
	if (!conf_is_found) {
		if (m_confRestriction->FindMultiConference(conferenceName, cd, cnt, from, IsBS(m_recvMess->SrcServer())) == 0) {
			conf_is_found = g_storage->FindConference(SimpleStrToStringView(cd.m_name), cd);
		}
	}

	if (conf_is_found) {
		VS_AutoInviteService::Unsubscribe(cd.m_name, from);
		LogParticipantReject(cd.m_name, from, invited_from,(VS_Reject_Cause)cause);

		if (cd.m_state==cd.PARTICIPANT_INVITED
			&& (cd.m_type == CT_PRIVATE ? cd.m_owner==invited_from : cd.m_type == CT_HALF_PRIVATE ? cd.m_party== invited_from : false))
		{
			VS_Container rCnt;
			rCnt.AddValue(METHOD_PARAM, REJECT_METHOD);
			rCnt.AddValue(CONFERENCE_PARAM, cd.m_name);
			rCnt.AddValue(NAME_PARAM, from);
			rCnt.AddValue(CAUSE_PARAM, cause);
			PostRequest(0, invited_from, rCnt);
			RemoveConference_Event(cd, cd.TERMINATED_BY_REJECT);
		}
		else if (cd.m_state==cd.PARTICIPANT_ACCEPTED && (cd.m_type==CT_MULTISTREAM||cd.m_type==CT_INTERCOM) && cd.m_owner==from) {
			VS_Container rCnt;
			rCnt.AddValue(METHOD_PARAM, REJECT_METHOD);
			rCnt.AddValue(CONFERENCE_PARAM, cd.m_name);
			rCnt.AddValue(CALLID_PARAM, alias ? alias : from);
			rCnt.AddValue(CAUSE_PARAM, cause);
			PostRequest(0, invited_from, rCnt);
		}
	}
	else
		LogParticipantReject(conferenceName, from, invited_from, (VS_Reject_Cause)cause);

}

////////////////////////////////////////////////////////////////////////////////
// HANGUP_METHOD(CONFERENCE_PARAM, NAME_PARAM)
////////////////////////////////////////////////////////////////////////////////
void VS_ConferenceService::Hangup_Method(const char *conferenceName, const char *name, long del, HangupFlags hflags)
{
	if (!conferenceName || !*conferenceName || !name || !*name) return;

	dprint3("Hangup from %s to <%s>\n", name, conferenceName);

	VS_ConferenceDescription cd;

	if (g_storage->FindConference(conferenceName, cd)) {
		VS_ParticipantDescription pd;
		bool CanHangupConf = (cd.m_owner == name || (g_storage->FindParticipant(name, pd) && pd.m_role == PR_LEADER)) ? true : false;

		if (bool(hflags & HangupFlags::BY_USER)) {
			Unsubscribe(conferenceName, name);
		}

		switch(cd.m_state)
		{
		case cd.CONFERENCE_CREATED:
		case cd.PARTICIPANT_INVITED:
			if (cd.m_owner==name || cd.m_party==name)
				RemoveConference_Event(cd, cd.TERMINATED_BY_HUNGUP);
			break;
		case cd.PARTICIPANT_ACCEPTED:
			if (CanHangupConf && del)
				RemoveConference_Event(cd, cd.TERMINATED_BY_HUNGUP);
			else
				RemoveParticipant_Event(name, VS_ParticipantDescription::DISCONNECT_HANGUP);
			break;
		case cd.CONFERENCE_ENDED:
			break;
		case cd.CONFERENCE_HOLD:
			break;
		}
	}
	else {
		VS_Container rCnt;
		rCnt.AddValue(METHOD_PARAM, CONFERENCEDELETED_METHOD);
		rCnt.AddValueI32(RESULT_PARAM, CONFERENCE_DOESNT_EXIST);
		rCnt.AddValue(NAME_PARAM, conferenceName);
		PostReply(rCnt);
	}
}

////////////////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// KICK_METHOD(CONFERENCE_PARAM, NAME_PARAM)
////////////////////////////////////////////////////////////////////////////////
void VS_ConferenceService::Kick_Method(const VS_Container &cnt)
{
	auto conferenceName = cnt.GetStrValueRef(CONFERENCE_PARAM);
	auto name = cnt.GetStrValueRef(NAME_PARAM);
	bool soft_kick(false);
	cnt.GetValue(SOFT_KICK_PARAM, soft_kick);
	if (!conferenceName || !*conferenceName || !name || !*name)	return;

	VS_ConferenceDescription	cd;
	VS_ParticipantDescription	pd;
	auto from_server = m_recvMess->IsFromServer()
		&& m_confRestriction->CheckSessionID(cnt.GetStrValueRef(SESSION_PARAM));
	string_view user_id;
	if (!from_server)
		user_id = m_recvMess->SrcUser_sv();
	dstream3 << "Kick "<< name
		<< " from " << (!from_server ? user_id : "configurator")
		<< " in <" << conferenceName << ">" << (soft_kick ? " soft" : "");
	if (!from_server && user_id.empty())
		return;
	if (g_storage->FindConferenceByUser(name, cd)
		&& (from_server || g_storage->FindParticipant(user_id, pd)))
	{
		if(cd.m_name == conferenceName
			&& (from_server
				|| SimpleStrToStringView(cd.m_owner) == user_id
				|| pd.m_role == PR_LEADER))
		{
			if(!soft_kick)
				AddConfKick(conferenceName, name);
			RemoveParticipant_Event(name,
				VS_ParticipantDescription::DISCONNECT_BYKICK);
			VS_AutoInviteService::Unsubscribe(cd.m_name, name);
		}
		else
			SendPartsList(cd);		// update
	}
}
////////////////////////////////////////////////////////////////////////////////
// IGNORE_METHOD(CONFERENCE_PARAM, NAME_PARAM)
////////////////////////////////////////////////////////////////////////////////
void VS_ConferenceService::Ignore_Method(const char *conferenceName, const char *name)
{
	if (!conferenceName || !*conferenceName || !name || !*name)	return;

	VS_ParticipantDescription pd;
	if (g_storage->FindParticipant(m_recvMess->SrcUser_sv(), pd))
		if (pd.m_rights&pd.RIGHTS_RCV_LIST)
			if (g_storage->FindConfIDByUser(name)==(vs_conf_id)conferenceName)
				g_storage->AddParticipantToIgnoreList(conferenceName, name);
}
////////////////////////////////////////////////////////////////////////////////
// JOIN_METHOD(NAME_PARAM)
////////////////////////////////////////////////////////////////////////////////
class JoinTask: public VS_PoolThreadsTask, public VS_TransportRouterServiceHelper
{
	boost::shared_ptr<VS_PresenceService> m_presenceService;
public:
	vs_user_id			m_user_id;
	VS_SimpleStr		m_callId;
	VS_SimpleStr		m_addString;
	VS_BinBuff			m_buff;

	JoinTask(const boost::shared_ptr<VS_PresenceService>& PresSRV, const char* user_id, const char* callId, const char* addString, const void* body, unsigned long bodySize)
		: m_presenceService(PresSRV), m_user_id(user_id), m_callId(callId), m_addString(addString), m_buff(body, bodySize) {}

	void Run() {
		VS_CallIDInfo ci;
		VS_UserPresence_Status status = m_presenceService->Resolve(m_callId, ci, false);
		const auto our_endpoint = OurEndpoint();
		const bool result = !/*eq*/(our_endpoint == nullptr || *our_endpoint == 0 /*empty*/ ? ci.m_serverID.empty() : ci.m_serverID == our_endpoint) && (status==USER_PUBLIC || status==USER_VIPPUBLIC);

		if (result) { // forward to other server
			dstream3 << "Join_Task forward to " << ci.m_serverID;
			VS_RouterMessage *mess = new VS_RouterMessage(CONFERENCE_SRV, m_addString, CONFERENCE_SRV, 0, m_user_id, ci.m_serverID.c_str(), OurEndpoint(), 20000,  m_buff.Buffer(), m_buff.Size());
			if (!PostMes(mess))
				delete mess;
		}
		else { /// send Reject
			dprint3("Join_Task fail, host %s, status=%d \n", m_callId.m_str, status);
			VS_Container rCnt;
			rCnt.AddValue(METHOD_PARAM, JOIN_METHOD);
			rCnt.AddValueI32(RESULT_PARAM, CONFERENCE_IS_BUSY);
			PostRequest(0, m_user_id, rCnt, 0, CONFERENCE_SRV, 20000, CONFERENCE_SRV);
		}
	}
};



void VS_ConferenceService::Join_Method(VS_Container &cnt)
{
	const auto name = cnt.GetStrValueView(NAME_PARAM);
	if (name.empty())
		return;

	const char *from = m_recvMess->SrcUser(); // USERNAME_PARAM is unused
	const char *app_id = cnt.GetStrValueRef(APPID_PARAM);

	const char* pass_ref = cnt.GetStrValueRef(PASSWORD_PARAM);
	VS_SimpleStr pass=pass_ref;
	bool pass_exists=pass_ref!=0;

	int32_t type = -1;
	cnt.GetValue(TYPE_PARAM, type);

	VS_ConferenceDescription	cd;
	VS_ParticipantDescription	pd, pd2; 	// member,host
	VS_UserData					ud2;		// host

	long result = CONFERENCE_IS_BUSY;

	if (g_storage->FindUser(name, ud2)) {	 // now we in hosts native broker

		if (g_storage->FindConferenceByUser(SimpleStrToStringView(ud2.m_name), cd) &&
			(cd.m_type==CT_PUBLIC || cd.m_type==CT_BROADCAST)?(type==CT_PUBLIC || type==CT_BROADCAST):(cd.m_type==type) &&
			SimpleStrToStringView(cd.m_owner) == name)
		{
			if (!cd.m_password || (!!pass && cd.m_password == pass) ) {
				pd.m_conf_id = cd.m_name;
				pd.m_user_id = from;
				if (cd.m_type==CT_HALF_PRIVATE) {
					pd.m_type = pd.HPRIVATE_GUEST;
					g_storage->AddParticipantToIgnoreList(pd.m_conf_id, pd.m_user_id);
				}
				else {
					pd.m_type = pd.PUBLIC_MEMBER;
				}
				pd.m_version = atou_s(m_recvMess->AddString());
				size_t size;
				const void *buff = cnt.GetBinValueRef(CLIENTCAPS_PARAM, size);
				pd.m_caps.Set(buff, size);
				pd.m_server_id = m_recvMess->SrcServer();
				pd.m_appID = app_id;
				pd.m_IsOperatorByGroups = m_confRestriction->IsOperator(pd.m_user_id);

				int error_code=g_storage->AddParticipant(pd);

				if(error_code==0) {
					vs_public_conf_buf* strBuff = new vs_public_conf_buf;
					if (m_sr->AddParticipant(cd.m_name.m_str, from)) {
////
						if(pd.m_caps.IsValid())
						{
							m_sr->SetParticipantCaps(cd.m_name.m_str, pd.m_user_id.m_str, pd.m_caps.Buffer(), pd.m_caps.Size());
						}
////
						// find host caps
						g_storage->FindParticipant(name, pd2);
						stream::Track tracks[4];
						unsigned nTracks;
						long fltr = VS_CheckFmt(pd.m_caps, pd2.m_caps, VS_RcvFunc::FLTR_ALL_MEDIA);
						VS_RcvFunc::SetTracks(fltr, tracks, nTracks);
						m_sr->ConnectParticipantReceiver(cd.m_name.m_str, from, cd.m_owner.m_str, tracks, nTracks);

						SendPartsList(cd);

						// LOG
						LogParticipantJoin(pd);
						// send message
						SendJoinMessage(cd, pd.m_user_id, 0);
						result = JOIN_OK;
					}
					else {
						if (strBuff) delete strBuff;
						result = NO_ENOUGH_RESOURCES_FOR_CONFERENCE;
					}
				}
				else {
					switch(error_code)
					{
					case VSS_CONF_MAX_PART_NUMBER:	result = CONFERENCE_IS_BUSY; break;
					case VSS_CONF_NOT_VALID:		result = INVALID_CONFERENCE; break;
					case VSS_CONF_NO_MONEY:			result = REACH_MONEY_LIMIT;	 break;
					case VSS_CONF_ACCESS_DENIED:	result = REJECTED_BY_ACCESS_DENIED; break;
					}
				}
			}
			else {
				if(pass_exists)
				{ result = !pass ? CONFERENCE_PASSWORD_REQUIRED : REJECTED_BY_WRONG_PASSWORD; }
				else
					result = REJECTED_BY_ACCESS_DENIED;
			}
		}
		else result = INVALID_CONFERENCE;
	}
	else {	// forward to other server
		PutTask(new JoinTask(m_presenceService, m_recvMess->SrcUser(), name.c_str(), m_recvMess->AddString(), m_recvMess->Body(), m_recvMess->BodySize()), "Join", 30);
		return;
	}

	{	// JOIN_METHOD
		VS_Container rCnt;
		rCnt.AddValue(METHOD_PARAM, JOIN_METHOD);
		rCnt.AddValueI32(RESULT_PARAM, result);
		if (result == JOIN_OK) {
			rCnt.AddValue(NAME_PARAM, from); // additional check
			rCnt.AddValue(CONFERENCE_PARAM, cd.m_name);
			rCnt.AddValue(USERNAME_PARAM, ud2.m_name);
			rCnt.AddValue(DISPLAYNAME_PARAM, ud2.m_displayName);
			rCnt.AddValue(CALLID_PARAM, ud2.m_name);
			if (pd2.m_caps.IsValid())
				rCnt.AddValue(CLIENTCAPS_PARAM, pd2.m_caps.Buffer(), pd2.m_caps.Size());
			if (strcasecmp(OurEndpoint(),m_recvMess->SrcServer())!=0) {
				// not local
				VS_Container rCnt;
				if (g_appServer->GetNetInfo(rCnt))
				{
					VS_MultiLoginCapability ml_cap = VS_MultiLoginCapability::UNKNOWN;
					cnt.GetValueI32(MULTI_LOGIN_CAPABILITY_PARAM, ml_cap);
					const char* to_user = (ml_cap == VS_MultiLoginCapability::UNKNOWN ||
						ml_cap == VS_MultiLoginCapability::SINGLE_USER)? m_recvMess->SrcUser(): nullptr;
					PostRequest(m_recvMess->SrcServer(), to_user, rCnt, 0, CONFIGURATION_SRV);
				}
			}
		}
		PostReply(rCnt);
	}
}

////////////////////////////////////////////////////////////////////////////////
// Sink for Stream Router
////////////////////////////////////////////////////////////////////////////////
void VS_ConferenceService::CreateConference(const char *conferenceName)
{
	assert(conferenceName != nullptr);
	dprint3("CreateConference <%s>\n", conferenceName);

	auto transceiversPool = m_transceiversPool.lock();
	assert(transceiversPool != nullptr);
	if (!transceiversPool) return;

	VS_ConferenceDescription cd;
	auto conferenceFound = g_storage->FindConference(conferenceName,cd);
	if (conferenceFound && cd.m_call_id && cd.m_call_id != conferenceName) {
		std::string fullConfID = cd.m_call_id.m_str;
		if (fullConfID.find('@') == std::string::npos) fullConfID += "@" + g_tr_endpoint_name;
		transceiversPool->ConnectReservedProxyToConference(fullConfID, conferenceName);
	}

	auto streamsCircuit = transceiversPool->GetTransceiverProxy(conferenceName, true);
	if (!!m_streamsRelay && !!streamsCircuit)
	{
		auto confControl = streamsCircuit->ConfControl();
		m_keyReqConn = confControl->ConnectToKeyFrameReq(boost::bind(&VS_ConferenceService::RequestKeyFrame, this, _1, _2));
		m_bitrateRestricConn = confControl->ConnectToRestrictBitrateSig(boost::bind(&VS_ConferenceService::RestrictBitrateSVC, this, _1, _2, _3, _4, _5));

		streamsCircuit->RegisterModule(m_confWriterModule, true);
		streamsCircuit->SetConfRecorderModule(m_confWriterModule);
		streamsCircuit->RegisterModule(m_RTSPBroadcastModule);
		streamsCircuit->SetRTSPBroadcastModule(m_RTSPBroadcastModule);

		m_confWriterModule->SetMessageSender(streamsCircuit, conferenceName);
		std::weak_ptr<VS_ConfRecorderModuleCtrl> w_confWriter(m_confWriterModule);
		streamsCircuit->SetRemoveFromSender([w_confWriter, conferenceName = std::string(conferenceName)]() {
			if (auto confWriter = w_confWriter.lock()) {
				confWriter->StopRecordConference(conferenceName.c_str());
				confWriter->RemoveMessageSender(conferenceName);
			}
		});

		m_RTSPBroadcastModule->SetMessageSender(streamsCircuit, conferenceName);
		std::weak_ptr<VS_RTSPBroadcastModuleCtrl> w_rtspBroadCast(m_RTSPBroadcastModule);
		streamsCircuit->SetRemoveFromSender([w_rtspBroadCast, conferenceName = std::string(conferenceName)]() {
			if (auto rtspBroadCast = w_rtspBroadCast.lock())
				rtspBroadCast->RemoveMessageSender(conferenceName);
		});

		if(conferenceFound)
		{
			std::shared_ptr<VS_CircuitStreamRelayInterface> temp = streamsCircuit;
			temp->StartConference(conferenceName,cd.m_owner,(VS_Conference_Type)cd.m_type,(VS_GroupConf_SubType)cd.m_SubType);
		}

		cd.m_transceiverName = streamsCircuit->GetTransceiverName();
		g_storage->UpdateConference(cd);
		m_streamsRelay->ConnectToConference(conferenceName, streamsCircuit);
	}
}

void VS_ConferenceService::AddParticipant(const char *conferenceName, const char *participantName)
{
	dprint3("AddParticipant <%s> to <%s> conf\n", participantName, conferenceName);
}

void VS_ConferenceService::RemoveConference(const char *conferenceName, const stream::ConferenceStatistics& cs)
{
	dprint3("RemoveConference <%s>\n", conferenceName);
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, DELETECONFERENCE_METHOD);
	rCnt.AddValue(NAME_PARAM, conferenceName);
	rCnt.AddValueI32(CAUSE_PARAM, VS_ConferenceDescription::TERMINATED_BY_STREAMS);
	PostRequest(OurEndpoint(), 0,  rCnt);
}

void VS_ConferenceService::RemoveParticipant(const char *conferenceName,
											 const char *participantName,
											 const stream::ParticipantStatistics& ps)
{
	dprint3("RemoveParticipant %s from <%s>\n", participantName, conferenceName);
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, DELETEPARTICIPANT_METHOD);
	rCnt.AddValue(CONFERENCE_PARAM, conferenceName);
	rCnt.AddValue(USERNAME_PARAM, participantName);
	rCnt.AddValue(DATA_PARAM, &ps, sizeof(stream::ParticipantStatistics));
	rCnt.AddValueI32(CAUSE_PARAM, VS_ParticipantDescription::DISCONNECT_BYSTREAM);
	PostRequest(OurEndpoint(), 0, rCnt);
}


////////////////////////////////////////////////////////////////////////////////
// Timer
////////////////////////////////////////////////////////////////////////////////
bool VS_ConferenceService::Timer( unsigned long tickcount)
{
	int ticks = tickcount;
	if (ticks - m_conf_check_time > 5000) {
		OnTimer_CheckConferences();
		m_conf_check_time = ticks;
	}
	if (ticks - m_part_check_time > 10000) {
		OnTimer_CheckPartLimit(ticks - m_part_check_time);
		m_part_check_time = ticks;
	}
	if (ticks - m_invite_check_time > 1000) {
		OnTimer_CheckInvites();
		m_invite_check_time = ticks;
	}

	VS_AutoInviteService::Timer(tickcount);

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// OnTimer finctions
////////////////////////////////////////////////////////////////////////////////
void VS_ConferenceService::OnTimer_CheckPartLimit(int dif)
{
	int NumOfPart = 0, i;
	VS_ParticipantDescription* pd;
	if ( (NumOfPart = g_storage->NextConferenceMinute(pd, dif, this))>0) {
		for (i = 0; i<NumOfPart; i++)
			RemoveParticipant_Event(pd[i].m_user_id, pd[i].m_cause);
		delete[] pd;
	}
}

void VS_ConferenceService::OnTimer_CheckConferences() {
	// If some conference is inactive and time expired initiate Requests "Conference Deleted"
	VS_ConferenceDescription* cd2 = NULL;
	int NumOfConf = 0, i;

	if ( (NumOfConf = g_storage->FindConferences(cd2, "OLD"))>0 ) {			// lost conferences
		for (i = 0; i < NumOfConf; i++)
			RemoveConference_Event(cd2[i], cd2[i].TERMINATED_BY_EXPIR);
		delete[] cd2;
	}
}

void VS_ConferenceService::OnTimer_CheckInvites()
{
	std::vector<InviteInfo> invites;
	g_storage->GetTimedoutInvites( invites,  InviteInfo::CONF_1_TO_1 );
	for (unsigned i = 0; i < invites.size(); i++)
		if (invites[i].fwd_limit > 0)	PutTask(new Invite_Task(m_presenceService, invites[i], true, m_confRestriction), "InvOnTmr");
}

////////////////////////////////////////////////////////////////////////////////
// Conference notifiy Events
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************
* 1 - Call RemoveParticipant_Event if any
* 2 - Release conference
*****************************************************************************/
void VS_ConferenceService::RemoveConference_Event(VS_ConferenceDescription& /*cd*/, long /*cause*/)
{
}

/******************************************************************************
* 1 - Notify participant by CONFERENCEDELETED_METHOD
* 2 - Release participant (log it)
*****************************************************************************/
void VS_ConferenceService::RemoveParticipant_Event(const char* /*participant*/, long /*cause*/, VS_ConfPartStat* /*ps*/)
{
}

bool VS_ConferenceService::IsBS(const char * server) const
{
	return server && VS_GetServerType(server) == ST_BS;
}

void VS_ConferenceService::SendPartsList(VS_ConferenceDescription &cd)
{
	VS_ParticipantDescription *parts;
	int NumOfPart = 0, j;

	if ( (NumOfPart = g_storage->GetParticipants(cd.m_name, parts))>0 ) {
		VS_Container rCnt, webcnt;
		rCnt.AddValue(METHOD_PARAM, SENDPARTSLIST_METHOD);
		rCnt.AddValue(CONFERENCE_PARAM, cd.m_name);
		webcnt.AddValue(METHOD_PARAM, SENDPARTSLIST_METHOD);
		webcnt.AddValue(CONFERENCE_PARAM, cd.m_name);
		// fil with parts and roles
		for (j = 0; j<NumOfPart; j++) {
			rCnt.AddValue(USERNAME_PARAM, parts[j].m_user_id);
			//long status = parts[j].m_type|(parts[j].m_rights<<16);
			//rCnt.AddValue(PUBCONFSTATUS_PARAM, status);
			rCnt.AddValueI32(ROLE_PARAM, (parts[j].m_role&0xff)|(parts[j].m_brStatus<<8));
			rCnt.AddValue(IS_OPERATOR_PARAM, parts[j].m_IsOperatorByGroups);
			rCnt.AddValueI32(DEVICESTATUS_PARAM, parts[j].m_devStatus);

			webcnt.AddValue(USERNAME_PARAM, parts[j].m_user_id);
			webcnt.AddValueI32(ROLE_PARAM, (parts[j].m_role & 0xff) | (parts[j].m_brStatus << 8));
			webcnt.AddValue(IS_OPERATOR_PARAM, parts[j].m_IsOperatorByGroups);
			webcnt.AddValueI32(DEVICESTATUS_PARAM, parts[j].m_devStatus);
			webcnt.AddValue(DISPLAYNAME_PARAM, parts[j].m_displayName);
		}
		void* body = nullptr;
		size_t bodySize = 0;
		rCnt.SerializeAlloc(body, bodySize);
		void* bodyw = nullptr;
		size_t bodySizew = 0;
		webcnt.SerializeAlloc(bodyw, bodySizew);
		for (j = 0; j<NumOfPart; j++)
			if (parts[j].m_rights&parts->RIGHTS_RCV_LIST) {
				if (parts[j].m_ClientType == CT_WEB_CLIENT)
					PostRequest(parts[j].m_server_id, parts[j].m_user_id, bodyw, bodySizew, 0, PRESENCE_SRV, 20000);
				else
					PostRequest(parts[j].m_server_id, parts[j].m_user_id, body, bodySize, 0, PRESENCE_SRV, 20000);
			}
		free(body);
		free(bodyw);
		delete[] parts;
	}

}

void VS_ConferenceService::SendJoinMessage(VS_ConferenceDescription &cd, vs_user_id &user_id, int type)
{
	char mess[256]; *mess = 0;
	if (cd.m_state==cd.CONFERENCE_ENDED) return;

	VS_ParticipantDescription pd;	// host
	if (!g_storage->FindParticipant(SimpleStrToStringView(cd.m_owner), pd)) return;

	VS_UserData ud;			// member
	const char* name(nullptr);
	if (g_storage->FindUser(SimpleStrToStringView(user_id), ud) && !ud.m_displayName.empty())
		name = ud.m_displayName.c_str();
	else
		name = user_id;

	int users = g_storage->GetNumOfParts(cd.m_name) - 1; // whithout host

	if (type == 0) {
		if (users!=1)
			snprintf(mess, 256, "%s has joined the channel (currently %d users)", name, users);
		else
			snprintf(mess, 256, "%s has joined the channel (currently 1 user)", name);
	}
	else { //type ==1
		if (users!=1)
			snprintf(mess, 256, "%s has left the channel (%d users left)", name, users);
		else
			snprintf(mess, 256, "%s has left the channel (1 user left)", name);
	}
	SendSystemMessage(mess, pd.m_user_id);
}

void VS_ConfKick::AddConfKick(string_view conf_id, string_view user_id)
{
	if (conf_id.empty() || user_id.empty())
		return ;

	auto user_id_normalized = VS_NormalizeCallID(user_id);
	dstream3 << "KICK: AddConfKick " << user_id_normalized << " to " << conf_id << " conference";
	m_data.withLock([&](data_t& data) {
		data[std::string(conf_id)].insert(std::move(user_id_normalized));
	});
}

void VS_ConfKick::RemoveConfKick(string_view conf_id, string_view user_id)
{
	if (conf_id.empty() || user_id.empty())
		return ;

	auto user_id_normalized = VS_NormalizeCallID(user_id);
	dstream3 << "KICK: RemoveConfKick " << user_id_normalized << " from " << conf_id << " conference";
	m_data.withLock([&](data_t& data) {
		const auto it = data.find(conf_id);
		if (it == data.end())
			return;
		it->second.erase(user_id_normalized);
	});
}

void VS_ConfKick::RemoveConfKicks(string_view conf_id)
{
	if (conf_id.empty())
		return ;

	dstream3 << "KICK: RemoveConfKicks from " << conf_id << " conference";
	m_data.withLock([&](data_t& data) {
		const auto it = data.find(conf_id);
		if (it == data.end())
			return;
		data.erase(it);
	});
}

bool VS_ConfKick::IsKickedUser(string_view conf_id, string_view user_id, const VS_ConfRestrictInterface* confRestrict)
{
	if (conf_id.empty() || user_id.empty())
		return false;

	VS_ConferenceDescription cd;
	if (!g_storage->FindConference(conf_id, cd))
	{
		// Join to named_conf or VCS regsitry conf
		if (!confRestrict)
			return false;
		const auto local_conf_id = confRestrict->GetLocalMultiConfID(StringViewToSimpleStr(conf_id));
		if (local_conf_id.IsEmpty())
			return false;
		if (g_storage->FindConferenceByCallID(SimpleStrToStringView(local_conf_id), cd) != 0)
			return false;
		conf_id = SimpleStrToStringView(cd.m_name);
		assert(!conf_id.empty());
	}

	auto user_id_normalized = VS_NormalizeCallID(user_id);
	const bool result = m_data.withLock([&](data_t& data) {
		const auto it = data.find(conf_id);
		if (it == data.end())
			return false;
		return it->second.count(user_id_normalized) > 0;
	});
	dstream3 << "KICK: IsKickedUser " << user_id_normalized << " from " << conf_id << "conference: " << std::boolalpha << result;
	return result;
}

void VS_ConferenceService::onRecordStartInfo(const vs_conf_id& conf_id, const std::string& filename,
	std::chrono::system_clock::time_point started_at)
{
	if (!IsInProcessingThread())
	{
		CallInProcessingThread([this, conf_id, filename, started_at]
		{
			onRecordStartInfo(conf_id, filename, started_at);
		});
		return;
	}
	m_confRestriction->LogRecordStart(conf_id, filename, started_at, this);
}

void VS_ConferenceService::onRecordStopInfo(const vs_conf_id& conf_id, std::chrono::system_clock::time_point stopped_at,
	uint64_t file_size)
{
	if (!IsInProcessingThread())
	{
		CallInProcessingThread([this, conf_id, stopped_at, file_size]
		{
			onRecordStopInfo(conf_id, stopped_at, file_size);
		});
		return;
	}
	m_confRestriction->LogRecordStop(conf_id, stopped_at, file_size, this);
}
