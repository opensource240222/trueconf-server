#ifndef _CLIENT_INTERFACE_
#define _CLIENT_INTERFACE_

//#include "../AddressBookCache/VZOchat7.h"

#include "../VSClient/VSClientBase.h"
#include <windows.h>
#define GET_CAP(n)(1<<n)
#define CAP_DIRECTSHOW          0
#define CAP_AUDIORENDER         4
#define CAP_AUDIORENDER_CONF    5
#define CAP_VIDEOCOMPRESSOR     8
#define CAP_VIDEODECOMPRESSOR   9
#define REGISTERED_FLAG         11

typedef struct DEVICEDESK{
    char name[MAX_PATH];
  } tDeviceDesk;
typedef LRESULT (*RENDERPROC)( HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam,LPVOID lParam1);
int InterfaceGetVersion(DWORD_PTR pGUIVersion,DWORD_PTR pAPP_version);
DWORD_PTR Initialize(DWORD_PTR dwFlag,HWND hwnd,char*szRegInit,char *szHomeDomain);
#ifdef VZOCHAT7
DWORD_PTR Initialize(DWORD_PTR dwFlag, DWORD ThreadId, char*szRegInit,char *szHomeDomain);
#endif

void Release(DWORD_PTR lParam);

DWORD Process(DWORD_PTR lParam,int Operation,char*szInterface,VARIANT *pVar);

DWORD QueryUserInfo(DWORD_PTR lParam,char*szUID);
DWORD ReadUserInfo(DWORD_PTR lParam,char*szUID,char**szFileds);

int GetChatMessage(DWORD_PTR lParam,int Id, char* From, char* Mess, char* to, char* Dn);
int GetChatMessageV2(DWORD_PTR lParam,int Id, char* From, char* Mess, char* to, char* Dn, long long *time);
void GetCommandMessage(DWORD_PTR lParam,int Id, char* From, char* Command);
void BanUser(DWORD_PTR lParam,char*UserID,int Strong);
int StartSender(DWORD_PTR lParam/*,DWORD_PTR pAudio,DWORD_PTR pVideo,HWND hwnd,RENDERPROC *pwndpSelfView,LPVOID *lpSelfView*/,int Width,int Height,int Freq);
void StopSender(DWORD_PTR lParam);
DWORD GetPropertyPage(DWORD_PTR lParam,HWND hwnd);
void ConnectSender(DWORD_PTR lParam,HWND hReportHwnd,DWORD_PTR Type);
void DisconnectSender(DWORD_PTR lParam);
int SendConnectionSender(DWORD_PTR lParam,DWORD_PTR lParam1);
int SendConnectionReceiver(DWORD_PTR lParam,DWORD_PTR lParam1);
void SetHistory(DWORD_PTR lParam,DWORD_PTR lParam1);
void GetHistory(DWORD_PTR lParam,DWORD_PTR lParam1);
void GetTraffic(DWORD_PTR lParam,DWORD_PTR lParam1,DWORD_PTR lParam2);

int LoginToServer(DWORD_PTR lParam, DWORD_PTR pLogin,DWORD_PTR pPassword,DWORD_PTR iAutoLogin, int Encrypt);
void LogoutServer(DWORD_PTR lParam);
int  GetMyName(DWORD_PTR lParam,DWORD_PTR UserName,DWORD_PTR FirstName,DWORD_PTR LastName);
void GetOtherName(DWORD_PTR lParam,DWORD_PTR UserName,DWORD_PTR FirstName,DWORD_PTR LastName);

int AcceptProtocolConnect(DWORD_PTR lParam);
void RejectProtocolConnect(DWORD_PTR lParam);
void HangupProtocolConnect(DWORD_PTR lParam,int Strong);

DWORD BwtStart(DWORD_PTR lParam, DWORD_PTR hwnd);
DWORD BwtStop(DWORD_PTR lParam);
DWORD BwtGet(DWORD_PTR lParam, DWORD_PTR Status, DWORD_PTR Id);
void BwtWizardOn(DWORD_PTR lParam, int mode);

DWORD SetProxySet(DWORD_PTR array);
void GetProxySet(DWORD_PTR array);
void SetProxyDialog(DWORD_PTR func);
void ProxyDialogEnd(int ret);
void SetProxyNetMode(int* cfg);
int  SetNetType(int mode);
int  SetManualPort(unsigned short *port, char* host, int mode);
void ForceBicubic(DWORD_PTR lParam,DWORD_PTR force);
void SetDisableDirect(DWORD_PTR lParam,DWORD_PTR lParam1);
void GetFlags(DWORD_PTR lParam,DWORD_PTR lParam1);
void SetFlags(DWORD_PTR lParam,DWORD_PTR lParam1);
void GetSkinName(DWORD_PTR lParam,DWORD_PTR FileName);
void SetSkinName(DWORD_PTR lParam,DWORD_PTR FileName);
void SendMessages(DWORD_PTR lParam,DWORD_PTR pMessage,void* szOther);
void SendCommands(DWORD_PTR lParam, char* command, char* to);

int GetServers(DWORD_PTR lParam,DWORD_PTR size,DWORD_PTR pName,DWORD_PTR pDesk);
int GetDefaultServer(DWORD_PTR lParam, DWORD_PTR name);
void SetCurrentServer(DWORD_PTR lParam,DWORD_PTR pName);

void SetSaturation(DWORD_PTR lParam,DWORD_PTR iSaturation);

DWORD GetReceiverState(DWORD_PTR lParam,int ID,char *name);

void GetLoggedUsersList(DWORD_PTR lParam,DWORD_PTR UserList,int Type);

DWORD GetProperty(DWORD_PTR lParam,DWORD_PTR lParam1,DWORD_PTR lParam2);
void SetServer(DWORD_PTR lParam,char *szName,char *szIP,char*szPort);
DWORD AddressBook(DWORD_PTR lParam,DWORD abCommand,char*param,long addressBook, long hash);

void SetAutologin(DWORD_PTR lParam,DWORD ALogin);
DWORD GetAutologin(DWORD_PTR lParam);
void SetJitter(DWORD_PTR lParam,int Mode);
void CommOperation(DWORD_PTR lParam,char*szPort,int *pSpeed,int *pMode,int read);
void CommEnabled(DWORD_PTR lParam,int iEnabled);
int CreateLoopBackChannel(DWORD_PTR lParam,int bAccept,int iPort);
void CloseLoopBackChannel(DWORD_PTR lParam);
void SetTrackCallBack(DWORD_PTR lParam,void *pCallBack,void*pParam);
void SetExtTrackCallBack(DWORD_PTR lParam,void *pCallBack,void*pParam);

#endif