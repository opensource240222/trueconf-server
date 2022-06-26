#include "VS_STUNClient.h"
#include "../STUNParserLib/VS_STUNField_Address.h"
#include "../STUNParserLib/VS_STUNField_Change.h"
#include "../SIPParserBase/VS_Const.h"
#include "std/cpplib/ThreadUtils.h"
#include "std-generic/cpplib/hton.h"
#include <process.h>

void * VS_STUNClient::iThis = 0;

unsigned int _stdcall STUNThread_GetTotalInfoAsync(void* lpParam);

bool VS_STUNClient::InitACS()
{
	if ( !VS_AcsLibInitial() )
		return false;

	return true;
}

bool VS_STUNClient::Init()
{
	if ( !InitACS() )
		return false;

	const unsigned long host_name_size = 512;
	char* host_name = new char[host_name_size];

	if ( !VS_GetDefaultHostName(host_name, host_name_size) )
	{
		if ( host_name ) { delete [] host_name; host_name = 0; }
		return false;
	}

	if ( !VS_GetIpByHostName(host_name, &iInternalIP) )
	{
		if ( host_name ) { delete [] host_name; host_name = 0; }
        return false;
	}
	if ( host_name ) { delete [] host_name; host_name = 0; }

	iIter = STUN_PORT - 1;

	return true;
}

VS_STUNClient* VS_STUNClient::Instance()
{
	if ( !iThis )
	{
		iThis = new VS_STUNClient;

		if ( !((VS_STUNClient*) iThis)->Init() )
		{
			if ( iThis ) { delete iThis; iThis = 0; }
			return 0;
		}
	}

	return (VS_STUNClient*) iThis;
}

VS_STUNClient::VS_STUNClient():
	iThreadParam(0), iThreadEvent(0), iThreadHandle(0),
	iIter(STUN_PORT - 1), iInternalIP(0),iThreadNeedToBeClosed(true)
{

}

VS_STUNClient::~VS_STUNClient()
{
	if ( iThreadParam )
	{
		delete iThreadParam;
		iThreadParam = 0;
	}

	if ( iThreadEvent )
	{
		CloseHandle(iThreadEvent);
		iThreadEvent = 0;
	}

	if ( iThreadHandle && iThreadNeedToBeClosed )
	{
		CloseHandle(iThreadHandle);
		iThreadHandle = 0;
	}

	if ( iThis )
		iThis = 0;
}

bool VS_STUNClient::MakeTest(const unsigned int aFlag, char* &aOutput, unsigned int &aSize) const
{
	VS_STUNField_Change* Change = new VS_STUNField_Change;

	Change->iChangeIP = (aFlag & STUN_MASK_CHANGE_IP) > 0;
	Change->iChangePort = (aFlag & STUN_MASK_CHANGE_PORT) > 0;
	Change->iType = STUN_FIELD_CHANGE_REQUEST;
	Change->iLength = 0x0004;
	Change->SetError(TSIPErrorCodes::e_ok);
	Change->SetValid(true);

    VS_STUNMetaField* Meta = new VS_STUNMetaField;

	Meta->iType = STUN_MESSAGE_TYPE_BINDING_REQUEST;

	Meta->iTransactionID = new char[16 + 1];
	memcpy(Meta->iTransactionID, STUN_TRANSACTION_ID, 16);
	Meta->iTransactionID[16] = '\0';

	Meta->iContainer.push_back(Change);

	int len = GetMetaLength(Meta);
	if (len < 0)
	{
		if ( Meta ) { delete Meta; Meta = 0; }

		return false;
	}

	Meta->iLength = len;

	Meta->SetError(TSIPErrorCodes::e_ok);
	Meta->SetValid(true);

	VS_SIPBuffer theBuffer;
	if (TSIPErrorCodes::e_ok != Meta->Encode(theBuffer) )
	{
		if ( Meta )
			delete Meta;

		return false;
	}

	if ( Meta )
		delete Meta;

	unsigned int sz = theBuffer.GetWriteIndex();

	if (aSize < sz)
	{
		aSize = sz;
		return false;
	}

	if (TSIPErrorCodes::e_ok != theBuffer.GetData(aOutput, sz) )
		return false;

	aSize = sz;

	return true;
}

int VS_STUNClient::GetMetaLength(const VS_STUNMetaField* aMeta) const
{
	if ( !aMeta )
		return -1;

	unsigned int sz = 0;
	sz = (unsigned int) aMeta->iContainer.size();

	if ( sz < 1 )
		return -1;

	unsigned int sum = 0;
	VS_STUNField* theField = 0;
	VS_BaseField* theBase = 0;
	for(unsigned int i=0; i < sz; i++)
	{
		theBase = aMeta->iContainer[i];

		if ( !theBase )
			return -1;

		theField = dynamic_cast<VS_STUNField*> (theBase);
		if ( !theField )
			return false;

		sum += theField->iLength;

		theBase = 0;
		theField = 0;
	}

	sum += sz * 4;		// Field Headers

	return sum;
}

bool VS_STUNClient::DoTest(unsigned int &aTimeOut,
						   const unsigned int aFlag,
						   const unsigned long aServerIP,
						   const unsigned short aServerPort,
						   const unsigned int aBindPort,
						   VS_STUNMetaField* &aMeta,
						   unsigned long* &aFromIP,
						   unsigned short* &aFromPort
						   )
{
	if ( !aMeta )
		return false;
	VS_ConnectionUDP * theUdpPtr = new VS_ConnectionUDP;
	if (!theUdpPtr)
		return false;

	VS_ConnectionUDP &theUDP  = *theUdpPtr;

	if ( !theUDP.IsValid() || !theUDP.CreateOvReadEvent() || !theUDP.CreateOvWriteEvent() )
	{
		if (theUdpPtr)
		{
			theUdpPtr->Close();
			delete theUdpPtr;
			theUdpPtr = 0;
		}
		return false;
	}

	char* host = new char[256];
	if ( !VS_GetDefaultHostName(host , 256) )
	{
		if ( host ) { delete [] host; host = 0; }
		if (theUdpPtr)
		{
			theUdpPtr->Close();
			delete theUdpPtr;
			theUdpPtr = 0;
		}
		return false;
	}

	if ( !theUDP.Bind(host, aBindPort,false) )
	{
		if ( host ) { delete [] host; host = 0; }
		if (theUdpPtr)
		{
			theUdpPtr->Close();
			delete theUdpPtr;
			theUdpPtr = 0;
		}
		return false;
	}
	if ( host ) { delete [] host; host = 0; }

	char* SendBuff = 0;
	unsigned int SendBuff_sz = 0;

	if ( !MakeTest(aFlag, SendBuff, SendBuff_sz) )
	{
		if (SendBuff_sz > 0)
		{
			SendBuff = new char[SendBuff_sz + 1];

			if ( !MakeTest(aFlag, SendBuff, SendBuff_sz) )
			{
				if ( SendBuff ) { delete [] SendBuff; SendBuff = 0; }
				if (theUdpPtr)
				{
					theUdpPtr->Close();
					delete theUdpPtr;
					theUdpPtr = 0;
				}

				return false;
			}
		}else
		{
			if (theUdpPtr)
			{
				theUdpPtr->Close();
				delete theUdpPtr;
				theUdpPtr = 0;
			}
			return false;
		}
	}

	int recvSize = 0;
	int sendSize = 0;

	if ( !theUDP.WriteTo(SendBuff, SendBuff_sz, aServerIP, aServerPort) )
	{
		if ( SendBuff ) { delete [] SendBuff; SendBuff = 0; }

		if (theUdpPtr)
		{
			theUdpPtr->Close();
			delete theUdpPtr;
			theUdpPtr = 0;
		}

		return false;
	}

	if ( SendBuff ) { delete [] SendBuff; SendBuff = 0; }

	unsigned long mills = aTimeOut;
	unsigned int RecvBuff_sz = 1024;
	void* RecvBuff = new char[RecvBuff_sz];
	char unused[128];

	if ( theUDP.AsynchReceiveFrom(RecvBuff, RecvBuff_sz, unused, aFromIP, aFromPort) == false)
	{
		if ( RecvBuff ) { delete [] RecvBuff; RecvBuff = 0; }
		if (theUdpPtr)
		{
			theUdpPtr->Close();
			delete theUdpPtr;
			theUdpPtr = 0;
		}
		return false;
	}

	VS_SIPBuffer InSIPBuff;
	DWORD res;
	unsigned int sz = 0;
	bool isOK = false;

	const unsigned int hEventsNum = 2;
	HANDLE hEvents[hEventsNum] = { theUDP.OvReadEvent(), theUDP.OvWriteEvent() };

	int i = 0;
	while( !isOK )
	{

		res = WaitForMultipleObjects(hEventsNum, hEvents, false, mills);
		switch( res )
		{
		case WAIT_OBJECT_0:
			recvSize = theUDP.GetReadResult(mills, 0, true);

			if (recvSize < 0)
			{
				if ( RecvBuff )
					delete RecvBuff;
				if (theUdpPtr)
				{
					theUdpPtr->Close();
					delete theUdpPtr;
					theUdpPtr = 0;
				}

                return false;
			}

			InSIPBuff.AddData((const char*) RecvBuff, recvSize);
			if ( RecvBuff ) { delete [] RecvBuff; RecvBuff = 0; }

			if ( aMeta->Decode(InSIPBuff) != TSIPErrorCodes::e_ok)
			{
				if (theUdpPtr)
				{
					theUdpPtr->Close();
					delete theUdpPtr;
					theUdpPtr = 0;
				}
				return false;
			}

			if ( strcmp(STUN_TRANSACTION_ID, aMeta->iTransactionID) != 0)
			{
				if (theUdpPtr)
				{
					theUdpPtr->Close();
					delete theUdpPtr;
					theUdpPtr = 0;
				}
				return false;
			}

			isOK = true;
//			return true;

			break;

		case WAIT_OBJECT_0 + 1:
			sendSize = theUDP.GetWriteResult(mills);

			if (sendSize != SendBuff_sz)
			{
				if ( RecvBuff ) { delete [] RecvBuff; RecvBuff = 0; }
				if (theUdpPtr)
				{
					theUdpPtr->Close();
					delete theUdpPtr;
					theUdpPtr = 0;
				}
				return false;
			}

			break;

		case WAIT_TIMEOUT:
				theUdpPtr->Close();
				++i;
				if (i==1)
					break;

		default:
				aTimeOut = 0;

				if ( RecvBuff ) { delete [] RecvBuff; RecvBuff = 0; }
				if (theUdpPtr)
				{
					theUdpPtr->Close();
					delete theUdpPtr;
					theUdpPtr = 0;
				}
				return false;
			break;
		}
	}
	if (theUdpPtr)
	{
		theUdpPtr->Close();
		delete theUdpPtr;
		theUdpPtr = 0;
	}

	return true;
}

bool VS_STUNClient::GetExternalIP(const unsigned int aTimeOut, unsigned long &aExternalIP)
{
	unsigned long ServerIP;
	if ( !VS_GetIpByHostName(STUN_SERVER_1, &ServerIP) )
		return false;

	unsigned int BindPort = GetNextBindPort();

	unsigned int mills = aTimeOut;
	unsigned long* FromIP = 0;
	unsigned short* FromPort = 0;
	VS_STUNMetaField* Meta = new VS_STUNMetaField;

	bool res = DoTest(mills, STUN_MASK_CHANGE_NO, ServerIP, STUN_PORT, BindPort, Meta, FromIP, FromPort);

	if ( res )
	{
		VS_BaseField* theBase = 0;
		VS_STUNField_Address* theAddr = 0;

		for(unsigned int i=0; i < Meta->iContainer.size(); i++)
		{
			theBase = Meta->iContainer[i];

			theAddr = dynamic_cast<VS_STUNField_Address*> (theBase);
			if ( theAddr )
			{
				if ( theAddr->iType == STUN_FIELD_ADDRESS_MAPPED )
				{
					aExternalIP = theAddr->iAddress;

					if ( Meta ) { delete Meta; Meta = 0; }

					return true;
				}
			}
		}
	}

	if ( Meta ) { delete Meta; Meta = 0; }

	return false;
}

bool VS_STUNClient::GetTypeOfNAT(const unsigned int aTimeOut, int &aTypeOfNAT)
{
	unsigned long ServerIP;
	if ( !VS_GetIpByHostName(STUN_SERVER_1, &ServerIP) )
		return false;

	unsigned int BindPort = GetNextBindPort();

 	unsigned int mills = aTimeOut / 4;
	unsigned long* FromIP = 0;
	unsigned short* FromPort = 0;
	unsigned long ExtIP1 = 0;
	unsigned short ExtPort1 = 0;
	unsigned long ExtIP2 = 0;
 	unsigned short ExtPort2 = 0;
	unsigned long ChangedIP = 0;
	unsigned short ChangedPort = 0;
 	VS_STUNMetaField* Meta = new VS_STUNMetaField;


	bool res = DoTest(mills, STUN_MASK_CHANGE_NO, ServerIP, STUN_PORT, BindPort, Meta, FromIP, FromPort);
	if ( !res || !mills )
	{
		if ( Meta ) { delete Meta; Meta = 0; }

		if ( !mills )
		{
			aTypeOfNAT = STUN_NAT_UDP_BLOCKED;
			return true;
		}else{
			return false;
		}
	}

	VS_BaseField* theBase = 0;
	VS_STUNField_Address* theAddr = 0;

	for(unsigned int i=0; i < Meta->iContainer.size(); i++)
	{
		theBase = Meta->iContainer[i];

		theAddr = dynamic_cast<VS_STUNField_Address*> (theBase);
		if ( theAddr )
		{
			if ( theAddr->iType == STUN_FIELD_ADDRESS_MAPPED )
			{
				ExtIP1 = theAddr->iAddress;
				ExtPort1 = theAddr->iPort;
			}

			if ( theAddr->iType == STUN_FIELD_ADDRESS_CHANGED )
			{
				ChangedIP = theAddr->iAddress;
				ChangedPort = theAddr->iPort;
			}
		}
	}

	if ( Meta ) { delete Meta; Meta = 0; }

	if (ExtIP1 == iInternalIP)		// Open Internet
	{
		FromIP = 0;
		FromPort = 0;
		Meta = new VS_STUNMetaField;

		bool res = DoTest(mills, STUN_MASK_CHANGE_IP | STUN_MASK_CHANGE_PORT, ServerIP, STUN_PORT, BindPort, Meta, FromIP, FromPort);

		if ( Meta ) { delete Meta; Meta = 0; }

		if ( res && mills )
			aTypeOfNAT = STUN_NAT_OPEN_INTERNET;
		else
			if ( !mills )
				aTypeOfNAT = STUN_NAT_SYMMETRIC_UDP_FIREWALL;
			else
                return false;

		return true;

	}else{		// Behind NAT

		FromIP = 0;
		FromPort = 0;
		Meta = new VS_STUNMetaField;

		bool res = DoTest(mills, STUN_MASK_CHANGE_IP | STUN_MASK_CHANGE_PORT, ServerIP, STUN_PORT, BindPort, Meta, FromIP, FromPort);

		if ( Meta ) { delete Meta; Meta = 0; }

		if ( res && mills )
		{
			if ( mills )
			{
				aTypeOfNAT = STUN_NAT_FULL_CONE;
				return true;
			}else{
				return false;
			}
		}else{
            if ( mills )
				return false;
		}

		if ( !ChangedIP || !ChangedPort )
			return false;

		ChangedIP = vs_htonl(ChangedIP);

		FromIP = 0;
		FromPort = 0;
		Meta = new VS_STUNMetaField;
		mills = aTimeOut / 4;

		res = DoTest(mills, STUN_MASK_CHANGE_NO, ChangedIP, ChangedPort, BindPort, Meta, FromIP, FromPort);

 		if ( !res || !mills )
		{
			if ( Meta ) { delete Meta; Meta = 0; }
			return false;
		}

		VS_BaseField* theBase = 0;
		VS_STUNField_Address* theAddr = 0;

		for(unsigned int i=0; i < Meta->iContainer.size(); i++)
		{
			theBase = Meta->iContainer[i];

			theAddr = dynamic_cast<VS_STUNField_Address*> (theBase);
			if ( theAddr )
			{
				if ( theAddr->iType == STUN_FIELD_ADDRESS_MAPPED )
				{
					ExtIP2 = theAddr->iAddress;
					ExtPort2 = theAddr->iPort;
				}
			}
		}

		if ( Meta ) { delete Meta; Meta = 0; }

		if ( (ExtIP1 != ExtIP2) || (ExtPort1 != ExtPort2) )
		{
			aTypeOfNAT = STUN_NAT_SYMMETRIC;
			return true;
		}

		// Restricted NAT
		FromIP = 0;
		FromPort = 0;
		Meta = new VS_STUNMetaField;

		res = DoTest(mills, STUN_MASK_CHANGE_PORT, ServerIP, STUN_PORT, BindPort, Meta, FromIP, FromPort);

		if ( Meta ) { delete Meta; Meta = 0; }

		if ( res && mills )
		{
			aTypeOfNAT = STUN_NAT_RESTRICTED_IP;
		}else{
			if ( !mills )
				aTypeOfNAT = STUN_NAT_RESTRICTED_PORT;
			else
				return false;
		}

		return true;
	}

	return false;
}

unsigned int VS_STUNClient::GetNextBindPort()
{
	if ((iIter + 1) < (STUN_PORT + STUN_LOCAL_PORT_RANGE) )
		iIter++;
	else
		iIter = STUN_PORT;

	return iIter;
}

bool VS_STUNClient::GetTotalInfo(const unsigned int aTimeOut, int &aTypeOfNAT, unsigned long &aExternalIP)
{
	unsigned int mills = aTimeOut / 2;

	bool res,res1 = GetTypeOfNAT(mills, aTypeOfNAT);

	if ( mills )
		res = GetExternalIP(mills, aExternalIP);
	else
		return false;
	if (!res1) aTypeOfNAT = -1;
	if ( res || res1 )
		return true;
	else
		return false;
}

bool VS_STUNClient::GetTotalInfoAsync(const unsigned int aTimeOut)
{
	if ( iThreadEvent || iThreadHandle )
		return false;

	iThreadEvent = CreateEvent(0, false, false, 0);

	if ( !iThreadEvent )
		return false;

	if ( iThreadParam ) { delete iThreadParam; iThreadParam = 0; }

	iThreadParam = new VS_STUNThreadParam;

	iThreadParam->iEvent = iThreadEvent;
	iThreadParam->iTimeOut = aTimeOut;

	unsigned int tid = 0;

	iThreadHandle = (HANDLE) _beginthreadex(0, 0, STUNThread_GetTotalInfoAsync, iThreadParam, 0, &tid);

	if ( !iThreadHandle )
	{
		CloseHandle(iThreadEvent);

		if ( iThreadParam ) { delete iThreadParam; iThreadParam = 0; }

		return false;
	}

	return true;
}

void VS_STUNClient::STUNMsgThread_GetTotalInfoAsync(void* lpParam)
{
	vs::SetThreadName("STUN_GetInfo");
	DWORD ThreadID = *((DWORD*) lpParam);
	delete lpParam;
	WPARAM wParam = -1;
	LPARAM lParam = 0;

	VS_STUNClient* STUNClient = VS_STUNClient::Instance();


	if ( !STUNClient->InitACS() )
	{
		if ( STUNClient ) { delete STUNClient; STUNClient = 0; }
		PostThreadMessage(ThreadID, WM_APP+98, wParam, lParam);
		return ;
	}
 	int TypeOfNAT = 0;
	unsigned long ExternalIP = 0;

	bool res = STUNClient->GetTotalInfo(STUN_TIMEOUT*2, TypeOfNAT, ExternalIP);
	STUNClient->iThreadNeedToBeClosed = false;

	if ( res )
		PostThreadMessage(ThreadID, WM_APP+98, TypeOfNAT, ExternalIP);
	else
		PostThreadMessage(ThreadID, WM_APP+98, wParam, lParam);

	return;
}
void VS_STUNClient::STUNMsgTranslate(WPARAM wParam,LPARAM lParam,char *aString,unsigned int aStringSize )
{

	if (-1==wParam && 0==lParam)
	{
		_snprintf(aString,aStringSize-1,"\n There is no information about your NAT.");
		return;
	}
	if (-1!=wParam && 0!=lParam)/// both
	{
		unsigned char * aPtr  = (unsigned char *)&lParam;
		switch(wParam)
		{
		case STUN_NAT_UDP_BLOCKED:
			_snprintf(aString,aStringSize-1,"Type of the NAT is a UDP blocked NAT. \n External ip is %u.%u.%u.%u.",aPtr[0],aPtr[1],aPtr[2],aPtr[3]);
			break;
		case STUN_NAT_SYMMETRIC_UDP_FIREWALL:
			_snprintf(aString,aStringSize-1,"The NAT is not existing.You have a Firewall that stops UDP.\n External ip is %u.%u.%u.%u.",aPtr[0],aPtr[1],aPtr[2],aPtr[3]);
			break;
		case STUN_NAT_OPEN_INTERNET:
			_snprintf(aString,aStringSize-1,"The NAT is not existing.You have direct internet ip address.\n External ip is %u.%u.%u.%u.",aPtr[0],aPtr[1],aPtr[2],aPtr[3]);
			break;
		case STUN_NAT_FULL_CONE:
			_snprintf(aString,aStringSize-1,"Type of the NAT is a full cone NAT.\n External ip is %u.%u.%u.%u.",aPtr[0],aPtr[2],aPtr[1],aPtr[0]);
			break;
		case STUN_NAT_SYMMETRIC:
			_snprintf(aString,aStringSize-1,"Type of the NAT is a symmetric NAT.\n External ip is %u.%u.%u.%u.",aPtr[0],aPtr[1],aPtr[2],aPtr[3]);
			break;
		case STUN_NAT_RESTRICTED_IP:
			_snprintf(aString,aStringSize-1,"Type of the NAT is a restricted ip NAT.\n External ip is %u.%u.%u.%u.",aPtr[0],aPtr[1],aPtr[2],aPtr[3]);
			break;
		case STUN_NAT_RESTRICTED_PORT:
			_snprintf(aString,aStringSize-1,"Type of the NAT is a restricted port NAT.\n External ip is %u.%u.%u.%u.",aPtr[0],aPtr[1],aPtr[2],aPtr[3]);
			break;
		default:
			break;
		}
		return;
	}
	if (-1 != wParam) /// type of nat
	{
		switch(wParam)
		{
		case STUN_NAT_UDP_BLOCKED:
			_snprintf(aString,aStringSize-1,"Type of the NAT is a UDP blocked NAT.");
			break;
		case STUN_NAT_SYMMETRIC_UDP_FIREWALL:
			_snprintf(aString,aStringSize-1,"The NAT is not existing.You have a Firewall that stops UDP.");
			break;
		case STUN_NAT_OPEN_INTERNET:
			_snprintf(aString,aStringSize-1,"The NAT is not existing.You have direct internet ip address.");
			break;
		case STUN_NAT_FULL_CONE:
			_snprintf(aString,aStringSize-1,"Type of the NAT is a full cone NAT.");
			break;
		case STUN_NAT_SYMMETRIC:
			_snprintf(aString,aStringSize-1,"Type of the NAT is a symmetric NAT.");
			break;
		case STUN_NAT_RESTRICTED_IP:
			_snprintf(aString,aStringSize-1,"Type of the NAT is a restricted ip NAT.");
			break;
		case STUN_NAT_RESTRICTED_PORT:
			_snprintf(aString,aStringSize-1,"Type of the NAT is a restricted port NAT.");
			break;
		default:
			break;
		}
		return;
	}
	if (0 != lParam) /// external IP
	{
		unsigned char * aPtr  = (unsigned char *)&lParam;
		_snprintf(aString,aStringSize-1,"External ip is %u.%u.%u.%u.",aPtr[0],aPtr[1],aPtr[2],aPtr[3]);
		return;
	}


}

unsigned int _stdcall STUNThread_GetTotalInfoAsync(void* lpParam)
{
	vs::SetThreadName("STUN_GetInfo");
	VS_STUNThreadParam* Param = (VS_STUNThreadParam*) lpParam;

	VS_STUNClient* STUNClient = VS_STUNClient::Instance();

	if ( !STUNClient->InitACS() )
	{
		if ( STUNClient ) { delete STUNClient; STUNClient = 0; }
		return -1;
	}

	bool res = STUNClient->GetTotalInfo(Param->iTimeOut,
		Param->iTypeOfNAT,
		Param->iExternalIP
	);

	if ( res )
		if ( !SetEvent(STUNClient->iThreadEvent) )
			return -1;

	return 0;
}

int VS_STUNClient::GetResult(const unsigned int aTimeOut, int &aTypeOfNAT, unsigned long &aExternalIP)
{
	if ( !iThreadEvent || !iThreadHandle )
		return -1;

	HANDLE Events[] = { iThreadEvent, iThreadHandle };

	DWORD r = WaitForMultipleObjects(2, Events, false, aTimeOut);

// Destruct
	CloseHandle(iThreadEvent);
	CloseHandle(iThreadHandle);

	iThreadEvent = 0;
	iThreadHandle = 0;

	if ( r == WAIT_OBJECT_0 )
	{
		if ( iThreadParam )
		{
			aExternalIP = iThreadParam->iExternalIP;
			aTypeOfNAT = iThreadParam->iTypeOfNAT;
		}
		return 0;
	}

	if ( iThreadParam ) { delete iThreadParam; iThreadParam = 0; }

	return -1;
}

bool VS_STUNClient::MsgTotalInfoAsync(DWORD aThreadID)
{
	unsigned int tid = 0;
	if (iThreadHandle!=0)
	{
		DWORD r =WaitForSingleObject(iThreadHandle,30);
		switch(r)
		{
		case WAIT_TIMEOUT:
			return false;
		case WAIT_OBJECT_0: break;
		default: break;
		}
		iThreadHandle = 0;
	}

	if ( iThreadHandle )
		return false;

	DWORD * a = new DWORD;
	*a = aThreadID;
	iThreadHandle = (HANDLE) _beginthread(STUNMsgThread_GetTotalInfoAsync, 0,a);

	if ( !iThreadHandle )
		return false;

	return true;
}