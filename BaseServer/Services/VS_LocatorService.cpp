/****************************************************************************
 * (c) 2007 Visicron Systems, Inc.  http://www.visicron.net/
 * $Revision: 17 $
 * $History: VS_LocatorService.cpp $
 *
 * *****************  Version 17  *****************
 * User: Ktrushnikov  Date: 21.12.11   Time: 18:43
 * Updated in $/VSNA/Servers/BaseServer/Services
 * LocatorSRV: reconnect to PostgreSQL
 * - ProcessDBCall() added
 *
 * *****************  Version 16  *****************
 * User: Mushakov     Date: 14.10.10   Time: 18:28
 * Updated in $/VSNA/Servers/BaseServer/Services
 * LOGIN_PARAM -> GetServerByLogin
 *
 * *****************  Version 15  *****************
 * User: Mushakov     Date: 15.06.10   Time: 15:32
 * Updated in $/VSNA/Servers/BaseServer/Services
 *  - sharding
 *
 * *****************  Version 14  *****************
 * User: Mushakov     Date: 11.06.08   Time: 18:45
 * Updated in $/VSNA/Servers/BaseServer/Services
 * additional logging ProcessCOMError added
 *
 * *****************  Version 13  *****************
 * User: Dront78      Date: 19.05.08   Time: 12:19
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - if condition fixed
 *
 * *****************  Version 12  *****************
 * User: Smirnov      Date: 16.05.08   Time: 19:29
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - fixed error with timeout in login messages
 *
 * *****************  Version 11  *****************
 * User: Stass        Date: 22.04.08   Time: 21:32
 * Updated in $/VSNA/Servers/BaseServer/Services
 * fix
 *
 * *****************  Version 10  *****************
 * User: Stass        Date: 22.04.08   Time: 21:27
 * Updated in $/VSNA/Servers/BaseServer/Services
 * fixed eof
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 13.03.08   Time: 16:47
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - memory leaks
 *
 * *****************  Version 8  *****************
 * User: Stass        Date: 20.02.08   Time: 12:45
 * Updated in $/VSNA/Servers/BaseServer/Services
 *
 * *****************  Version 7  *****************
 * User: Stass        Date: 20.02.08   Time: 12:09
 * Updated in $/VSNA/Servers/BaseServer/Services
 * fixed eof
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 19.02.08   Time: 21:04
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - auto connect to sm
 *
 * *****************  Version 5  *****************
 * User: Mushakov     Date: 12.02.08   Time: 19:13
 * Updated in $/VSNA/Servers/BaseServer/Services
 * GetAppProperties method realized
 *
 * *****************  Version 4  *****************
 * User: Stass        Date: 6.02.08    Time: 19:10
 * Updated in $/VSNA/Servers/BaseServer/Services
 * added fwd for no server in locator
 *
 * *****************  Version 3  *****************
 * User: Dront78      Date: 23.01.08   Time: 17:51
 * Updated in $/VSNA/Servers/BaseServer/Services
 * _malloca redefinition warning fixed.
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 13.12.07   Time: 12:04
 * Updated in $/VSNA/Servers/BaseServer/Services
 * added #bs, fixed Init
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 5.12.07    Time: 11:09
 * Created in $/VSNA/Servers/BaseServer/Services
 * BS - new iteration
 *
 ***********************************************/

#ifdef _WIN32	// not ported

#include <malloc.h>
#include <string.h>

#include "../../ServerServices/Common.h"

#include "VS_LocatorService.h"
#include "transport/Router/VS_RouterMessage.h"
#include "../../common/std/cpplib/VS_RegistryKey.h"
#include "../../common/std/cpplib/VS_RegistryConst.h"
#include "../../common/std/cpplib/VS_Protocol.h"
#include "../../common/std/cpplib/VS_MemoryLeak.h"
#include "../../common/std/VS_ProfileTools.h"
#include "../../common/std/debuglog/VS_Debug.h"
#include "storage/VS_LocatorStorage.h"

#define DEBUG_CURRENT_MODULE VS_DM_RESOLVE


VS_LocatorService::VS_LocatorService()
{
	m_TimeInterval = std::chrono::seconds(5);
	m_IsSmOnline = false;
	m_SmLastCheckTime = 0;
}

bool VS_LocatorService::Init( const char *our_endpoint, const char *our_service, const bool permittedAll )
{
	char buff[512];

	////////////////////////////////////////////////////////////
	// database init
	VS_RegistryKey    cfg_root(false, CONFIGURATION_KEY, false, true);
	if (!cfg_root.IsValid())
	{
		dprint0("registry not valid\n");
		return false;
	};
	if (!VS_LocatorStorage::Instance().IsValid())
		return false;

	if (cfg_root.GetValue(buff, 256, VS_REG_STRING_VT, SERVER_MANAGER_TAG) > 0 && *buff) {
		m_SM = buff;
	}
	else {
		dprint0("invalid server manager specified \n");
		return false;
	}
	ConnectServer(m_SM);

	return true;
}

bool VS_LocatorService::Processing( std::unique_ptr<VS_RouterMessage>&&recvMess)
{
	if (recvMess == 0)  return true;

	switch (recvMess->Type())
	{
	case transport::MessageType::Invalid:
		break;
	case transport::MessageType::Request:
	case transport::MessageType::Reply:
	{
		VS_SimpleStr service = recvMess->AddString();
		if (!service) {
			dprint1("no service to forward (msg from %s:%s)\n", recvMess->SrcServer(), recvMess->SrcUser());
			break;
		}

		VS_Container cnt;
		std::string server;
		const char* user = 0;
		string_view passwd;

		if (cnt.Deserialize(recvMess->Body(), recvMess->BodySize())) {
			if ((user = cnt.GetStrValueRef(LOGIN_PARAM)) != 0) {
				dprint3("locate by login %s srv %s\n", user, service.m_str);
				if (!*user)	break;

				const char *pwd = nullptr;
				if ((pwd = cnt.GetStrValueRef(PASSWORD_PARAM)) != 0 && *pwd) passwd = pwd;

				VS_LocatorStorage::Instance().GetServerByLogin(user, passwd, server);
			}
			else if ((user = cnt.GetStrValueRef(USERNAME_PARAM)) != 0 || (user = cnt.GetStrValueRef(QUERY_PARAM)) != 0) { // QUERY_PARAM for Search AB
				dprint3("locate by call_id %s srv %s\n", user, service.m_str);

				if(!*user)
					break;
				VS_LocatorStorage::Instance().GetServerByCallID(user, server);
			}
		}

		if (server.empty()) {
			dprint3(" no server found (msg from %s:%s)\n", recvMess->SrcServer(), recvMess->SrcUser());
			server = OurEndpoint();
		}

		recvMess->SetAddString("");
		recvMess->SetDstService((char*)service);
		recvMess->SetDstServer(server.c_str());
		auto m = recvMess.release();
		if (!PostMes(m))
			delete m;

		dprint3(" to server %s\n", server.c_str());
		break;
	}
	case transport::MessageType::Notify:
		break;
	}
	return true;
}

bool VS_LocatorService::Timer(unsigned long tickcount)
{
	if (tickcount - m_SmLastCheckTime > 30000) {
		if (!m_IsSmOnline)
			ConnectServer(m_SM);
		m_SmLastCheckTime = tickcount;
	}
	return true;
}


bool VS_LocatorService::OnPointConnected_Event(const VS_PointParams* prm)
{
	if (prm->reazon <= 0) {
		dprint1("Error {%d} while Connect to (%s)\n", prm->reazon, prm->uid);
	}
	else if (prm->type==VS_PointParams::PT_SERVER) {
		dprint2("Server Connect: uid=%s, reason: %2d\n", prm->uid, prm->reazon);
		if (m_SM==prm->uid) {
			if (!m_IsSmOnline) {
				dprint1("SM (%s) is UP\n", prm->uid);
			}
			else {
				dprint1("SM (%s) RECONNECT\n", prm->uid);
			}
			m_IsSmOnline = true;
		}
	}
	else {
		dprint1("NOT SERVER Connect!: uid=%s, reason: %2d\n", prm->uid, prm->reazon);
	}
	return true;
}

bool VS_LocatorService::OnPointDisconnected_Event(const VS_PointParams* prm)
{
	if (prm->type==VS_PointParams::PT_SERVER) {
		dprint2("Server DisConnect: uid=%s, reason: %2d\n", prm->uid, prm->reazon);
		if (m_SM==prm->uid) {
			m_IsSmOnline = false;
			dprint1("SM (%s) is DOWN\n", prm->uid);
		}
	}
	return true;
}
#endif