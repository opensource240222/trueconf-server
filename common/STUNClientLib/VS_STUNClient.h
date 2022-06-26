#pragma once

#include "../acs/connection/VS_ConnectionUDP.h"
#include "../acs/Lib/VS_AcsLib.h"
#include "../STUNParserLib/VS_STUNMetaField.h"

#include <Windows.h>

static const unsigned int STUN_PORT = 3478;
static const unsigned int STUN_LOCAL_PORT_RANGE = 10;
static const char* STUN_SERVER_1 = "stunserver.org";			// stun.fwdnet.net
static const char* STUN_TRANSACTION_ID = " TrueConf  STUN ";	// must be 16-bytes
static const unsigned int STUN_TIMEOUT = 10 * 1000;

struct VS_STUNThreadParam
{
	HANDLE iEvent;
	unsigned int iTimeOut;
	unsigned long iExternalIP;
	int iTypeOfNAT;
};

/*****************
*   Singleton    *
******************/
class VS_STUNClient
{
public:
	~VS_STUNClient();

	static VS_STUNClient* Instance();

	static bool InitACS();
	bool Init();

	bool GetExternalIP(const unsigned int aTimeOut,	unsigned long &aExternalIP);
	bool GetTypeOfNAT(const unsigned int aTimeOut, int &aTypeOfNAT);
	bool GetTotalInfo(const unsigned int aTimeOut, int &aTypeOfNAT, unsigned long &aExternalIP);
	bool GetTotalInfoAsync(const unsigned int aTimeOut);
	int GetResult(const unsigned int aTimeOut, int &aTypeOfNAT, unsigned long &aExternalIP);

	bool MsgTotalInfoAsync(DWORD aThreadID);
	static void STUNMsgThread_GetTotalInfoAsync(void* lpParam);
	static void STUNMsgTranslate(WPARAM wParam,LPARAM lParam,char *aString,unsigned int aStringSize);

protected:
	VS_STUNClient();

private:
	static void* iThis;

	friend unsigned int _stdcall STUNThread_GetTotalInfoAsync(void* lpParam);


	unsigned int iIter;
	unsigned long iInternalIP;

	HANDLE iThreadEvent;
	HANDLE iThreadHandle;
	VS_STUNThreadParam* iThreadParam;
	bool iThreadNeedToBeClosed;

	bool DoTest(unsigned int &aTimeOut,
		const unsigned int aFlag,
		const unsigned long aServerIP,
		const unsigned short aServerPort,
		const unsigned int aBindPort,
		VS_STUNMetaField* &aMeta,
		unsigned long* &aFromIP,
		unsigned short* &aFromPort
	);

	bool MakeTest(const unsigned int aFlag,	char* &aOutput,	unsigned int &aSize) const;
	int GetMetaLength(const VS_STUNMetaField* aMeta) const;
	unsigned int GetNextBindPort();
};