/**
 **************************************************************************
 * \file VSBwtClient.cpp
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief VSBwtClient Implementation
 *
 * \b Project Client
 * \author SMirnovK
 * \date 08.08.2003
 *
 * $Revision: 7 $
 *
 * $History: VSBwtClient.cpp $
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 14.04.08   Time: 20:48
 * Updated in $/VSNA/VSClient
 * - network test now resent after login
 * - net config sent always
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 7.04.08    Time: 19:08
 * Updated in $/VSNA/VSClient
 * - hashed properties
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 26.02.08   Time: 16:54
 * Updated in $/VSNA/VSClient
 * - new servers coonect shceme
 *
 * *****************  Version 4  *****************
 * User: Dront78      Date: 18.01.08   Time: 17:02
 * Updated in $/VSNA/VSClient
 * - bigfixes from old architecture.
 * - ambiguous code marked as "old code"
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 16.01.08   Time: 15:15
 * Updated in $/VSNA/VSClient
 * - network settings repaired
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 29.11.07   Time: 18:42
 * Updated in $/VSNA/VSClient
 * - registry moved to NA in client
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/VSClient
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 20.03.07   Time: 12:37
 * Updated in $/VS2005/VSClient
 * - bug #1779
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 24  *****************
 * User: Smirnov      Date: 29.01.07   Time: 13:47
 * Updated in $/VS/VSClient
 * - small bugfix with report outBytes
 *
 * *****************  Version 23  *****************
 * User: Smirnov      Date: 22.06.06   Time: 19:54
 * Updated in $/VS/VSClient
 * - upnp OK!
 *
 * *****************  Version 22  *****************
 * User: Smirnov      Date: 29.05.06   Time: 20:45
 * Updated in $/VS/VSClient
 * - Stun Formatted
 *
 * *****************  Version 21  *****************
 * User: Smirnov      Date: 29.05.06   Time: 18:06
 * Updated in $/VS/VSClient
 * - UPnP and STUN messages
 *
 * *****************  Version 20  *****************
 * User: Melechko     Date: 10.05.06   Time: 16:52
 * Updated in $/VS/VSClient
 *
 * *****************  Version 19  *****************
 * User: Smirnov      Date: 17.12.04   Time: 20:22
 * Updated in $/VS/VSClient
 * header to files
 *
 * *****************  Version 18  *****************
 * User: Smirnov      Date: 29.03.04   Time: 20:11
 * Updated in $/VS/VSClient
 * int bwt test № changed to #
 *
 * *****************  Version 17  *****************
 * User: Smirnov      Date: 25.11.03   Time: 19:07
 * Updated in $/VS/VSClient
 * removed often clicks bugs
 * added conference pings (protocol version now 9)
 * added check by conference pings (exparation timeouts)
 * added sending protocol version in some messages
 *
 * *****************  Version 16  *****************
 * User: Smirnov      Date: 28.10.03   Time: 13:23
 * Updated in $/VS/VSClient
 * check ranning state of bwt
 *
 * *****************  Version 15  *****************
 * User: Smirnov      Date: 27.10.03   Time: 15:06
 * Updated in $/VS/VSClient
 * bwt wizard off
 *
 * *****************  Version 14  *****************
 * User: Smirnov      Date: 24.10.03   Time: 19:02
 * Updated in $/VS/VSClient
 * added bwt wizard mode
 *
 * *****************  Version 13  *****************
 * User: Smirnov      Date: 17.10.03   Time: 11:51
 * Updated in $/VS/VSClient
 * wrong tcp fixed in bwt
 *
 * *****************  Version 12  *****************
 * User: Smirnov      Date: 16.09.03   Time: 13:21
 * Updated in $/VS/VSClient
 * grammatic
 *
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 10.09.03   Time: 15:52
 * Updated in $/VS/VSClient
 * Failed bwt test in registry
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 4.09.03    Time: 17:38
 * Updated in $/VS/VSClient
 * Net Test in Reg
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 28.08.03   Time: 13:41
 * Updated in $/VS/VSClient
 * added date in properties net test
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 26.08.03   Time: 18:06
 * Updated in $/VS/VSClient
 * set property "Network Test"
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 26.08.03   Time: 16:23
 * Updated in $/VS/VSClient
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 15.08.03   Time: 18:11
 * Updated in $/VS/VSClient
 * extended info in log
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 14.08.03   Time: 18:42
 * Updated in $/VS/VSClient
 * check new broker for valid configuration,
 * added new messages in bwt
 * proxy set rewrited
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 12.08.03   Time: 13:48
 * Updated in $/VS/VSClient
 * removed checks in bwt
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 11.08.03   Time: 19:11
 * Updated in $/VS/VSClient
 * critical section in bwt
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 11.08.03   Time: 16:50
 * Updated in $/VS/VSClient
 * Bwt Interface
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 8.08.03    Time: 19:54
 * Created in $/VS/VSClient
 * Bwt test
 ****************************************************************************/

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "VSBwtClient.h"
#include "VS_ApplicationInfo.h"
#include "../net/EndpointRegistry.h"
#include "../std/cpplib/VS_RegistryKey.h"

#include <time.h>
#include <Windows.h>
#include <mmsystem.h>

/****************************************************************************
 * Classes
 ****************************************************************************/
VSBwtClient::VSBwtClient(DWORD Thread)
{
	m_Thread = Thread; // implementation owner
	m_hwnd = NULL;
	m_Results = new VS_BwtResult;
	memset(m_Results, 0, sizeof(VS_BwtResult));
	m_IsStarted = false;
	m_IsRunning = false;
	m_StartTime = 0;
	m_StartTTime = 0;
	m_FinishTime = 0;
	m_TestTime = 0;
	m_Id = -1;
	m_Mess.SetDataFactory(VSBwtClientMess::Factory, VSBwtClientMess::Destructor);
	m_ret_code = VS_BWT_RET_OK;
	m_WizardOn = false;
	m_NumOfEpCon = 0;
	m_StunTest = VS_STUNClient::Instance();
	m_Out[0] = 0;
}

VSBwtClient::~VSBwtClient()
{
	if (m_Results) delete m_Results;
}

void VSBwtClient::WizardOn(int mode)
{
	m_WizardOn = mode!=0;
}

int VSBwtClient::Start(HWND hwnd, const char* ep)
{
	VS_AutoLock lock(this);
	if (m_IsRunning) return 1;
	if (!ep) { // read from registry
		char value[256];
		if (!VS_ReadAS(value))
			return 3;
		m_TestedEp = value;
	}
	else
		m_TestedEp = ep;
	m_NumOfEpCon = net::endpoint::GetCountConnectTCP(m_TestedEp.m_str);

	m_hwnd = hwnd;
	m_Mess.Clear();
	m_StartTime = timeGetTime();
	m_TestTime = VSBWT_TESTTIME*1000;
	memset(m_Results, 0, sizeof(VS_BwtResult));
	if (!VS_BwtAsync(NULL, m_TestedEp, m_Results, this, VS_BWT_MODE_DUPLEX, m_TestTime/1000, VSBWT_FRAMESIZE, VSBWT_PERIOD))
		return 4;
	m_IsStarted = true;
	m_IsRunning = true;
	DWORD val = m_Thread;
	if (m_StunTest)
		m_StunTest->MsgTotalInfoAsync(val);
	return 0;
}

int VSBwtClient::Stop()
{
	VS_AutoLock lock(this);
	int iIsStarted=m_IsStarted?0:-1;
	m_IsStarted = false;
	return iIsStarted;
}

int VSBwtClient::Get(VSBwtClientMess *OutRes, int Id)
{
	VS_AutoLock lock(this);
	if (!Id) {
		ZeroMemory(OutRes, sizeof(VSBwtClientMess));
		ReadRegTest(OutRes);
		return 0;
	}

	VS_Map::Iterator i = m_Mess.Find((void*)Id);
	if (i!=m_Mess.End()) {
		*OutRes = *(VSBwtClientMess*)(*i).data;
		return 0;
	}
	return 1;
}

bool VSBwtClient::Result(const unsigned status, const void *inf, const unsigned mark )
{
	VS_AutoLock lock(this);
	int PassedTime;
	BwtTest_Cond cond = BWT_TEST_ERR;
	VSBwtClientMess mess;
	VS_BwtResult* BwtResult = NULL;
	switch(status)
	{
	case VS_BWT_ST_INTER_RESULT:			// inf = &VS_BwtResult, mark - unused
		PassedTime = timeGetTime() - m_StartTTime + 10;
		mess.Progress = PassedTime*100/(m_TestTime+1);
		mess.Results = *(VS_BwtResult*)inf;
		sprintf(mess.Status, "Test in progress");
		cond = BWT_TEST_PROGRESS;
		break;
	case VS_BWT_ST_START_CONNECT:			// inf - unused, mark = 0-outbound/1-inbound
		sprintf(mess.Status, "%s connection is started", StrOutIn(mark));
		cond = BWT_TEST_BEGIN;
		break;
	case VS_BWT_ST_CONNECT_ATTEMPT:			// inf = &net::endpoint::ConnectTCP, mark = number
	{
		if (m_WizardOn && mark>m_NumOfEpCon) { // stop on second repeat
			Stop();
			break;
		}
		auto tcp = static_cast<const net::endpoint::ConnectTCP*>(inf);
		char host[64];
		if (!tcp->host.empty())
			strncpy(host, tcp->host.c_str(), 63);
		else
			strncpy(host, "<none>", 63);
		host[63] = 0;
		char protocol[10];
		if (!tcp->protocol_name.empty())
			strncpy(protocol, tcp->protocol_name.c_str(), 9);
		else
			strncpy(protocol, "<none>", 9);
		protocol[9] = 0;
		sprintf(mess.Status, "attempt #%2u: host %s, port %u, %s", mark, host, tcp->port, protocol);
		cond = BWT_TEST_BEGIN;
	}
		break;
	case VS_BWT_ST_CONNECT_OK:				// inf - unused, mark = 0-outbound/1-inbound
		sprintf(mess.Status, "%s connection is established", StrOutIn(mark));
		cond = BWT_TEST_BEGIN;
		break;
	case VS_BWT_ST_CONNECT_ERROR:			// inf - unused, mark = 0-outbound/1-inbound
		sprintf(mess.Status, "%s connection error", StrOutIn(mark));
		cond = BWT_TEST_ERR;
		break;
	case VS_BWT_ST_START_HANDSHAKE:			// inf - unused, mark = 0-outbound/1-inbound
		sprintf(mess.Status, "starting %s handshake", StrOutIn(mark));
		cond = BWT_TEST_BEGIN;
		break;
	case VS_BWT_ST_HANDSHAKE_OK:			// inf - unused, mark = 0-outbound/1-inbound
		sprintf(mess.Status, "%s handshake is passed Ok", StrOutIn(mark));
		cond = BWT_TEST_BEGIN;
		break;
	case VS_BWT_ST_HANDSHAKE_ERROR:			// inf - unused, mark = 0-нет соединения,
											//						1-неверен endpoint,
											//						2-нет ресурсов
		sprintf(mess.Status, "handshake error #%u", mark);
		cond = BWT_TEST_ERR;
		break;
	case VS_BWT_ST_NO_RESOURCES:			// inf - unused, mark = 0-outbound/1-inbound
		sprintf(mess.Status, "no resources for %s connection", StrOutIn(mark));
		cond = BWT_TEST_ERR;
		break;
	case VS_BWT_ST_START_TEST:				// inf - unused, mark = unused
		m_StartTTime = timeGetTime();
		sprintf(mess.Status, "Test in progress");
		cond = BWT_TEST_PROGRESS;
		break;
	case VS_BWT_ST_FINISH_TEST:				// inf - unused, mark = unused
		sprintf(mess.Status, "Test is finished");
		mess.Progress = 100;
		mess.Results = *m_Results;
		cond = BWT_TEST_OK;
		break;
	case VS_BWT_ST_CONNECTION_DIED:			// inf - unused, mark = 0-outbound/1-inbound
		sprintf(mess.Status, "Test was not finished properly");
		cond = BWT_TEST_ERR;
		break;
	case VS_BWT_ST_CONNECTION_ERROR:		// inf - unused, mark = 0-outbound/1-inbound
		sprintf(mess.Status, "Test was not finished properly");
		cond = BWT_TEST_ERR;
		break;
	default:
		m_IsStarted = false;
		break;
	}
	if (!m_IsStarted) {
		if		(cond == BWT_TEST_BEGIN)
			cond = BWT_TEST_ERR;
		else if (cond == BWT_TEST_PROGRESS)
			cond = BWT_TEST_OK;
		sprintf(mess.Status, "Test was stopped by user");
	}
	Set(mess);
	Post(cond);
	return m_IsStarted;
}

bool VSBwtClient::Set(VSBwtClientMess &OutRes)
{
	m_Id++;
	return m_Mess.Insert((void*)m_Id, &OutRes);
}

int VSBwtClient::GetId()
{
	return m_Id;
}

void VSBwtClient::Post(int test_cond)
{
	if (test_cond==BWT_TEST_OK) {
		VS_BwtResult bwt = *m_Results;
		char bufferIn[2560]; *bufferIn = 0;
		char bufferOut[2560]; *bufferOut = 0;

		sprintf(bufferOut, "%-16s |     IN  |     OUT   |\n", "Parameter");
		strcat(bufferIn, bufferOut);
		sprintf(bufferOut, "---------------------------------------|\n");
		strcat(bufferIn, bufferOut);

		sprintf(bufferOut, "%-16s |  %5d  |   %5d   |\n", "Bitrate, kBit", (int)(bwt.in_bps/128.), (int)(bwt.out_bps/128.));
		strcat(bufferIn, bufferOut);
		sprintf(bufferOut, "%-16s |   %4d  |    %4d   |\n", "Total, kB", (int)(bwt.in_bytes/1024.), (int)(bwt.out_bytes/1024.));
		strcat(bufferIn, bufferOut);
		sprintf(bufferOut, "%-16s |   %4ld  |    %4ld   |\n", "Jitter Max, ms", bwt.in_jitter_max_ms, bwt.out_jitter_max_ms);
		strcat(bufferIn, bufferOut);
		sprintf(bufferOut, "%-16s |   %4ld  |    %4ld   |\n", "Jitter Min, ms", bwt.in_jitter_min_ms, bwt.out_jitter_min_ms);
		strcat(bufferIn, bufferOut);
		sprintf(bufferOut, "%-16s |   %4ld  |    %4ld   |\n", "Response, ms", bwt.in_response_ms, bwt.out_response_ms);
		strcat(bufferIn, bufferOut);
		sprintf(bufferOut, "---------------------------------------|\n");
		strcat(bufferIn, bufferOut);

		time_t timeNow;
		time(&timeNow);
		sprintf(bufferOut, "%-16s%s", "Date:", ctime(&timeNow));
		strcat(bufferIn, bufferOut);

		strcpy(m_Out, bufferIn);
		PostThreadMessage(m_Thread, WM_USER + 22, 0, 0);
		// now save results in registry
		VSBwtClientMess mess;
		strncpy(mess.Status, ctime(&timeNow), 63);
		mess.Results = *m_Results;
		WriteRegTest(&mess);
		m_IsRunning = false;
	}
	else if (test_cond==BWT_TEST_ERR) {
		// now save results in registry
		time_t timeNow;
		time(&timeNow);
		VSBwtClientMess mess;
		strncpy(mess.Status, ctime(&timeNow), 63);
		WriteRegTest(&mess);
		m_IsRunning = false;
	}
	PostMessage(m_hwnd, WM_USER + 22, GetId(), test_cond);
}

const char* VSBwtClient::StrOutIn(const unsigned type)
{
	return !type ? "outbound" : "inbound";
}

void VSBwtClient::ReadRegTest(VSBwtClientMess *mess)
{
	char server[256];
	char keyname[256];
	if (VS_ReadAS(server)) {
		sprintf(keyname, "%s\\%s", REG_Servers, server);
		VS_RegistryKey key2(true, keyname);
		key2.GetValue(mess, sizeof(VSBwtClientMess), VS_REG_BINARY_VT, "NetworkTest");
	}
}

void VSBwtClient::WriteRegTest(VSBwtClientMess *mess)
{
	char server[256];
	char keyname[256];
	if (VS_ReadAS(server)) {
		sprintf(keyname, "%s\\%s", REG_Servers, server);
		VS_RegistryKey key2(true, keyname, false);
		key2.SetValue(mess, sizeof(VSBwtClientMess), VS_REG_BINARY_VT, "NetworkTest");
	}
}
