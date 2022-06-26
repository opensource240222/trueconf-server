/**
 ****************************************************************************
 * (c) 2002-2003 Visicron Inc.  http://www.visicron.net/
 *
 * Project: Visicron Media Broker
 *
 * \file VS_ReadLicense.h
 *
 * License handling function declaration
 *
 * $Revision: 17 $
 * $History: VS_ReadLicense.h $
 *
 * *****************  Version 17  *****************
 * User: Mushakov     Date: 24.07.12   Time: 18:20
 * Updated in $/VSNA/Servers/ServerServices
 *  - flag MULTIGATEWAY_ALLOWED = 0x8000 supported
 *
 * *****************  Version 16  *****************
 * User: Ktrushnikov  Date: 25.04.12   Time: 16:17
 * Updated in $/VSNA/Servers/ServerServices
 * #9715
 * - MaxParts fix at Join with conf_id at short and full formats
 * #7086
 * - Join to SpecConf at remote server is forbidden
 * - Join from remote server to SpecConf is declined
 *
 * *****************  Version 15  *****************
 * User: Ktrushnikov  Date: 10.05.11   Time: 19:52
 * Updated in $/VSNA/Servers/ServerServices
 * #8710: support for HD video flag
 * - set UR_COMM_HDVIDEO if HQ_ALLOW in license
 *
 * *****************  Version 14  *****************
 * User: Ktrushnikov  Date: 11.02.11   Time: 16:58
 * Updated in $/VSNA/Servers/ServerServices
 * VCS 3.2
 * - License: add Roaming users to OnlineUsers of our server
 * - Join & ReqInvite check for license for Roaming users
 *
 * *****************  Version 13  *****************
 * User: Ktrushnikov  Date: 12.11.10   Time: 11:55
 * Updated in $/VSNA/Servers/ServerServices
 * - VS_CheckLicense_MultiConferences() added
 *
 * *****************  Version 12  *****************
 * User: Ktrushnikov  Date: 11.10.10   Time: 14:26
 * Updated in $/VSNA/Servers/ServerServices
 * #7924: VIDEORECORDING bit in Rights by License
 * #7923: TarifRestrictions in VCS
 * - restrick in VCSConfRestrick
 * - send TARIFRESTR_PARAM to client
 *
 * *****************  Version 11  *****************
 * User: Mushakov     Date: 29.04.10   Time: 20:07
 * Updated in $/VSNA/Servers/ServerServices
 * - memory leaks removed;
 * - descktop sharing supported (server)
 *
 * *****************  Version 10  *****************
 * User: Mushakov     Date: 26.02.10   Time: 15:48
 * Updated in $/VSNA/Servers/ServerServices
 *  - mobile client supported (AuthService)
 * - new cert
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 15.02.10   Time: 20:02
 * Updated in $/VSNA/Servers/ServerServices
 * - enh #6964
 *
 * *****************  Version 8  *****************
 * User: Mushakov     Date: 3.01.10    Time: 13:51
 * Updated in $/VSNA/Servers/ServerServices
 * - LicenseEvents added
 * - restricted build in restrict interface included
 * - RESTRICTED_BUILD removed
 * - Reading "group allowed" flag from licenses added
 * - guest autorization in LDAP mode added
 *
 * *****************  Version 7  *****************
 * User: Mushakov     Date: 26.11.09   Time: 19:09
 * Updated in $/VSNA/Servers/ServerServices
 *  - init g_vcs_storage by getting Licenses
 *
 * *****************  Version 6  *****************
 * User: Ktrushnikov  Date: 6.11.09    Time: 19:55
 * Updated in $/VSNA/Servers/ServerServices
 * VS_ReadLicense
 * - fix with break
 * - LE_GUEST_LOGIN=9 added
 * VS_Storage
 * - m_num_guest - count of loggedin guests
 * VCSAuthService
 * - DeleteUser: guest from map ("registry")
 * VCSStorage
 * - DeleteUser added to interface
 * VS_RegistryStorage
 * - Guest login supported
 * - Check Guest password
 * - Check Count of Guests by License
 * - Read Shared Key from registry
 *
 * *****************  Version 5  *****************
 * User: Mushakov     Date: 4.11.09    Time: 18:49
 * Updated in $/VSNA/Servers/ServerServices
 *
 * *****************  Version 4  *****************
 * User: Mushakov     Date: 28.10.09   Time: 17:59
 * Updated in $/VSNA/Servers/ServerServices
 * - vs_ep_id removed
 * - registration corrected
 *
 * *****************  Version 3  *****************
 * User: Mushakov     Date: 23.10.09   Time: 15:05
 * Updated in $/VSNA/Servers/ServerServices
 *  - VCS 3
 *
 * *****************  Version 2  *****************
 * User: Mushakov     Date: 7.08.09    Time: 15:14
 * Updated in $/VSNA/Servers/ServerServices
 * - SBSv3 added by Matvey (SBSv3_m)
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 31.03.09   Time: 19:11
 * Created in $/VSNA/Servers/ServerServices
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Servers/MediaBroker/BrokerServices
 *
 * *****************  Version 8  *****************
 * User: Stass        Date: 30.10.06   Time: 18:46
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * added gateway counter
 *
 * *****************  Version 7  *****************
 * User: Stass        Date: 25.07.06   Time: 16:29
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * added roaming control in licenses
 *
 * *****************  Version 6  *****************
 * User: Stass        Date: 28.01.04   Time: 20:09
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * added license update logic
 *
 * *****************  Version 5  *****************
 * User: Stass        Date: 26.01.04   Time: 15:40
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * moved license things to lib
 *
 * *****************  Version 4  *****************
 * User: Stass        Date: 22.01.04   Time: 15:42
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * moved debug print funcs
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 23.12.03   Time: 16:59
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * added license reload
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 22.12.03   Time: 15:22
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * added New Conf license event
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 25.11.03   Time: 18:59
 * Created in $/VS/Servers/MediaBroker/BrokerServices
 * added licensing
 *
 ****************************************************************************/


#ifndef VS_READ_LICENSE_H
#define VS_READ_LICENSE_H

#include "../LicenseLib/VS_License.h"
#include "../common/tools/Server/VS_Server.h"
#include "../common/std/cpplib/VS_UserData.h"
#include "Common.h"
#include "../LicenseLib/VS_LicensesWrap.h"

#include <functional>

extern VS_LicensesWrap	*p_licWrap;

extern const char LIC_ONLINE_USERS_TAG[];
extern const char LIC_CONFERENCES_TAG[];
extern const char LIC_GATEWAYS_TAG[];
extern const char LIC_TERMINAL_PRO_USERS_TAG[];
extern const char LIC_SYMMETRIC_PARTICIPANTS_TAG[];
extern const char LIC_MAX_GUESTS_TAG[];
extern const char LIC_RESTRICTION_TAG[];
extern const char LIC_LIMITED_TAG[];
extern const char LIC_TRIAL_TIME_TAG[];
extern const char LIC_VALID_UNTIL_TAG[];
extern const char LIC_RELOAD_DATE_TAG[];
extern const char SERVER_SHUTDOWN_TIME_TAG[];


enum VS_LicenseEvents
{
	LE_LOGIN=1,
	LE_NEWCONFERENCE=2,
	LE_ROAMING_ON=3,
	LE_GATEWAYLOGIN=4,
	LE_TRIAL=5,
	LE_FILETRANSFER=6,
	LE_WHITEBOARD	=7,
	LE_SLIDESHOW	=8,
	LE_DSHARING,
	LE_GUEST_LOGIN ,
	LE_TERMINAL_LOGIN,
	LE_LDAP_ALLOWED,
	LE_ASYMMETRICCONF_ALLOWED,
	LE_ROLE_CONF_ALLOWED,
	LE_UDPMULTICAST_ALLOWED,
	LE_USER_GROUPS_ALLOWED,
	LE_SSL_STREAMS,
	LE_VIDEORECORDING,
	LE_HDVIDEO,
	LE_MULTIGATEWAY,
	LE_WEBRTC_BROADCAST,
	LE_IMPROVED_SECURITY,
	LE_ENABLE_DIRECTORY,
	LE_PAID_SERVER,
	LE_CONF_BROADCAST,
	LE_WEBINARS,
	LE_SDK_CLIENTS,
	LE_ENTERPRISE_MASTER,
	LE_ENTERPRISE_SLAVE
};

bool VS_ReadLicense(const VS_SimpleStr& myself,VS_License::Flags flags, VS_Container &cnt);
bool VS_CheckLicense(VS_LicenseEvents evt);
bool VS_CheckLicense_MultiConferences();
void VS_GetLicence_TarifRestrictions(int32_t tarif_opts[4], long Podiums = 4, bool IsUser = true, bool IsIntercom = false, bool IsPublic = false);
bool VS_CheckLicense_CheckRoamingParticipant();

using LicenseCheckFunctionT = std::function<bool(VS_LicenseEvents)>;

#endif // VS_READ_LICENSE_H
