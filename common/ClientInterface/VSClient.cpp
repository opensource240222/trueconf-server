/****************************************************************************
* (c) 2002 Visicron Inc.  http://www.visicron.net/
*
* Project:
*
* $Revision: 42 $
* $History: VSClient.cpp $
 *
 * *****************  Version 42  *****************
 * User: Sanufriev    Date: 2.08.12    Time: 10:25
 * Updated in $/VSNA/ClientInterface
 * - were added Opus audio codec
 *
 * *****************  Version 41  *****************
 * User: Sanufriev    Date: 2.07.12    Time: 18:11
 * Updated in $/VSNA/ClientInterface
 * - add iSAC codec
 *
 * *****************  Version 40  *****************
 * User: Smirnov      Date: 18.04.12   Time: 18:45
 * Updated in $/VSNA/ClientInterface
 * - bugfix#11012
 *
 * *****************  Version 39  *****************
 * User: Dront78      Date: 8.07.11    Time: 21:49
 * Updated in $/VSNA/^ClientInterface
 * - fixed terminald mic controls
 *
 * *****************  Version 38  *****************
 * User: Smirnov      Date: 14.03.11   Time: 15:40
 * Updated in $/VSNA/^ClientInterface
 * - set window in sender
 *
 * *****************  Version 37  *****************
 * User: Sanufriev    Date: 9.03.11    Time: 13:31
 * Updated in $/VSNA/^ClientInterface
 * - were added vpc stereo codec impl
 *
 * *****************  Version 36  *****************
 * User: Ktrushnikov  Date: 24.11.10   Time: 14:44
 * Updated in $/VSNA/^ClientInterface
 * - convert to UTF8 endpoint property "Hardware Config"
 * - memory leak deleted
 *
 * *****************  Version 35  *****************
 * User: Sanufriev    Date: 22.11.10   Time: 14:40
 * Updated in $/VSNA/^ClientInterface
 * - were added VS_VCODEC_VPXHD
 *
 * *****************  Version 34  *****************
 * User: Smirnov      Date: 8.11.10    Time: 20:32
 * Updated in $/VSNA/^ClientInterface
 * - hQ auto flag in registry
 *
 * *****************  Version 33  *****************
 * User: Smirnov      Date: 18.10.10   Time: 19:47
 * Updated in $/VSNA/^ClientInterface
 * - HQ detect alfa
 *
 * *****************  Version 32  *****************
 * User: Smirnov      Date: 23.09.10   Time: 21:02
 * Updated in $/VSNA/^ClientInterface
 * - added in windows firewall in case of first start
 *
 * *****************  Version 31  *****************
 * User: Smirnov      Date: 25.08.10   Time: 19:26
 * Updated in $/VSNA/^ClientInterface
 * - some optimization in interfaces
 *
 * *****************  Version 30  *****************
 * User: Smirnov      Date: 23.08.10   Time: 22:19
 * Updated in $/VSNA/^ClientInterface
 * - long names in devices
 * - corrected Wide names for devices
 * - init devices section rewrited
 *
 * *****************  Version 29  *****************
 * User: Smirnov      Date: 30.06.10   Time: 16:50
 * Updated in $/VSNA/^ClientInterface
 * delete conference on owner hungup (#7493)
 *
 * *****************  Version 28  *****************
 * User: Sanufriev    Date: 29.06.10   Time: 18:15
 * Updated in $/VSNA/^ClientInterface
 * - ench 7471 (prepare avi writer)
 *
 * *****************  Version 27  *****************
 * User: Sanufriev    Date: 28.06.10   Time: 17:03
 * Updated in $/VSNA/^ClientInterface
 * - were add VPX in enumaration of media format
 * - update vpx libs
 * - were change h.264 & vpx settings
 *
 * *****************  Version 26  *****************
 * User: Sanufriev    Date: 4.06.10    Time: 17:35
 * Updated in $/VSNA/^ClientInterface
 * - Direct3D Render implementation
 *
 * *****************  Version 25  *****************
 * User: Sanufriev    Date: 24.03.10   Time: 19:21
 * Updated in $/VSNA/^ClientInterface
 * - were added calc statictics (bug 7127)
 * - were merged all calc statistics in sender and receiver
 * - were added jitter calculation in Nhp receiver
 *
 * *****************  Version 24  *****************
 * User: Sanufriev    Date: 10.12.09   Time: 19:04
 * Updated in $/VSNA/^ClientInterface
 * - unicode capability for hardware lists
 *
 * *****************  Version 23  *****************
 * User: Sanufriev    Date: 15.06.09   Time: 18:49
 * Updated in $/VSNA/^ClientInterface
 * - static link h.264
 * - remove x264 define
 *
 * *****************  Version 22  *****************
 * User: Smirnov      Date: 3.06.09    Time: 11:52
 * Updated in $/VSNA/^ClientInterface
 * - audiorender cleanup
 *
 * *****************  Version 21  *****************
 * User: Smirnov      Date: 2.06.09    Time: 12:38
 * Updated in $/VSNA/^ClientInterface
 * - set default audio codec to speex, q=4, freq=16 kHz
 *
 * *****************  Version 20  *****************
 * User: Sanufriev    Date: 8.05.09    Time: 12:08
 * Updated in $/VSNA/^ClientInterface
 * - were modifed new version AviWriter
 *
 * *****************  Version 19  *****************
 * User: Smirnov      Date: 1.04.09    Time: 19:23
 * Updated in $/VSNA/^ClientInterface
 * - bugfix #5815
 *
 * *****************  Version 18  *****************
 * User: Dront78      Date: 24.03.09   Time: 16:53
 * Updated in $/VSNA/^ClientInterface
 * - VZOCHAT7 VSClient support added
 *
 * *****************  Version 17  *****************
 * User: Smirnov      Date: 21.11.08   Time: 12:58
 * Updated in $/VSNA/^ClientInterface
 * - ban removed
 *
 * *****************  Version 16  *****************
 * User: Smirnov      Date: 23.05.08   Time: 17:24
 * Updated in $/VSNA/^ClientInterface
 * - bugfix #4355
 *
 * *****************  Version 15  *****************
 * User: Smirnov      Date: 16.04.08   Time: 16:10
 * Updated in $/VSNA/^ClientInterface
 * - connect/disconnect events, methods
 * - connect to servers without transport reinstall
 * - auto server connects timeouts is more accurate
 *
 * *****************  Version 14  *****************
 * User: Smirnov      Date: 19.03.08   Time: 17:50
 * Updated in $/VSNA/^ClientInterface
 * - Login interface extended
 *
 * *****************  Version 13  *****************
 * User: Smirnov      Date: 6.03.08    Time: 16:05
 * Updated in $/VSNA/^ClientInterface
 * - chat rewrited
 *
 * *****************  Version 12  *****************
 * User: Smirnov      Date: 26.02.08   Time: 16:54
 * Updated in $/VSNA/^ClientInterface
 * - new servers coonect shceme
 *
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 14.02.08   Time: 18:54
 * Updated in $/VSNA/^ClientInterface
 * - ep properties checked
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 14.02.08   Time: 13:30
 * Updated in $/VSNA/^ClientInterface
 * - endpoInt properties
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 18.01.08   Time: 17:37
 * Updated in $/VSNA/^ClientInterface
 * - Discovery functions verified
 *
 * *****************  Version 8  *****************
 * User: Dront78      Date: 18.01.08   Time: 17:01
 * Updated in $/VSNA/^ClientInterface
 * - bigfixes from old architecture.
 * - ambiguous code marked as "old code"
 *
 * *****************  Version 7  *****************
 * User: Dront78      Date: 13.12.07   Time: 13:15
 * Updated in $/VSNA/^ClientInterface
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 10.12.07   Time: 19:46
 * Updated in $/VSNA/^ClientInterface
 * - broker format removed
 * - default server point to Stass machine
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 30.11.07   Time: 13:11
 * Updated in $/VSNA/^ClientInterface
 * - link ok
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 29.11.07   Time: 18:42
 * Updated in $/VSNA/^ClientInterface
 * - registry moved to NA in client
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 27.11.07   Time: 20:27
 * Updated in $/VSNA/^ClientInterface
 * - auth procedure
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 22.11.07   Time: 16:41
 * Updated in $/VSNA/^ClientInterface
 * - repare client
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:48
 * Created in $/VSNA/^ClientInterface
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 4.07.07    Time: 19:25
 * Updated in $/VS2005/^ClientInterface
 * - device statuses added
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 16.05.07   Time: 17:37
 * Updated in $/VS2005/^ClientInterface
 * - Registry flags revised, set PresiceScaling flag to 1 always
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 7.03.07    Time: 16:43
 * Updated in $/VS2005/^ClientInterface
 * - Intercom all features supported
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 7.02.07    Time: 12:23
 * Updated in $/VS2005/^ClientInterface
 * - default server and autodicovery
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:50
 * Created in $/VS2005/^ClientInterface
 *
 ****************************************************************************/

#include "VSClient.h"
#include "../VSClient/VSAudio.h"
#include "VSClient/VSAudioDs.h"
#include "../VSClient/VSCapture.h"
#include "../VSClient/VSRender.h"
#include "../VSClient/VSCompress.h"
#include "../VSClient/VSThinkClient.h"
#include "../VSClient/VSTrClientProc.h"
#include "../VSClient/VS_ApplicationInfo.h"
#include "../net/EndpointRegistry.h"
#include "../std/cpplib/VS_RegistryKey.h"
#include "../std/cpplib/VS_Endpoint.h"
#include "../acs/Lib/VS_AcsLib.h"
#include "../streams/Client/VS_StreamClientReceiver.h"
#include "../streams/Client/VS_StreamClientSender.h"
#include "../std/cpplib/VS_ClientCaps.h"
#include "VSClient/VS_Dmodule.h"
#include "../std/cpplib/VS_WideStr.h"
#include "../std/cpplib/VS_PerformanceMonitor.h"
/**********************************************************************************************************/
const static char m_FSize[]		="FontSize";
const static char m_SXFlag[]	="SelfX";
const static char m_SYFlag[]	="SelfY";
const static char m_MyColor[]	="MyColor";
const static char m_DDFlag[]	="DisableDirect";
const static char m_History[]	="Connect_";
const static char m_Skin[]		="SkinName";
const static char m_Saturation[]="Saturation";

#ifdef VZOCHAT7
DWORD VSClient::threadId = 0;
HWND VSClient::threadHwnd = 0;
#endif

/**********************************************************************************************************/
VSClient::VSClient(char*szRegInit, char *service):
CVSInterface("Client",NULL,szRegInit,true)
{
	VS_CaptureDevice::Open();
	g_pDtrase = new VS_DebugOut;
	m_pProtocol = std::make_shared<CVSTrClientProc>(this);
	m_pProtocol->SetDiscoveryServise(service);
	m_pProtocol->TrInit(VS_FileTransfer::DEFAULT_FLAGS);
	m_pProtocol->TrSetLimits(0, 0);
	m_pReceivers = new CReceiversPool(this, m_pProtocol.get());
	m_pThinkClientSender=new CThinkClientSender(this, m_pProtocol.get());
	g_AviWriterGroup = new VS_AviWriteGroup(this);
	TIMECAPS caps = {0, 0};
	timeGetDevCaps(&caps, sizeof (caps));
	if (caps.wPeriodMin < 5)
		timeBeginPeriod(5);
	VS_PerformanceMonitor::Instance().Start();
}

/**********************************************************************************************************/
#ifdef _VIRTUAL_OUT
bool VSClient::CheckStatus(void *pParam){
	CVSTrClientProc *pProtocol=(CVSTrClientProc *)pParam;
	return (pProtocol->m_Status.dwStatus&STATUS_LOGGEDIN)?true:false;
};
#endif

/**********************************************************************************************************/
void VSClient::SetTrackCallBack(void *pCallBack,void*pParam){
	TrackCallBack.SetCallBack(pCallBack,pParam);
}

void VSClient::SetExtTrackCallBack(void *pCallBack,void*pParam){
	TrackCallBack.SetExternalCallBack(pCallBack,pParam);
}

/**********************************************************************************************************/
int VSClient::ProcessFunction(VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar)
{
	_variant_t* var=(_variant_t*)pVar;
	if (stricmp(pSection, "VideoQuality")==0)
	{
		int ret = 1;
		int snd = GetSenderTraffic()/128;
		if (m_pProtocol->m_Status.CurrConfInfo->confType==0) {
			int rcv = m_pReceivers->GetQualityBitrate()/128;
			int retb = 1;
			if (rcv > 64) retb = 2;
			if (rcv >128) retb = 3;
			if (rcv >200) retb = 4;
			if (rcv >500) retb = 5;
			if (rcv >1000) retb = 6;
			int retq = m_pReceivers->GetQuality();

			if (retq > 0) {// voice is on
				if (retb > 1) // bitrate > 64
					ret = int(sqrt((float)retq*retb) + 0.5);
				else {
					ret = retq - 1;
					if (ret < 1)
						ret = 1;
				}
			}
			else {
				ret = retb;
			}
		}
		else if (m_pProtocol->m_Status.CurrConfInfo->confType==4 || m_pProtocol->m_Status.CurrConfInfo->confType==5) {
			ret = m_pReceivers->GetQuality();
		}
		*var = ret;
		return VS_INTERFACE_OK;
	}
	return VS_INTERFACE_NO_FUNCTION;
}

/**********************************************************************************************************/
DWORD VSClient::GetProperty(const char* Name, char* Property) {
	if(m_pProtocol)
		return m_pProtocol->GetProperties(Name,Property);
	return -1;
}

/**********************************************************************************************************/
DWORD VSClient::AddressBook(DWORD abCommand, char*param, long addressBook, long hash)
{
	switch(abCommand)
	{
	case AB_QUERY_BOOK:
		return m_pProtocol->SearchAddressBook("",addressBook, hash);
	case AB_ADD_USER:
		{
			if (addressBook==AB_BAN_LIST)
				return m_pProtocol->BanUser(param, hash);
			else
				return m_pProtocol->AddUserToAddressBook(param, addressBook);
		}
	case AB_ADD_BY_EMAIL:
		return m_pProtocol->AddUserToAddressBook(NULL,addressBook,param);
	case AB_REMOVE_USER:
		return m_pProtocol->RemoveFromAddressBook(param,addressBook);
	case AB_SEARCH:
		return m_pProtocol->SearchAddressBook(param,addressBook, hash);
	default:
		return 1;
	}
}

/**********************************************************************************************************/
int VSClient::GetChatMessage( int Id, char* From, char* Mess, char* to, char* Dn){
	return m_pProtocol->GetMessage(Id,  From,  Mess, to, Dn);
}
/**********************************************************************************************************/
int VSClient::GetChatMessageV2( int Id, char* From, char* Mess, char* to, char* Dn, long long *time )
{
	return m_pProtocol->GetMessage(Id,  From,  Mess, to, Dn, time);
}
/**********************************************************************************************************/
void VSClient::GetCommandMessage(int Id, char* From, char* Command){
	m_pProtocol->GetCommand(Id,  From,  Command);
};
/**********************************************************************************************************/
VSClient::~VSClient()
{
	VS_PerformanceMonitor::Instance().Stop();
	TIMECAPS caps = {0, 0};
	timeGetDevCaps(&caps, sizeof (caps));
	if (caps.wPeriodMin < 5)
		timeEndPeriod(5);

	delete m_pReceivers;

#ifdef _VIRTUAL_OUT
	m_pVideoRenderSelfView->SetVirtualOut(false, NULL, NULL);
#endif
	delete m_pThinkClientSender;
	// now we can close AudioDeviceManager
	VS_AudioDeviceManager::Close();
	// now we can close CVideoRenderBase
	CVideoRenderBase::Close();
	// MF close
	VS_CaptureDevice::Close();
	m_pProtocol.reset();
	delete g_pDtrase; g_pDtrase = 0;
	delete g_AviWriterGroup; g_AviWriterGroup = 0;
};
/**********************************************************************************************************/
void VSClient::GetSkinName(char* FileName){
	if( m_kKeyName->GetValue(FileName,MAX_PATH,VS_REG_STRING_VT,m_Skin)==0)
		FileName[0]=0;
};
/**********************************************************************************************************/
void VSClient::SetSkinName(char* FileName){
	m_kKeyName->SetString(FileName, m_Skin);
};
/**********************************************************************************************************/
void VSClient::SendMessage(void *pMessage,void* szOther){
	m_pProtocol->ChatSend((char*)pMessage,(char*)szOther);
};
/**********************************************************************************************************/
void VSClient::SendCommand(char * command, char * to){
	m_pProtocol->SendCommand(command, to);
};
/**********************************************************************************************************/
void VSClient::SendConfStat() {
	TConferenceStatistics snd, rcv;
	m_pThinkClientSender->GetConfStat(&snd);
	m_pReceivers->GetConfStat(&rcv);
	m_pProtocol->SendConfStat(&snd, &rcv);
};
/**********************************************************************************************************/
int VSClient::GetServers(int size, unsigned char*pName, unsigned char*pDesk)
{
	return m_pProtocol->GetServers(size, pName);
}

/**********************************************************************************************************/
int VSClient::GetDefaultServer(char *name){
	if (!name)
		return -1;
	VS_ReadAS(name, true);
	return 0;
}

/**********************************************************************************************************/
void VSClient::SetCurrentServer(unsigned char*pName){
	m_pProtocol->ConnectServer((char*)pName);
};
/**********************************************************************************************************/
bool VSClient::GetRegFlag(const char* FlagName,bool bDefault){
	int iAnswer;
	if( m_kKeyName->GetValue(&iAnswer,sizeof(int),VS_REG_INTEGER_VT,FlagName)!=0)
		return iAnswer?TRUE:FALSE;
	else
		return bDefault;
}

/**********************************************************************************************************/
void VSClient::InitConfig(HWND hwnd)
{
	VS_AudioDeviceManager::Open(hwnd);
	CVideoRenderBase::Open();
	VS_AudioMixerVolume::SetWnd(hwnd);
	CVSTrClientProc::SetFirewall();

	InitDevicesList();
	Process(RUN_COMMAND, "Receivers\\PlaybackDevice", NULL);

	m_kKeyName = new VS_RegistryKey(TRUE, "Client", FALSE, TRUE);

	m_iSelfX=-1;
	m_kKeyName->GetValue(&m_iSelfX,sizeof(int),VS_REG_INTEGER_VT,m_SXFlag);

	m_iSelfY=-1;
	m_kKeyName->GetValue(&m_iSelfY,sizeof(int),VS_REG_INTEGER_VT,m_SYFlag);

	m_iMyColor=0x008000;
	m_kKeyName->GetValue(&m_iMyColor,sizeof(int),VS_REG_INTEGER_VT,m_MyColor);

	m_iFontSize=4;
	m_kKeyName->GetValue(&m_iFontSize,sizeof(int),VS_REG_INTEGER_VT,m_FSize);

	m_bDisableDirect =GetRegFlag(m_DDFlag,FALSE);
	SetDisableDirect((DWORD)m_bDisableDirect);

	int iSaturation = 120;
	m_kKeyName->GetValue(&iSaturation,sizeof(int),VS_REG_INTEGER_VT,m_Saturation);
	SetSaturation(iSaturation);

	m_pProtocol->Init(hwnd);
	m_pThinkClientSender->SetNotifyWnd(hwnd);
}

#ifdef VZOCHAT7
LRESULT CALLBACK VSClient::ThreadWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (hwnd == threadHwnd) {
		PostThreadMessage(threadId, uMsg, wParam, lParam);
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void VSClient::InitConfig(DWORD ThreadId)
{
	WNDCLASS wc = {};
	wc.hInstance = GetModuleHandle(0);
	wc.lpszClassName = "VZO7WND";
	wc.lpfnWndProc = &VSClient::ThreadWindowProc;
	RegisterClass(&wc);

	VSClient::threadId = ThreadId;
	VSClient::threadHwnd = CreateWindowEx(0, "VZO7WND", "VZOchat7", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, GetModuleHandle(0), 0);
	InitConfig(VSClient::threadHwnd);
};

CVSTrClientProc * VSClient::GetProtocol()
{
	return m_pProtocol;
}

CThinkClient * VSClient::GetThinkClient()
{
    return m_pThinkClientSender;
}

#endif

/**********************************************************************************************************/
DWORD VSClient::QueryUserInfo(char*szUID){
	return m_pProtocol->GetUserInformation(szUID,NULL);
};
/**********************************************************************************************************/
DWORD VSClient::ReadUserInfo(char*szUID,char**szFileds){
	return m_pProtocol->GetUserInformation(szUID,szFileds);
};
/**********************************************************************************************************/
DWORD VSClient::GetReceiverState(int ID,char *name)
{
	long flags=0;
	boost::shared_ptr<VS_StreamClientReceiver> rcv;
	int action = m_pProtocol->GetRcvAction(ID, name, rcv, flags);
	if (action == 2) {
		_variant_t var(flags);
		char rcv[256];
		sprintf(rcv, "%s>\\%s", name, CThinkClientReceiver::_funcSetRsvFltr);
		m_pReceivers->Process(SET_PARAM, rcv, &var);
	}
	return DWORD(flags);
}
/**********************************************************************************************************/
void VSClient::ForceBicubic(DWORD force){
	CVideoRenderBase::ForceBicubic(force);
};
/**********************************************************************************************************/
void VSClient::SetDisableDirect(DWORD iDd){
	CVSTrClientProc::m_dwSuppressDirectCon=iDd;
};
/**********************************************************************************************************/
void VSClient::GetFlags(int *pFlags){
	_variant_t var;
	pFlags[2] = 1;
	pFlags[3] = m_bDisableDirect;
	pFlags[5] = m_iFontSize;
	pFlags[6] = m_iSelfX;
	pFlags[7] = m_iSelfY;
	pFlags[8] = m_iMyColor;
	pFlags[10] = CVideoRenderBase::m_dwSaturation;
	Process(GET_PARAM,"Receivers\\AudioPlayback\\SystemVolume",&var);
	pFlags[11] = var;
	Process(GET_PARAM,"Sender\\AudioCapture\\MicrophoneVolume",&var);
	pFlags[12] = var;
	Process(GET_PARAM,"Sender\\VideoCapture\\CameraFPS",&var);
	pFlags[19] = var;
};
/**********************************************************************************************************/
void VSClient::SetJitter(int Mode){
};
/**********************************************************************************************************/
void VSClient::BanUser(char*UserID,int Strong){
	if(Strong)
		if(Strong<0)
			m_pProtocol->Ignore(UserID);
		else
			m_pProtocol->AddUserToAddressBook(UserID, AB_BAN_LIST);
	else
		m_pProtocol->Kick(UserID);
};
/**********************************************************************************************************/
void VSClient::SetFlags(int *pFlags){
	_variant_t var;

	m_bDisableDirect = !!pFlags[3];
	m_kKeyName->SetValue(&pFlags[3],sizeof(int),VS_REG_INTEGER_VT,m_DDFlag);

	m_iFontSize =pFlags[5];
	m_kKeyName->SetValue(&pFlags[5],sizeof(int),VS_REG_INTEGER_VT,m_FSize);

	m_iSelfX=pFlags[6];
	m_kKeyName->SetValue(&pFlags[6],sizeof(int),VS_REG_INTEGER_VT,m_SXFlag);
	m_iSelfY=pFlags[7];
	m_kKeyName->SetValue(&pFlags[7],sizeof(int),VS_REG_INTEGER_VT,m_SYFlag);
	m_iMyColor=pFlags[8];
	m_kKeyName->SetValue(&pFlags[8],sizeof(int),VS_REG_INTEGER_VT,m_MyColor);

	m_kKeyName->SetValue(&pFlags[10],sizeof(int),VS_REG_INTEGER_VT,m_Saturation);
}

/**********************************************************************************************************/
void VSClient::ReleaseSender()
{
	m_pThinkClientSender->Release();
}

/**********************************************************************************************************/
int VSClient::InitSender(int Width,int Height,int Freq)
{
	DWORD framerate = 0;
	DWORD freq_table[] = {8000, 11025, 16000, 22050, 32000, 44100, 48000};
	DWORD acodec_table[] = {VS_ACODEC_GSM610, VS_ACODEC_G723, VS_ACODEC_G729A, VS_ACODEC_G728, VS_ACODEC_G722, VS_ACODEC_SPEEX, VS_ACODEC_ISAC, VS_ACODEC_OPUS_B0914, VS_ACODEC_AAC, VS_ACODEC_MP3};
	DWORD vcodec_table[] = {VS_VCODEC_XC02, VS_VCODEC_H261, VS_VCODEC_H263, VS_VCODEC_H263P, VS_VCODEC_H264, VS_VCODEC_VPX, VS_VCODEC_VPXHD, VS_VCODEC_VPXSTEREO};

	DWORD audioDurr = -1;	// default value
	DWORD freq = 2;			// 16 kHz
	DWORD acodec = 7;		// 0 = GSM, 1 = G723, 2 = g729a, 3 = g728, 4 = g722, 5 = speex, 6 = isac, 7 = opus beta 0.9.14, 8 = AAC, 9 = MP3
	DWORD vcodec = 5;		// 0 = xc02, 1 = h261, 2 = h263, 3 = h263+, 4 = h264, 5 = vpx, 6 = vpx hd, vpx stereo
	DWORD format = 0;		// auto
	VS_RegistryKey key(true, REG_CurrentConfiguratuon);

#define CORRECT_BOUNDS(x, a, b) (x) = (x) < a ? a : ((x) > b ? b : (x));

	key.GetValue(&audioDurr, 4, VS_REG_INTEGER_VT, "AudioBufferDurr");
	if (audioDurr!=-1)
		CORRECT_BOUNDS(audioDurr, 30, 120)

	if (key.GetValue(&format, 4, VS_REG_INTEGER_VT, "Format") > 0) {
		freq = (format>>16)&0xff;
		CORRECT_BOUNDS(freq, 0, 6)
		framerate = (format>>24)&0xff;
		Width = (format & 0x000000ff) * 8;
		Height = ((format >> 8) & 0x000000ff) * 8;
	}
	if (key.GetValue(&format, 4, VS_REG_INTEGER_VT, "Format2") > 0) {
		Width = (format & 0x0000ffff) * 8;
		Height = ((format >> 16) & 0x0000ffff) * 8;
	}
	if (framerate == 0) framerate = 15;

	key.GetValue(&acodec, 4, VS_REG_INTEGER_VT, "AudioCodec");
	CORRECT_BOUNDS(acodec, 0, 9)

	key.GetValue(&vcodec, 4, VS_REG_INTEGER_VT, "VideoCodec");
	CORRECT_BOUNDS(vcodec, 0, 7)

	VS_MediaFormat mf;
	mf.SetAudio(freq_table[freq], acodec_table[acodec], audioDurr);
	mf.SetVideo(Width, Height, vcodec_table[vcodec], framerate);
	m_pThinkClientSender->m_DeviceStatus.bVideoValid = true;
	m_pThinkClientSender->m_DeviceStatus.bAudioValid = true;
	m_pThinkClientSender->Init(mf);

	return 0;
}

/**********************************************************************************************************/
int VSClient::GetMyName(char *UserName, char *FirstName, char *LastName){
	return m_pProtocol->GetMyName(UserName,FirstName,LastName);
};
/**********************************************************************************************************/
void VSClient::GetOtherName(char *UserName, char *FirstName, char *LastName){
	m_pProtocol->GetOtherName(UserName,FirstName,LastName);
};
/**********************************************************************************************************/
int VSClient::LoginToServer(char *pLogin,char *pPassword,int iAutoLogin, int Encrypt){
	return m_pProtocol->LoginUser(pLogin, pPassword, iAutoLogin, !Encrypt);
};
/**********************************************************************************************************/
int VSClient::AcceptProtocolConnect(){
	return m_pProtocol->Accept();
};
/**********************************************************************************************************/
void VSClient::RejectProtocolConnect(){
	m_pProtocol->Reject();
};
/**********************************************************************************************************/
void VSClient::HangupProtocolConnect(int Strong)
{
	m_pProtocol->Hangup(Strong);
}
/**********************************************************************************************************/
DWORD VSClient::BwtStart(HWND hwnd){return m_pProtocol->BwtStart(hwnd);}
DWORD VSClient::BwtStop(){return m_pProtocol->BwtStop();}
DWORD VSClient::BwtGet(void * out, int id){return m_pProtocol->BwtGet(out, id);}
void  VSClient::BwtWizardOn(int mode) {m_pProtocol->BwtWizardOn(mode);}
/**********************************************************************************************************/
void VSClient::LogoutServer()
{
	m_pProtocol->LogoutUser(true);
}
/**********************************************************************************************************/
void  VSClient::ConnectSender(HWND hReportHwnd,int Type)
{
	VS_MediaFormat mf;
	char MyId[MAX_PATH];
	m_pProtocol->GetMyName(MyId);
	m_pProtocol->GetMediaFormat(MyId,mf);

	m_pThinkClientSender->m_DeviceStatus.bAudioValid=mf.IsAudioValid();
	m_pThinkClientSender->m_DeviceStatus.bVideoValid=mf.IsVideoValid();

	m_pReceivers->RstConfStat();
	boost::shared_ptr<VS_StreamClientSender> sender = m_pProtocol->GetConferenseSender();
	if (!sender) {
		PostMessage(hReportHwnd, WM_USER+17, 3, NULL);
	}
	else {
		m_pThinkClientSender->Connect(sender, hReportHwnd,Type,m_pProtocol->m_Status.CurrConfInfo->ClientCaps.GetBandWRcv(), m_pProtocol->m_Status.CurrConfInfo->confType==5);
		m_pThinkClientSender->SetCurrentMedia(mf);
	}

};
/**********************************************************************************************************/
void  VSClient::DisconnectSender()
{
	m_pThinkClientSender->Disconnect();
	SendConfStat();
}

/**********************************************************************************************************/
int VSClient::InitDevicesList()
{
	int ret = 0;
	_variant_t vr;

	m_pProtocol->m_PropAudioCupture.Empty();
	if(Process(RUN_COMMAND,"Sender\\AudioCaptureDevices\\AudioCaptureList",&vr)!=VS_INTERFACE_OK) {
		ret|=1;
	}
	else {
		Process(GET_PARAM,"Sender\\AudioCaptureDevices\\AudioCaptureList",&vr);
		VARIANT *vars = 0;
		int num = ExtractVars(vars, &vr);
		for (int i=0; i<num; i++) {
			VS_WideStr str;
			str.Assign((_bstr_t)vars[i]);
			char* utf8(str.ToUTF8());
			m_pProtocol->m_PropAudioCupture+= utf8;
			free(utf8);
			m_pProtocol->m_PropAudioCupture+="\n";
		}
		if (num > 0) delete[] vars;
	}

	m_pProtocol->m_PropAudioRender.Empty();
	if(Process(RUN_COMMAND,"Receivers\\AudioPlayback\\AudioRenderList",&vr)!=VS_INTERFACE_OK)
		ret|=4;
	else {
		Process(GET_PARAM,"Receivers\\AudioPlayback\\AudioRenderList",&vr);
		VARIANT *vars = 0;
		int num = ExtractVars(vars, &vr);
		for (int i=0; i<num; i++) {
			VS_WideStr str;
			str.Assign((_bstr_t)vars[i]);
			char* utf8(str.ToUTF8());
			m_pProtocol->m_PropAudioRender+= utf8;
			free(utf8);
			m_pProtocol->m_PropAudioRender+="\n";
		}
		if (num > 0) delete[] vars;
	}

	m_pProtocol->m_PropVideoCupture.Empty();
	if(Process(RUN_COMMAND,"Sender\\VideoCaptureDevices\\VideoCaptureList",&vr)!=VS_INTERFACE_OK) {
		ret|=2;
	}
	else {
		Process(GET_PARAM,"Sender\\VideoCaptureDevices\\VideoCaptureList",&vr);
		VARIANT *vars = 0;
		int num = ExtractVars(vars, &vr);
		for (int i=0; i<num; i++) {
			VS_WideStr str;
			str.Assign((_bstr_t)vars[i]);
			char* utf8(str.ToUTF8());
			m_pProtocol->m_PropVideoCupture+= utf8;
			free(utf8);
			m_pProtocol->m_PropVideoCupture+="\n";
		}
		if (num > 0) delete[] vars;
	}
	return ret;
}

/**********************************************************************************************************/
void VSClient::SetSaturation(int iSaturation)
{
	CVideoRenderBase::m_dwSaturation=(DWORD)iSaturation;
}

/**********************************************************************************************************/
void VSClient::GetHistory(char*pMem)
{
	int i;
	char s[MAX_PATH];
	for(i=0;i<10;i++){
		sprintf(&(s[0]),"%s%d",m_History,i);
		if(m_kKeyName->GetValue(pMem+i*sizeof(char)*MAX_PATH,MAX_PATH,VS_REG_STRING_VT,&(s[0]))==0)
			(pMem+i*sizeof(char)*MAX_PATH)[0]=0;
	}
}

/**********************************************************************************************************/
void VSClient::SetHistory(char*pMem){
	int i;
	char s[MAX_PATH];
	for(i=0;i<10;i++){
		sprintf(&(s[0]),"%s%d",m_History,i);
		m_kKeyName->SetString(pMem+i*sizeof(char)*MAX_PATH, &(s[0]));
	}
};

/**********************************************************************************************************/
int VSClient::GetSenderTraffic(){
	return m_pThinkClientSender->GetTraffic();
};
/**********************************************************************************************************/
void VSClient::GetLoggedUsersList(void** UserList,int Type){
	m_pProtocol->GetUsersList(UserList,Type);
};
/**********************************************************************************************************/
DWORD VSClient::GetAutoLogin(){
	return m_pProtocol->GetAutoLogin();
};
/**********************************************************************************************************/
void VSClient::SetAutoLogin(DWORD ALogin){
	if(ALogin)
		m_pProtocol->SetAutoLogin(true);
	else
		m_pProtocol->SetAutoLogin(false);
};
/**********************************************************************************************************/
int VSClient::CreateLoopBackChannel(bool bAccept,int iPort){
	int ret=0;
	return ret;
};
/**********************************************************************************************************/
void VSClient::CloseLoopBackChannel(){
};
/**********************************************************************************************************/
void VSClient::SetServer(char *szName, char *szIP, char* szPort){

	if (!szName || !*szName) { //If !szName decode szIP as encoded connects and szPort = domain

		std::map<std::string, std::vector<net::endpoint::ConnectTCP>> cc;

		if (szIP && *szIP) { // have connects
			std::string par_h = szIP;
			char* currconn = (char*)par_h.c_str();
			char* nextconn = currconn;
			char* currname = "someserver#tmp";
			while (nextconn) {
				nextconn = (char*)strchr(currconn, ',');
				if (nextconn) {
					*nextconn = 0; nextconn++;
				}
				char* host = (char*)strchr(currconn, '\\');
				char* name = 0;
				if (host) {
					*host = 0; host++;
					name = currconn;
					currname = name;
				}
				else {
					name = currname;
					host = currconn;
				}
				char* port = (char*)strchr(host, ':');
				if (port) {
					*port = 0; port++;
				}
				else {
					port = "4307";
				}
				// MSVC 12.0 ICEs if temporary object is supplied to emplace_back.
				// TODO: simplify after migrating from MSVC 12.0.
				net::endpoint::ConnectTCP tmp_ctcp{ host, atoi(port), REG_DefaultServerProtocol };
				cc[name].emplace_back(std::move(tmp_ctcp));
				currconn = nextconn;
			}
		}

		int len = cc.size();
		for (const auto& kv : cc)
		{
			const char *name = kv.first.c_str();
			net::endpoint::ClearAllConnectTCP(name);
			for (const auto& tcp : kv.second)
				net::endpoint::AddConnectTCP(tcp, name);
			m_pProtocol->m_ServerList.SetEvent(name, VS_ServerList::SRVT_SERVERUPDATE, -1, szPort);
		}
		// Set server to connect
		if (len > 0)
			VS_WriteAS(cc.begin()->first.c_str());
		// set service
		if (szPort && *szPort) {
			VS_RegistryKey key(true, REG_CurrentConfiguratuon);
			key.SetString(szPort, "domain");
			m_pProtocol->SetDiscoveryServise(szPort);
		}
	}
	else if (szIP && *szIP) { // if szPort==0, szIP = filename.
		VS_WriteAS(szName);
		char buff[256];
		sprintf(buff, "%s\\%s", REG_Servers, szName);
		VS_RegistryKey keyS(true, buff, false, true);

		if (szPort) {
			unsigned short Port = atoi(szPort);
			net::endpoint::ClearAllConnectTCP(szName);
			net::endpoint::AddConnectTCP({ szIP, Port, REG_DefaultServerProtocol }, szName);
			m_pProtocol->m_ServerList.SetEvent(szName, VS_ServerList::SRVT_SERVERUPDATE);
		}
		else {
			FILE *f = NULL;
			if ((f = fopen(szIP, "rb"))!= NULL) {
				fseek(f, 0, SEEK_END);
				unsigned int size = ftell(f);
				fseek(f, 0, SEEK_SET);
				void *buffer = malloc(size);
				if (buffer && fread(buffer, 1, size, f)) {
					net::endpoint::Deserialize(true, buffer, size, szName);
					free(buffer);
				}
				fclose(f);
			}
		}
	}
};
void VSClient::UpdateExtStatus(const char *name, const int value)
{
	if(m_pProtocol)
		m_pProtocol->UpdateExtStatus(name,value);
}
void VSClient::UpdateExtStatus(const char *name, const wchar_t *value)
{
	if(m_pProtocol)
		m_pProtocol->UpdateExtStatus(name,value);
}
void VSClient::UpdateExtStatus(const int fwd_type, const wchar_t *fwd_call_id, const int fwd_timeout, const wchar_t *timeout_call_id)
{
	if(m_pProtocol)
		m_pProtocol->UpdateExtStatus(fwd_type,fwd_call_id,fwd_timeout,timeout_call_id);
}
/**********************************************************************************************************/
