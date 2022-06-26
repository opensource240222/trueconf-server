/****************************************************************************
* (c) 2002 Visicron Inc.  http://www.visicron.net/
*
* Project:
*
* $Revision: 8 $
* $History: VSClient.h $
 *
 * *****************  Version 8  *****************
 * User: Dront78      Date: 8.07.11    Time: 21:49
 * Updated in $/VSNA/^clientinterface
 * - fixed terminald mic controls
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 6.11.09    Time: 12:50
 * Updated in $/VSNA/^ClientInterface
 * - static SetServer
 * - pareicipants number in conference info
 *
 * *****************  Version 6  *****************
 * User: Dront78      Date: 14.06.09   Time: 10:49
 * Updated in $/VSNA/^ClientInterface
 * - VZOchat7 merge. see VZOchat7.h for details
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 1.04.09    Time: 19:23
 * Updated in $/VSNA/^ClientInterface
 * - bugfix #5815
 *
 * *****************  Version 4  *****************
 * User: Dront78      Date: 24.03.09   Time: 16:53
 * Updated in $/VSNA/^ClientInterface
 * - VZOCHAT7 VSClient support added
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 19.03.08   Time: 17:50
 * Updated in $/VSNA/^ClientInterface
 * - Login interface extended
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 6.03.08    Time: 16:05
 * Updated in $/VSNA/^ClientInterface
 * - chat rewrited
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:48
 * Created in $/VSNA/^ClientInterface
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 16.05.07   Time: 17:37
 * Updated in $/VS2005/^ClientInterface
 * - Registry flags revised, set PresiceScaling flag to 1 always
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:50
 * Created in $/VS2005/^ClientInterface
 *
 * *****************  Version 122  *****************
 * User: Smirnov      Date: 2.08.06    Time: 13:35
 * Updated in $/VS/^ClientInterface
 * - address book and statuses processing removed
 * - added additional interface to receive container by GUI
 * - added hash param in SearchAB
 *
 * *****************  Version 121  *****************
 * User: Smirnov      Date: 13.07.06   Time: 18:56
 * Updated in $/VS/^ClientInterface
 * - new audio controls (via mixer)
 *
 * *****************  Version 120  *****************
 * User: Smirnov      Date: 21.06.06   Time: 11:06
 * Updated in $/VS/^ClientInterface
 * - autodiscovery incalcated
 *
 * *****************  Version 119  *****************
 * User: Smirnov      Date: 25.04.06   Time: 19:38
 * Updated in $/VS/^ClientInterface
 * - removed old code, Thread class changed
 *
 * *****************  Version 118  *****************
 * User: Smirnov      Date: 20.04.06   Time: 13:46
 * Updated in $/VS/^ClientInterface
 * - new audio hardware test
 *
 * *****************  Version 117  *****************
 * User: Melechko     Date: 16.06.05   Time: 16:13
 * Updated in $/VS/^ClientInterface
 * Add check User in Multiconference
 *
 * *****************  Version 116  *****************
 * User: Melechko     Date: 4/12/05    Time: 13:10
 * Updated in $/VS/^ClientInterface
 *
 * *****************  Version 115  *****************
 * User: Smirnov      Date: 22.02.05   Time: 18:22
 * Updated in $/VS/^ClientInterface
 * low latency switch
 * bitrate degradation more accurate
 * Neigl for streams only for local conf
 * fix with Localalloc for otherId
 *
 * *****************  Version 114  *****************
 * User: Melechko     Date: 24.01.05   Time: 15:18
 * Updated in $/VS/^ClientInterface
 * FixSaturation
 *
 * *****************  Version 113  *****************
 * User: Melechko     Date: 19.01.05   Time: 17:02
 * Updated in $/VS/^ClientInterface
 * some changes :)
 *
****************************************************************************/

#ifndef _VSCLIENT_
#define _VSCLIENT_

//#include "../AddressBookCache/VZOchat7.h"

#include <windows.h>
#include "ClientInterface.h"
#include "../VSClient/VSClientBase.h"

/**********************************************************************************************************/
class CCaptureAudioSource;
class CRenderAudio;
class CCaptureVideoSourceDirectShow;
class VS_RegistryKey;
class CVideoRenderBase;
class CDirectXRender;
class CDIBRender;
class CVideoCompressor;
class CVideoDecompressor;
class CThinkClient;
class CThinkClientSender;
class CThinkClientReceiver;
class VS_StreamClientReceiver;
class VS_StreamClientSender;
class CEndpointRegistry;
class CVSTrClientProc;
class VSTestAudio;
class VS_PlugTrans;
class VS_PlugChannel;
class CReceiversPool;
class CMultiReceiver;
/**********************************************************************************************************/
#define AB_QUERY_BOOK     0
#define AB_ADD_USER       1
#define AB_REMOVE_USER    2
#define AB_SEARCH         3
#define AB_ADD_BY_EMAIL   4
/**********************************************************************************************************/
#define WM_NOTIFY_CALL WM_USER+17
/**********************************************************************************************************/

class VSClient: public CVSInterface
{
public:
#ifdef VZOCHAT7
	static HWND threadHwnd;
	static DWORD threadId;
	static LRESULT CALLBACK ThreadWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void InitConfig(DWORD ThreadId);
	CVSTrClientProc * GetProtocol();
    CThinkClient * GetThinkClient();
#endif

	/**/
	VSClient(char*szRegInit, char *service);
	~VSClient();
	/**/
	void InitConfig(HWND hwnd);
	int InitDevicesList();
	int InitSender(int Width,int Height,int Freq);
	void ReleaseSender();
	/**/
	int LoginToServer(char *pLogin,char *pPassword,int iAutoLogin, int Encrypt);
	void LogoutServer();
	/**/
	void BanUser(char*UserID,int Strong);
	int  GetMyName(char *UserName, char *FirstName, char *LastName);
	void GetOtherName(char *UserName, char *FirstName, char *LastName);
	int  GetServers(int size,unsigned char*pName,unsigned char*pDesk);
	int  GetDefaultServer(char *name);
	void GetHistory(char*pMem);
	void GetSkinName(char* FileName);
	int GetChatMessage(int Id, char* From, char* Mess, char* to, char* Dn);
	int GetChatMessageV2(int Id, char* From, char* Mess, char* to, char* Dn, long long *time);
	void GetCommandMessage(int Id, char* From, char* Command);
	void GetFlags(int* pFlags);

	int  GetSenderTraffic();
	DWORD GetAutoLogin();
	/**/
	void SendMessage(void *pMessage,void* szOther);
	void SendCommand(char * command, char * to);

	void SetFlags(int* pFlags);
	void SetHistory(char*pMem);
	void SetCurrentServer(unsigned char*pName);
	void SetSkinName(char* FileName);

	void ForceBicubic(DWORD force);
	void SetDisableDirect(DWORD iDd);
	void SetSaturation(int iSaturation);
	void SetAutoLogin(DWORD ALogin);
	void SetJitter(int Mode);
	/**/

	void SetTrackCallBack(void *pCallBack,void*pParam);
	void SetExtTrackCallBack(void *pCallBack,void*pParam);
	void SendConfStat();

	DWORD QueryUserInfo(char*szUID);
	DWORD ReadUserInfo(char*szUID,char**szFileds);

	DWORD GetReceiverState(int ID,char *name);
	void GetLoggedUsersList(void** UserList,int Type);
	DWORD GetProperty(const char* Name, char* Property);

	int  StartVideoCapture(HWND hwnd,char *pVideoCapture);
	void ConnectSender(HWND hReportHwnd,int Type);
	void DisconnectSender();
	void SetServer(char *szName,char *szIP,char*szPort);

	int  AcceptProtocolConnect();
	void RejectProtocolConnect();
	void HangupProtocolConnect(int Strong);

	DWORD AddressBook(DWORD abCommand,char*param,long addressBook, long hash);

	DWORD BwtStart(HWND hwnd);
	DWORD BwtStop();
	DWORD BwtGet(void * out, int id);
	void  BwtWizardOn(int mode);

	int CreateLoopBackChannel(bool bAccept,int iPort);
	void CloseLoopBackChannel();

	void	UpdateExtStatus(const char *name, const int value); //ext_status, device_type, camera, mic
	void	UpdateExtStatus(const char *name, const wchar_t *value); //status_descripion
	void	UpdateExtStatus(const int fwd_type, const wchar_t *fwd_call_id, const int fwd_timeout, const wchar_t *timeout_call_id); /// CALLFWD_ST_NAME
	/**************************************************************/
private:
	int                          (*m_CallBack)(int);

	bool                          m_bDisableDirect;
	int                           m_iFontSize;
	int                           m_iSelfX;
	int                           m_iSelfY;
	int                           m_iMyColor;
#ifdef _VIRTUAL_OUT
	static bool CheckStatus(void *pParam);
#endif
	VS_RegistryKey                *m_kKeyName;
	CThinkClientSender            *m_pThinkClientSender;
	std::shared_ptr<CVSTrClientProc> m_pProtocol;
	CReceiversPool                *m_pReceivers;
	bool                          GetRegFlag(const char* FlagName,bool bDefault);
	int ProcessFunction(VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar) override;
};
/**********************************************************************************************************/
#endif
