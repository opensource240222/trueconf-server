/**
 **************************************************************************
 * \file VSBwtClient.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Network bandwidth test
 *
 * \b Project Client
 * \author SMirnovK
 * \date 08.08.2003
 *
 * $Revision: 3 $
 *
 * $History: VSBwtClient.h $
 *
 * *****************  Version 3  *****************
 * User: Mushakov     Date: 6.03.12    Time: 16:16
 * Updated in $/VSNA/VSClient
 * - some folders renamed
 * - pathes was fixed
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 16.04.08   Time: 16:10
 * Updated in $/VSNA/VSClient
 * - VS_UPnPInteractor removed
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/VSClient
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 15  *****************
 * User: Smirnov      Date: 29.05.06   Time: 18:06
 * Updated in $/VS/VSClient
 * - UPnP and STUN messages
 *
 * *****************  Version 14  *****************
 * User: Smirnov      Date: 3.02.05    Time: 20:22
 * Updated in $/VS/VSClient
 * EcHo cancel repaired
 *
 * *****************  Version 13  *****************
 * User: Smirnov      Date: 17.12.04   Time: 20:22
 * Updated in $/VS/VSClient
 * header to files
 *
 * *****************  Version 12  *****************
 * User: Smirnov      Date: 25.11.03   Time: 19:07
 * Updated in $/VS/VSClient
 * removed often clicks bugs
 * added conference pings (protocol version now 9)
 * added check by conference pings (exparation timeouts)
 * added sending protocol version in some messages
 *
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 28.10.03   Time: 13:23
 * Updated in $/VS/VSClient
 * check ranning state of bwt
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 24.10.03   Time: 19:02
 * Updated in $/VS/VSClient
 * added bwt wizard mode
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 4.09.03    Time: 17:38
 * Updated in $/VS/VSClient
 * Net Test in Reg
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
 * User: Smirnov      Date: 12.08.03   Time: 13:48
 * Updated in $/VS/VSClient
 * removed checks in bwt
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 11.08.03   Time: 19:48
 * Updated in $/VS/VSClient
 * remove print
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
#ifndef VS_BWT_CLIENT_H
#define VS_BWT_CLIENT_H


/****************************************************************************
 * Includes
 ****************************************************************************/
#include <stdio.h>
#include <string.h>

#include "../Bwt/VS_Bwt.h"
#include "../acs/Lib/VS_AcsLib.h"
#include "../std/cpplib/VS_Map.h"
#include "../std/cpplib/VS_SimpleStr.h"
#include "../std/cpplib/VS_Lock.h"
#include "../STUNClientLib\VS_STUNClient.h"
#include <windows.h>


/****************************************************************************
 * Defines
 ****************************************************************************/
#define VSBWT_TESTTIME 8
#define VSBWT_FRAMESIZE 2000
#define VSBWT_PERIOD 1

/****************************************************************************
 * Structures
 ****************************************************************************/
/**
 **************************************************************************
 * \brief Message container send to GUI
 ****************************************************************************/
struct VSBwtClientMess
{
	char			Status[256];
	VS_BwtResult	Results;
	int				Progress;
	VSBwtClientMess(){ZeroMemory(this, sizeof(VSBwtClientMess));}
	VSBwtClientMess(const VSBwtClientMess &src){ *this = src;}
	VSBwtClientMess &operator =(const VSBwtClientMess &src) {
		strcpy(Status, src.Status);
		Results = src.Results;
		Progress = src.Progress;
		return *this;
	}
	static void* Factory(const void* upd) {	return new VSBwtClientMess(*(VSBwtClientMess*)upd); }
	static void Destructor(void* upd) {		delete (VSBwtClientMess*) upd;	}
};


/****************************************************************************
 * Classes
 ****************************************************************************/
/**
 **************************************************************************
 * \brief Network bandwidth test
 ****************************************************************************/
class VSBwtClient: public VS_BwtIntermediate, VS_Lock
{
public:
	enum BwtTest_Cond{
		BWT_TEST_BEGIN,
		BWT_TEST_PROGRESS,
		BWT_TEST_OK,
		BWT_TEST_ERR
	};
	VSBwtClient(DWORD Thread);
	~VSBwtClient();

	void WizardOn(int mode);
	int  Start(HWND hwnd, const char* ep = NULL);
	int  Stop();
	int  Get(VSBwtClientMess *OutRes, int Id);
private:
	bool Result(const unsigned status, const void *inf, const unsigned mark);
	bool Set(VSBwtClientMess &OutRes);
	int  GetId();
	void Post(int test_cond);
	const char* StrOutIn(const unsigned type);
	void ReadRegTest(VSBwtClientMess *mess);
	void WriteRegTest(VSBwtClientMess *mess);

public:
	char				m_Out[2560];

private:
	HWND				m_hwnd;
	DWORD				m_Thread;
	VS_BwtResult*		m_Results;
	bool				m_IsStarted;
	bool				m_IsRunning;
	int					m_StartTime;
	int					m_StartTTime;
	int					m_FinishTime;
	int					m_TestTime;
	VS_Map				m_Mess;
	int					m_Id;
	unsigned int		m_ret_code;
	bool				m_WizardOn;
	unsigned int		m_NumOfEpCon;
	VS_SimpleStr		m_TestedEp;
	VS_STUNClient*		m_StunTest;
};

#endif