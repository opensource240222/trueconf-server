/****************************************************************************
 * (c) 2002 Visicron Inc.  http://www.visicron.net/
 *
 * Project:
 *
 * $Revision: 11 $
 * $History: ClientInterface.cpp $
 *
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 23.08.10   Time: 22:19
 * Updated in $/VSNA/^ClientInterface
 * - long names in devices
 * - corrected Wide names for devices
 * - init devices section rewrited
 *
 * *****************  Version 10  *****************
 * User: Ktrushnikov  Date: 7.05.10    Time: 13:04
 * Updated in $/VSNA/^ClientInterface
 * [#7304]
 * - xcl.dll unloading removed
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 22.12.09   Time: 14:25
 * Updated in $/VSNA/^ClientInterface
 * - try to bugfix#4638
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 6.11.09    Time: 12:50
 * Updated in $/VSNA/^ClientInterface
 * - static SetServer
 * - pareicipants number in conference info
 *
 * *****************  Version 7  *****************
 * User: Dront78      Date: 14.06.09   Time: 10:49
 * Updated in $/VSNA/^ClientInterface
 * - VZOchat7 merge. see VZOchat7.h for details
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 1.04.09    Time: 19:23
 * Updated in $/VSNA/^ClientInterface
 * - bugfix #5815
 *
 * *****************  Version 5  *****************
 * User: Dront78      Date: 24.03.09   Time: 16:53
 * Updated in $/VSNA/^ClientInterface
 * - VZOCHAT7 VSClient support added
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 19.03.08   Time: 17:50
 * Updated in $/VSNA/^ClientInterface
 * - Login interface extended
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 6.03.08    Time: 16:05
 * Updated in $/VSNA/^ClientInterface
 * - chat rewrited
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 26.02.08   Time: 16:54
 * Updated in $/VSNA/^ClientInterface
 * - new servers coonect shceme
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:48
 * Created in $/VSNA/^ClientInterface
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:50
 * Created in $/VS2005/^ClientInterface
 *
 * *****************  Version 118  *****************
 * User: Smirnov      Date: 15.11.06   Time: 18:06
 * Updated in $/VS/^ClientInterface
 * - debug info
 *
 * *****************  Version 117  *****************
 * User: Smirnov      Date: 2.08.06    Time: 13:35
 * Updated in $/VS/^ClientInterface
 * - address book and statuses processing removed
 * - added additional interface to receive container by GUI
 * - added hash param in SearchAB
 *
 * *****************  Version 116  *****************
 * User: Smirnov      Date: 21.06.06   Time: 11:06
 * Updated in $/VS/^ClientInterface
 * - autodiscovery incalcated
 *
 * *****************  Version 115  *****************
 * User: Melechko     Date: 14.06.06   Time: 17:09
 * Updated in $/VS/^ClientInterface
 * Add szHomeDomain param
 *
 * *****************  Version 114  *****************
 * User: Smirnov      Date: 29.05.06   Time: 18:04
 * Updated in $/VS/^ClientInterface
 * - on Com error return zero value
 *
 * *****************  Version 113  *****************
 * User: Smirnov      Date: 22.05.06   Time: 12:07
 * Updated in $/VS/^ClientInterface
 * - set/get host-port for current connection
 *
 * *****************  Version 112  *****************
 * User: Smirnov      Date: 25.04.06   Time: 19:38
 * Updated in $/VS/^ClientInterface
 * - removed old code, Thread class changed
 *
 * *****************  Version 111  *****************
 * User: Smirnov      Date: 20.04.06   Time: 13:46
 * Updated in $/VS/^ClientInterface
 * - new audio hardware test
 *
 * *****************  Version 110  *****************
 * User: Smirnov      Date: 14.04.06   Time: 18:43
 * Updated in $/VS/^ClientInterface
 * - on port update force reconnect to server throw home broker
 * - on network settings update force reconnect to server using current
 * broker
 *
 * *****************  Version 109  *****************
 * User: Melechko     Date: 21.06.05   Time: 13:06
 * Updated in $/VS/^ClientInterface
 * fix external callback
 *
 * *****************  Version 108  *****************
 * User: Melechko     Date: 16.06.05   Time: 16:13
 * Updated in $/VS/^ClientInterface
 * Add check User in Multiconference
 *
 * *****************  Version 107  *****************
 * User: Melechko     Date: 4/12/05    Time: 13:10
 * Updated in $/VS/^ClientInterface
 *
 * *****************  Version 106  *****************
 * User: Melechko     Date: 24.01.05   Time: 15:18
 * Updated in $/VS/^ClientInterface
 * FixSaturation
 *
 * *****************  Version 105  *****************
 * User: Melechko     Date: 20.01.05   Time: 18:31
 * Updated in $/VS/^ClientInterface
 * Remove unused function
 *
 * *****************  Version 104  *****************
 * User: Melechko     Date: 19.01.05   Time: 17:00
 * Updated in $/VS/^ClientInterface
 * some changes :)
 *
 ****************************************************************************/
#include "ClientInterface.h"
#include <windows.h>
#include "../acs/Lib/VS_AcsLib.h"
#include "../VSClient/VS_ApplicationInfo.h"
#include "../VSClient/VS_Dmodule.h"
#include "VSClient.h"
#include "../VSClient/VSProxySettings.h"

#define CURRENT_VERSION 1000
/**********************************************************************************************************/
DWORD Process(DWORD_PTR lParam, int Operation, char*szInterface, VARIANT *pVar){
	DTRACE(VSTM_PROC, "PROCESS: %s, OP=%d", szInterface, Operation);
	VSClient *vsc=(VSClient *)lParam;

	{
		std::string sbase(szInterface);
		std::string sfind("\\VideoCapture\\");
		std::string::size_type p = sbase.find(sfind);
		if (p != sbase.npos) {
			sbase.erase(p, sfind.length() - 1);
			strcpy(szInterface, sbase.c_str());
		}
	}

	DWORD ret=vsc->Process((VS_INTERFACE_OPERATION)Operation,szInterface,pVar);
	if((pVar->vt&VT_INT)==VT_INT){
		pVar->vt^=VT_INT;
		pVar->vt|=VT_I4;
	}
	if((pVar->vt&VT_UINT)==VT_UINT){
		pVar->vt^=VT_UINT;
		pVar->vt|=VT_UI4;
	}
	if(pVar->vt==VT_EMPTY){
		_variant_t *var=(_variant_t *)pVar;
		*var=long(0);
	}
	return ret;
};
/**********************************************************************************************************/
void SetJitter(DWORD_PTR lParam,int Mode){
  VSClient *vsc=(VSClient *)lParam;
  vsc->SetJitter(Mode);
};
/**********************************************************************************************************/
DWORD GetProperty(DWORD_PTR lParam,DWORD_PTR lParam1,DWORD_PTR lParam2){
  VSClient *vsc=(VSClient *)lParam;
  return vsc->GetProperty((char*)lParam1,(char*)lParam2);
};
/**********************************************************************************************************/
DWORD AddressBook(DWORD_PTR lParam,DWORD abCommand,char*param,long addressBook, long hash){
  VSClient *vsc=(VSClient *)lParam;
  return vsc->AddressBook(abCommand,param,addressBook, hash);
};
/**********************************************************************************************************/
int GetChatMessage( DWORD_PTR lParam,int Id, char* From, char* Mess, char* to, char* Dn){
	VSClient *vsc=(VSClient *)lParam;
	return vsc->GetChatMessage(Id, From, Mess, to, Dn);
};
/**********************************************************************************************************/
int GetChatMessageV2( DWORD_PTR lParam,int Id, char* From, char* Mess, char* to, char* Dn, long long *time)
{
	VSClient *vsc=(VSClient *)lParam;
	return vsc->GetChatMessageV2(Id, From, Mess, to, Dn, time);
};
/**********************************************************************************************************/
void GetCommandMessage(DWORD_PTR lParam,int Id, char* From, char* Command){
	VSClient *vsc=(VSClient *)lParam;
	vsc->GetCommandMessage(Id, From, Command);
};
/**********************************************************************************************************/
void GetTraffic(DWORD_PTR lParam,DWORD_PTR lParam1,DWORD_PTR lParam2){
    VSClient *vsc=(VSClient *)lParam;
    _variant_t var;
    int *pSnd=(int*)lParam1,*pRcv=(int*)lParam2;
    *pSnd=vsc->GetSenderTraffic();
	var=0;
    vsc->Process(GET_PARAM,"Receivers\\GetReceivedData",&var);
    *pRcv=var;

};
/**********************************************************************************************************/
int InterfaceGetVersion(DWORD_PTR pGUIVersion,DWORD_PTR pAPP_version){
	VS_SetAppVer((char*)pGUIVersion, (char*)pAPP_version);
    return CURRENT_VERSION;
};
/**********************************************************************************************************/
void GetSkinName(DWORD_PTR lParam,DWORD_PTR FileName){
  VSClient *vsc=(VSClient *)lParam;
  vsc->GetSkinName((char*)FileName);
};
/**********************************************************************************************************/
void SetSkinName(DWORD_PTR lParam,DWORD_PTR FileName){
  VSClient *vsc=(VSClient *)lParam;
  vsc->SetSkinName((char*)FileName);
};
/**********************************************************************************************************/
void BanUser(DWORD_PTR lParam,char*UserID,int Strong){
  VSClient *vsc=(VSClient *)lParam;
  vsc->BanUser(UserID,Strong);
};
/**********************************************************************************************************/
void SendMessages(DWORD_PTR lParam,DWORD_PTR pMessage,void* szOther){
  VSClient *vsc=(VSClient *)lParam;
  vsc->SendMessage((void*)pMessage,szOther);
};
/**********************************************************************************************************/
void SendCommands(DWORD_PTR lParam, char* command, char* to){
  VSClient *vsc=(VSClient *)lParam;
  vsc->SendCommand(command, to);
};
/**********************************************************************************************************/
void ConnectSender(DWORD_PTR lParam,HWND hReportHwnd,DWORD_PTR Type){
    VSClient *vsc=(VSClient *)lParam;
    vsc->ConnectSender(hReportHwnd,Type);
};
/**********************************************************************************************************/
void DisconnectSender(DWORD_PTR lParam){
    VSClient *vsc=(VSClient *)lParam;
    vsc->DisconnectSender();
};
/**********************************************************************************************************/
int StartSender(DWORD_PTR lParam,int Width,int Height,int Freq){
    VSClient *vsc=(VSClient *)lParam;
    return vsc->InitSender( Width, Height,Freq);
};
/**********************************************************************************************************/
void StopSender(DWORD_PTR lParam){
    VSClient *vsc=(VSClient *)lParam;
    vsc->ReleaseSender();
    return;
};
/**********************************************************************************************************/
DWORD BwtStart(DWORD_PTR lParam, DWORD_PTR hwnd){
  VSClient *vsc=(VSClient *)lParam;
  return vsc->BwtStart((HWND)hwnd);
}
DWORD BwtStop(DWORD_PTR lParam){
  VSClient *vsc=(VSClient *)lParam;
  return vsc->BwtStop();
}
DWORD BwtGet(DWORD_PTR lParam, DWORD_PTR Status, DWORD_PTR Id){
  VSClient *vsc=(VSClient *)lParam;
  return vsc->BwtGet((void*)Status, Id);
}
void BwtWizardOn(DWORD_PTR lParam, int mode){
  VSClient *vsc=(VSClient *)lParam;
  vsc->BwtWizardOn(mode);
}

DWORD SetProxySet(DWORD_PTR array){
	void ** p = (void**)array;
	return VSProxySettings::Set((int*)p[0], (char*)p[1], (unsigned short*)p[2], (char*)p[3], (char*)p[4]);
}
void GetProxySet(DWORD_PTR array){
	void ** p = (void**)array;
	VSProxySettings::Get((int*)p[0], (char*)p[1], (unsigned short*)p[2], (char*)p[3], (char*)p[4]);
}
void SetProxyDialog(DWORD_PTR func){
	VSProxySettings::SetDialog((void*)func);
}
void ProxyDialogEnd(int ret){
	VSProxySettings::DialogEnd(ret);
}
void SetProxyNetMode(int* cfg){
	VSProxySettings::SetNetMode(cfg, true);
}
int SetNetType(int mode){
	return VSProxySettings::SetNetType(mode);
}
int  SetManualPort(unsigned short *port, char* host, int mode){
	return VSProxySettings::SetManualPort(port, host, mode);
}

/**********************************************************************************************************/
int AcceptProtocolConnect(DWORD_PTR lParam){
  VSClient *vsc=(VSClient *)lParam;
  return vsc->AcceptProtocolConnect();
};
/**********************************************************************************************************/
void RejectProtocolConnect(DWORD_PTR lParam){
  VSClient *vsc=(VSClient *)lParam;
  vsc->RejectProtocolConnect();
};
/**********************************************************************************************************/
void HangupProtocolConnect(DWORD_PTR lParam,int Strong){
  VSClient *vsc=(VSClient *)lParam;
  vsc->HangupProtocolConnect(Strong);
};
/**********************************************************************************************************/
int GetMyName(DWORD_PTR lParam,DWORD_PTR UserName,DWORD_PTR FirstName,DWORD_PTR LastName){
  VSClient *vsc=(VSClient *)lParam;
  return vsc->GetMyName((char*)UserName,(char*)FirstName,(char*)LastName);
};
/**********************************************************************************************************/
void GetOtherName(DWORD_PTR lParam,DWORD_PTR UserName,DWORD_PTR FirstName,DWORD_PTR LastName){
  VSClient *vsc=(VSClient *)lParam;
  vsc->GetOtherName((char*)UserName,(char*)FirstName,(char*)LastName);
};
/**********************************************************************************************************/
int LoginToServer(DWORD_PTR lParam, DWORD_PTR pLogin,DWORD_PTR pPassword,DWORD_PTR iAutoLogin, int Encrypt){
  VSClient *vsc=(VSClient *)lParam;
  return vsc->LoginToServer((char*)pLogin,(char*)pPassword,(int)iAutoLogin, Encrypt);
};
/**********************************************************************************************************/
void LogoutServer(DWORD_PTR lParam){
  VSClient *vsc=(VSClient *)lParam;
  vsc->LogoutServer();
};
/**********************************************************************************************************/
DWORD_PTR Initialize(DWORD_PTR dwFlag, HWND hwnd, char*szRegInit,char *szHomeDomain)
{
	DWORD *dwF = (DWORD *)dwFlag;
	VSClient *vsc = 0;
	HRESULT res = CoInitialize(NULL);
	if (res==S_OK || res==S_FALSE || res==RPC_E_CHANGED_MODE){
		*dwF = 0;
		if (VS_AcsLibInitial()){
			vsc = new VSClient(szRegInit, szHomeDomain);
			vsc->InitConfig(hwnd);
			_variant_t var;
			vsc->Process(GET_PARAM,"Receivers\\PlaybackDevice",&var);
			_bstr_t bt(var);
			if (bt.length() > 0)
				*dwF|=GET_CAP(CAP_AUDIORENDER_CONF);
		}
	}
	return (DWORD_PTR)vsc;
}
#ifdef VZOCHAT7
/**********************************************************************************************************/
DWORD_PTR Initialize(DWORD_PTR dwFlag, DWORD ThreadId, char*szRegInit,char *szHomeDomain)
{
	DWORD *dwF = (DWORD *)dwFlag;
	VSClient *vsc = 0;
	HRESULT res = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (res==S_OK || res==S_FALSE || res==RPC_E_CHANGED_MODE){
		*dwF = 0;
		if (VS_AcsLibInitial()){
			vsc = new VSClient(szRegInit, szHomeDomain);
			vsc->InitConfig(ThreadId);
			_variant_t var;
			vsc->Process(GET_PARAM,"Receivers\\PlaybackDevice",&var);
			_bstr_t bt(var);
			if (bt.length() > 0)
				*dwF|=GET_CAP(CAP_AUDIORENDER_CONF);
		}
	}
	return (DWORD_PTR)vsc;
}
#endif
/**********************************************************************************************************/
void SetFlags(DWORD_PTR lParam,DWORD_PTR lParam1){
    VSClient *vsc=(VSClient *)lParam;
    vsc->SetFlags((int*)lParam1);
};
/**********************************************************************************************************/
void GetFlags(DWORD_PTR lParam,DWORD_PTR lParam1){
    VSClient *vsc=(VSClient *)lParam;
    vsc->GetFlags((int*)lParam1);
};
/**********************************************************************************************************/
DWORD GetReceiverState(DWORD_PTR lParam,int ID,char *name){
    VSClient *vsc=(VSClient *)lParam;
    return vsc->GetReceiverState(ID,name);
};

/**********************************************************************************************************/
/*char **GetMultiList(DWORD_PTR lParam,DWORD_PTR lParam1){
  VSClient *vsc=(VSClient *)lParam;
  return vsc->GetMultiList((int **)lParam1);
};*/
/**********************************************************************************************************/
/*int  StartBroadcast(DWORD_PTR lParam,HWND hReportHwnd){
  VSClient *vsc=(VSClient *)lParam;
  return vsc->StartBroadcast(hReportHwnd);
};*/

/**********************************************************************************************************/
void SetDisableDirect(DWORD_PTR lParam,DWORD_PTR lParam1){
  VSClient *vsc=(VSClient *)lParam;
  vsc->SetDisableDirect((DWORD)lParam1);
};
/**********************************************************************************************************/
void ForceBicubic(DWORD_PTR lParam,DWORD_PTR force){
    VSClient *vsc=(VSClient *)lParam;
    vsc->ForceBicubic(force);
};
/**********************************************************************************************************/
void GetHistory(DWORD_PTR lParam,DWORD_PTR lParam1){
    VSClient *vsc=(VSClient *)lParam;
    char *pMem=( char *)lParam1;
    vsc->GetHistory(pMem);
};
/**********************************************************************************************************/
void SetHistory(DWORD_PTR lParam,DWORD_PTR lParam1){
    VSClient *vsc=(VSClient *)lParam;
    char *pMem=( char *)lParam1;
    vsc->SetHistory(pMem);
};
/**********************************************************************************************************/
void Release(DWORD_PTR lParam){
    VSClient *vsc=(VSClient *)lParam;
    delete vsc;
    CoUninitialize();
};
/**********************************************************************************************************/
/**********************************************************************************************************/
void SetSaturation(DWORD_PTR lParam,DWORD_PTR iSaturation){
  VSClient *vsc=(VSClient *)lParam;
  vsc->SetSaturation((int)iSaturation);
};
/**********************************************************************************************************/
/**********************************************************************************************************/
DWORD QueryUserInfo(DWORD_PTR lParam,char*szUID){
  VSClient *vsc=(VSClient *)lParam;
  return vsc->QueryUserInfo(szUID);
};
/**********************************************************************************************************/
DWORD ReadUserInfo(DWORD_PTR lParam,char*szUID,char**szFileds){
  VSClient *vsc=(VSClient *)lParam;
  return vsc->ReadUserInfo(szUID,szFileds);
};
/**********************************************************************************************************/
int GetServers(DWORD_PTR lParam,DWORD_PTR size,DWORD_PTR pName,DWORD_PTR pDesk){
  VSClient *vsc=(VSClient *)lParam;
  return vsc->GetServers((int)size,(unsigned char*)pName,(unsigned char*)pDesk);
};
/**********************************************************************************************************/
int GetDefaultServer(DWORD_PTR lParam, DWORD_PTR name){
  VSClient *vsc=(VSClient *)lParam;
  return vsc->GetDefaultServer((char*)name);
};
/**********************************************************************************************************/
void SetCurrentServer(DWORD_PTR lParam,DWORD_PTR pName){
  VSClient *vsc=(VSClient *)lParam;
  vsc->SetCurrentServer((unsigned char*)pName);
};
/**********************************************************************************************************/
void GetLoggedUsersList(DWORD_PTR lParam,DWORD_PTR UserList,int Type){
  VSClient *vsc=(VSClient *)lParam;
  vsc->GetLoggedUsersList((void**)UserList,Type);
};

/**********************************************************************************************************/
void SetServer(DWORD_PTR lParam,char *szName,char *szIP,char*szPort){
  VSClient *vsc=(VSClient *)lParam;
  vsc->SetServer(szName,szIP,szPort);
};
/**********************************************************************************************************/
void SetAutologin(DWORD_PTR lParam,DWORD ALogin){
  VSClient *vsc=(VSClient *)lParam;
  vsc->SetAutoLogin(ALogin);
};
/**********************************************************************************************************/
DWORD GetAutologin(DWORD_PTR lParam){
  VSClient *vsc=(VSClient *)lParam;
  return vsc->GetAutoLogin();
};
/**********************************************************************************************************/
void SetTrackCallBack(DWORD_PTR lParam,void *pCallBack,void*pParam){
  VSClient *vsc=(VSClient *)lParam;
  vsc->SetTrackCallBack(pCallBack,pParam);
};
/**********************************************************************************************************/
void SetExtTrackCallBack(DWORD_PTR lParam,void *pCallBack,void*pParam){
  VSClient *vsc=(VSClient *)lParam;
  vsc->SetExtTrackCallBack(pCallBack,pParam);
};
/**********************************************************************************************************/
void CommEnabled(DWORD_PTR lParam,int iEnabled){
};
/**********************************************************************************************************/
void CommOperation(DWORD_PTR lParam,char*szPort,int *pSpeed,int *pMode,int read){
};
/**********************************************************************************************************/
int CreateLoopBackChannel(DWORD_PTR lParam,int bAccept,int iPort){
  VSClient *vsc=(VSClient *)lParam;
  return vsc->CreateLoopBackChannel((bAccept==0)? false:true,iPort);
};
/**********************************************************************************************************/
void CloseLoopBackChannel(DWORD_PTR lParam){
  VSClient *vsc=(VSClient *)lParam;
  vsc->CloseLoopBackChannel();
};
/**********************************************************************************************************/