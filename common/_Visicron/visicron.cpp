/**
 **************************************************************************
 * \file visicron.cpp
 * \brief contain dll interface functions implementation
 *
 * \b Project Visicron
 * \author Melehcko Ivan
 * \date 25.11.02
 *
 ****************************************************************************/

/*! \mainpage
 *
 * \par Documentation:
 * - \ref intro
 * - \ref install
 * - \ref reestr
 *  - \ref reestr1
 *  - \ref reestr2
 *  - \ref reestr3
 *  - \ref reestr4
 *  - \ref reestr5
 *  - \ref reestr6
 *  .
 * - \ref technologies
 *  - \ref technologies1
 *  - \ref technologies2
 *  - \ref technologies3
 *  - \ref technologies4
 *  .
 *
 * \section intro ������� ��������

Visicron.dll ������������ ����� ������, ��������������� ��� ���������� ������ � �������� ����������� ������������ ������� Visicron �. ������ �� ������������ ��������� �������� ����������������� ���������� � ��� ���������������� �������������� �������� ����������� ������������. ������ ������������ ��������� ����������������:
 -	������ ������� ��������� ����� � �����
 -	������ ����� ������ �� ��������� ��������� ������� �����
 -	����� �����������
 -	���������/������������ �����������
 -	������ ����� ������ �� ���������� ���������� ������� �����
 -	������������ ����������� �� ��������� ���������� ������������ �����
 -	���������/������������ �����������
 -	����������� ��������� �����������, ����������� ������������ ����� ������������ �����������
 -	����� � �������� � �������� ����������� ������ �� ������������� ��������� Visicron �. ��� ������������ ������� ��������������� ��� �������� ��������� � �������, ������������ ������������� � �������� ����������� ����������, ����� ���:
  -	����������� ������������
  -	��������� �����������
  -	������� �� �������� ����
  -	��������� ����
 -	�������� ����������� �� ������������� ��������� Visicron � � ������ �����������
 -	��������� ������� �������� ���������� � ������� Visicron �
 -	���������� �������� ����� � �������� �������� ����������� ����� ������� � �������� Visicron �.

 *****************************************************************************
 * \section install �����������
 *
 ���������� �� ����� ���������� �������� ����������� ����������
 *****************************************************************************
 * \section reestr ������ � ��������
 *
���������� �������� ������ ������������ � ������ � �����
\b HKEY_CURRENT_USER\\Software\\Visicron\\ApplicationName\\

\subsection reestr1 ����� ..\\Client\\
 - (t)	�\e ARenderStrictBuffDurr� � �� ������������
 -	�\e Bandwidth� � ������������ �������� ���������� �������� � �����������. ��������� �������� [3..2048]. ��������� �������� 128.
 -	�\e DisableDirect� � ��������� ��������� �������� �� �������� ����������� ����� �������� �� ������ IP � ����� � �������������� ���������� NHP. ��������� �������� 0.
 - (t)	�\e EnableNoiseGen� � �� ������������
 - (t)	�\e FPSRate� � ����������� ������������ ��������� ����� �������� ���������� ������ � �������� ������ �����, ��� ������, ��� ���� ��� � ������ ������� ������. ��������� �������� [8..56]. ��������� ������� 32.
 -	�\e InputBandwidth� � ������������ �������� ��������� �������� � ��������� �����������. ��������� �������� [3..4096]. ��������� �������� 128.
 -	�\e RenderAudioName� � ��������� ������������� ������������� ���������� ��������������� �����. ������ ���������� ����� ������������ ��� ���� ��������� � �����������.
 -	�\e RenderAudioVolume� � ���������� ����������� ��������� ���������������� �����. ��������� �������� [0..65536]. ��������� �������� 32767.
 -	�\e Saturation� � ����������� ������������ ����� ������������ �����������. ��������� �������� [0..200]. ��������� �������� 120.
 -	�\e UseDefaultAudioRender� � ��������� �������� ��������, ��� ������������ ��������� (������ � ������) ���������������.

\subsection reestr1_1 ����� ..\\Client\\AudioCaptureSlot\\
 -	�\e DeviceName� � ��������� ������������� ������������� ���������� ������� �����.
 - (t)	�\e EnableAGC� � ��������� �������� �������� �������� �������������� ����������� ��������. ��������� �������� 0.
 -	�\e UseDefaultDevice� � ��������� �������� ��������, ��� ������������ ��������� (������ � ������) ���������������.
 - 	�\e UseXPAec� � ��������� �������� �������� ������� ������������� � ������������� ��������� DirectSound Audio � �������� "�������������". � ���� ������ �� �������� ��������� �������� �������������. �� ��������� 0.
 -	�\e Volume� � ���������� ����������� ��������� ������� �����. ��������� �������� [0..65535]. ��������� �������� 65535.

\subsection reestr1_2 ����� ..\\Client\\VideoCaptureSlot\\
 -	�\e Channel� � ����� ������ ��������������� (S-video, Component, ... ect).
 - (t)	�\e CptFmt� � �������� ������� �����, ������������� � ��������� �����. �������� ��������: W/8 + (H/8)*256. ��� ������� �������� ������������ ������� �� ������������. ��������� �������� 0.
 - (t)	�\e Deinterlace� � ��������� �������� �������� ������������� ��������� ���������������. �� ��������� 0.
 -	�\e DeviceName� � ��������� ������������� ������������� ���������� ������� video.
 - (t)	�\e DVC90DimFix� � ��������� �������� �������� �� ������������ ���������� 352�288, ������������� � ���������������. ���������� ��� ���������� "DVC90". ��������� �������� 0.
 - (t)	�\e FrameRate� � ������������ ������� ���������� ������, ������������� � ���������� ������������. ��������� ��������: [5..30]. ��������� �������� 15.
 - (t)	�\e InvertUV� � ��������� �������� �������� ����� ������� U � V ���������� � ���������� YV12. �� ��������� 0.
 - (t)	�\e ScaleCapturedImage� � ��������� �������� �������� ���������� ��������� ��������������� ��� �������������� ���������� �������� ����� � ������������ ��������� �����, ����������� � ���������� �����. ��� ������� �������� ������������ �������� ���������/�������. ��������� �������� 0.
 -	�\e UseDefaultDevice� � ��������� �������� ��������, ��� ������������ ��������� (������ � ������) ���������������.
 -	�\e VideoMode� � ����� ������ ����� (NTSC, PAL,... ect).

\subsection reestr2 ����� ..\\Current configuration\\
 - (t)	�\e AudioBufferDurr� � ����� ������������ ����� ������ ����������� � ������������, ���������� ��������� �������� {-1, 30..120}. ��������� �������� -1.
 - (t)	�\e AudioCodec� � ����� ����������� ������������, ���������� ��������� ��������:
	- 0 - ����� GSM6.10
	- 1 - ����� g723.1
	- 2 - ����� g729a
	- 3 - ����� g728
	- 4 - ����� g722
	- 5 - ����� speex, WB, ������� ������������� ����� 16 ���
	- �� ��������� 1.
 - (t)	�\e BrLoadFactor� � �����������, ����������� �������� ������� ������������ � ������� ������� � ����������� ����-��-����. ��������� �������� [50..250]. ��������� �������� 100.
 - (t)	�\e BrAutoDisabled� � ������������� �������� �������� ��������������� �������������� ����������� �������� . ��������� �������� 0.
 - (t)	�\e Debug Modules� � ����� ��� ��������� ����������� ������� � �������� ������, ���������� ��������� �����:
	- 0 - �� ���� ������,
	- 1 - ������ �����
	- 2 - ������ ����� � ��������
	- 4 - ������ �����
	- 8 - ������ �������� � �������
	- 16 - ������ ����������� ���������� (�� ���)
	- �� ��������� 0.
 - (t)	�\e Debug OutType� � ����� ���� ������ ����������� � �������� ������, ���������� ��������� ��������:
	- 0 - ��� ������
	- 1 - ����� � ���� ��������
	- 2 - ����� � �������
	- 3 - ����� � ����
	- �� ��������� 0.
 -	�\e DefaultServer� � ��� ���������� �������.
 -	�\h DirectPort� � ���� ����������, �� �������� � ���� ����� �������������� ��������. �� ��������� 5050.
 - (t)	�\e ForceNhp� � ��������� �������� �������� ������������� NHP ���������� ��� ������� ����� NHP. �� ��������� 0.
 - (t)	�\e KeepAspectRatio� � ��������� �������� �������� ������������ ��������� ����������� ��� ��������� (��������, ��� ������� "����������") �� ��������� 1.
 -h	�\e MarkQos� � ��������� �������� �������� ������������� ���������� QOS ��� ���������� �����������. �� ��������� 0.
 -	�\e NetworkMode� � 	����� ���� �������� ���������� � ��������, ���������� ��������� ��������:
	- 0 - ������������ ������ ����������
	- 3 � ������������ HTTP ��������������
	- 2 � ������������ ��������� Windos
	- 1 � ������������ ������ ���������
	- �� ��������� 0.
 -	�\e NetworkType� � ��� ����������� ���������� � ����� (��������), ���������� ��������� ��������:
	- 0 � "I dont know"
	- 1 - "Dial-up"
	- 2 � "GPRS/EDGE"
	- 3 � "DSL"
	- 4 � "Cable"
	- 5 - "T1"
	- 6 � "Wi-Fi"
	- �� ��������� 0.
 -	�\e ProxyHost� � ������� ��� ��� ������� ����� ������-�������
 -	�\e ProxyPort� � ���� ������-�������
 -	�\e ProxyType� � ��� ������-�������, ���������� ��������� ��������:
	- 0 � HTTP
	- 1 � SOCKS 4
	- 2 � SOCKS 5
 -	�\e ProxyUserName� � ��� ������������ ������-�������
 -	�\e ProxyPassword� � ������ ������������ ������-�������
 - (t)	�\e ResizeMode� �  ����� ������ ��������������� �����, ���������� ��������� ��������:
	- 0 - ������� �����, ���������+������, ������� ��������, �� ������, ������ ��������
	- 1 - ����������� ��������, �������� ��������, ��� �����, ��������� ��������
	- 2 - ����������� �������, ������� ��������, ��� �����, ������� ��������
	- 3 - ���������������� ����������� �������, ���������� ��������, ������������ ��������.
	- �� ��������� 0.
 -	�\e Server� � ����������������� ������������� �������� �������.
 - (t)	�\e UseNhp� � ��������� �������� �������� ���������� ����� NHP. �� ��������� 0.
 - (t)	�\e VideoCodec� �  ����� ����������� ������������, ���������� ��������� ��������:
	- 0 - ����� xc02
	- 1 - ����� h261
	- 2 - ����� h263
	- 3 - ����� h263+
	- 4 - ����� h264
	- �� ��������� 0.

\subsection reestr3 ����� ..\\Current configuration\\Servers\\
��� ����� �������� ������ �����, ������������ ��������, ������� ��������� � ������� �������. �������� ����� � ������������������ ���������� �������� (XXXX).

\subsection reestr4 ����� ..\\Current configuration\\Servers\\XXXX\\
 -	�\e Endpoint� � ��� ��������� ����������� ���������� (���������� ������������� ���������� � ������� Visicron �.
 -	�\e HomeBroker� � ��� ��������� �������, � �������� �������������� ���������� ����������. �� ��������� XXXX:1.
 -	�\e Key� � ��������� �������������, ������������ ������ �������� ����������� ���������� �� ������ �������
 -	�\e NetworkTest� � ��������� ������������ �������� ����� � �������� ����.

\subsection reestr5 ����� ..\\Current configuration\\Servers\\XXXX\\XXXX:YYYY\\
 -	�\e LastUpdate� �  ����� ���������� ���������� ���������� � ������ �������� �������

\subsection reestr6 ����� ..\\Endpoints\\
�������� ����� � ������� ����������, ���������� ������ ����������� � ��� �� ����. (��. ������������ � ����������� ����������).

 *****************************************************************************
 * \section technologies ������������ ����������
 *
 * \subsection technologies1 C����� �����
 *
	��� ������ ����������� � ������� Visicron � ������������ ����� Cyclon, ������������� �������������� �������� Visicron � ���������� ��� ���������� � �����������������. � ������ ��������� ��������� ���������� � ������� ������ �����:
 -	Wavelet ������ �  ����������� �������� ����������� �� ������� ��������� �� �������� ������.
 -	��������� ����������� �������� ����������� ���������� ������� ���� � ��� ��������� ���������� ����� ������� � ������� ���������, ��� �������� ������������ ���������� ������ ����������� � ����������������.
 -	��� ������ ����������: ������������� �����������, ���������� � ����� �����������, ���������� ��������������� ����������� ���� �������� ������������ � �������� ����������� �� ��������� �� ������������ �������������.
 -	����������� ��������� �� ��������, � ����������� 3-� ��������� ������� ����������� ��������, ����������� ��� ��������� ���� ����������� ��������� ������������ ���������� ���������� �� ��������� ������� ��� �������� �������� � �������� ������ ����������� ����������.

 *****************************************************************************
 * \subsection technologies2 ����������� �����
 *
	��� ����������� ����� � ���������� ���������� ������ ���������� ���������� DirectX, ����������� �� ��������� ��������� ��� ����������� ����������. � ������ ������������ ������ ��������������� �����������, ������������� �������������� Visicron �, ������� �������� ���������� ����������� �������� ���������� ������������ ��������� Bicubic. ����� ���� ������ �������� �������� � ��������� ��� ������� ��������� Bicubic.

 *****************************************************************************
 * \subsection technologies3 ������������ ����������� � ����������� �������� ��� �������� �����
 *
	� �������� ������� ������ ���������� ���������� � ��������� �������� ����������� ����� ������������ ����� �� ���������� � ����������� ��������. � ���� ������ �������� ����� ����� ���������� ������������ �������� � 0.5 ������ � ����� (��� ���������� ������). ���� ���������� �� ����� � ������ ������ ��� ������������ ����� � ���� ������ � ������ �����, ��������� �������� �� ���� ����� � �����, � ����� ������������������ �� ����� ���� ����. ��� ���������� ����� �������� � ������ ����������� ���������� ������������ �����������. �������� ������ ������� ������������ � ������, � ����� ���������� �������������. ���� ��������� ����� � ������, �� ������ � ������� ������� ��� ������������ �� ����� ���� �����.
	� ����������� ������������ ����������� ������ �� ������ ��������� ������� �������� ������ ��� ����� ������ ��� ��������� ������� � ����. ���� ����� ������� ������ ��� ������������� �����, ����� ������ ���� ������� �������� �����������, �������� � ����������� �Video On Demend�, ����� ������ ���������� �������� ������� ����� �������� ������-���� ������� � �������� �� ��������������� � ����������. ��� ������ �������� ������, ��� ������ ��� ������� (�������� ��� ���������������) �� �������. � ������� �� ������������� �����, � �������� ��������������� ��������� ������� ������ ������������� ����������� ������ �������� �������, ��� ��� ��� ����� ��������� � �������� ������ ����������� ����������� �������� �����. ��������� �������������� ������: � ����� ������� ���������� �������� ������ ����, ���������� ������ ��������� �������, � ������ � ��������� ������ ������� ��� ���������� ����������� ��������. ��� ���� ���� � ������ �� ������������� ��������� �������������� ����� �������, �� ��������� ���������� �������������� ������ ������������� �������� �������� �� ������ ������ �������.
 *****************************************************************************
 * \subsection technologies4 ����������� ������������� � ����������� ������ ��� ��������������� �����
 *
	���� ����� � ������ ����������� ��������� ������������� ��������, � ������ � �������� ������� ��������� ������������ ��� ������������ ������������, ����� ���������� ����� � ��������������� �����.
	��� ���������� ���� �������� � ������ ���������� ���������� ��������� �������� ����, �������	����������� ����� ����� � ���������������, ��� ����� �������������� ������������� ��������������� �����. � ����������������� ������ ��������, ������ � ������� ����� ��������� ������, � ����� �� ��� ����� �������. ��� ����������� ����������� � ��� �������� �� ���������� ������� ��������� (�������� ����� ������, ���������� ������) � ������� ��������� ����������� ���������� ����� (VAD). ��� ���
�������������� ����������� ������ ��������������� ������ ������� ����������.
 ****************************************************************************/

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "../ClientInterface/ClientInterface.h"
#include "Utils.h"

int VS_HardwareTest();

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************
 * Functions
 ****************************************************************************/
__declspec(dllexport)DWORD VSProcess(DWORD_PTR lParam,int Operation,char*szInterface,VARIANT *pVar){return Process(lParam,Operation,szInterface,pVar);};
__declspec(dllexport)int VSInterfaceGetVersion(DWORD_PTR pGUI_version,DWORD_PTR pAPP_version){ return InterfaceGetVersion(pGUI_version,pAPP_version);};
__declspec(dllexport)DWORD_PTR VSInitialize(DWORD_PTR dwFlag,HWND hwnd,char*szRegInit,char *szHomeDomain){return Initialize(dwFlag,hwnd,szRegInit,szHomeDomain);};
__declspec(dllexport)void VSRelease(DWORD_PTR lParam){Release(lParam);};
__declspec(dllexport)int  VSStartSender(DWORD_PTR lParam/*,DWORD_PTR pAudio,DWORD_PTR pVideo,HWND hwnd,DWORD_PTR pProc,LPVOID* inst*/,int Width,int Height,int Freq){return StartSender(lParam/*,pAudio,pVideo,hwnd,(RENDERPROC*)pProc,inst*/,Width,Height,Freq);};
__declspec(dllexport)void VSStopSender(DWORD_PTR lParam){ StopSender(lParam);};
__declspec(dllexport)void VSSetEchoMode(DWORD_PTR lParam, int mode){}
__declspec(dllexport)void VSConnectSender(DWORD_PTR lParam,HWND hReportHwnd,DWORD_PTR Type){ConnectSender(lParam,hReportHwnd,Type);};
__declspec(dllexport)void VSDisconnectSender(DWORD_PTR lParam){DisconnectSender(lParam);};
__declspec(dllexport)void VSSetHistory(DWORD_PTR lParam,DWORD_PTR lParam1){SetHistory(lParam,lParam1);};
__declspec(dllexport)void VSGetHistory(DWORD_PTR lParam,DWORD_PTR lParam1){GetHistory(lParam,lParam1);};
__declspec(dllexport)void VSGetTraffic(DWORD_PTR lParam,DWORD_PTR lParam1,DWORD_PTR lParam2){GetTraffic(lParam,lParam1,lParam2);};
__declspec(dllexport)void VSSetFlags(DWORD_PTR lParam,DWORD_PTR lParam1){SetFlags(lParam,lParam1);};
__declspec(dllexport)void VSGetFlags(DWORD_PTR lParam,DWORD_PTR lParam1){GetFlags(lParam,lParam1);};
__declspec(dllexport)void VSSetDisableDirect(DWORD_PTR lParam,DWORD_PTR lParam1){SetDisableDirect(lParam,lParam1);};
__declspec(dllexport)void VSForceBicubic(DWORD_PTR lParam,DWORD_PTR force){ForceBicubic(lParam,force);};
__declspec(dllexport)int VSLoginToServer(DWORD_PTR lParam, DWORD_PTR pLogin,DWORD_PTR pPassword,DWORD_PTR iAutoLogin, int Encrypt){return LoginToServer(lParam,pLogin,pPassword,iAutoLogin, Encrypt);};
__declspec(dllexport)void VSLogoutServer(DWORD_PTR lParam){LogoutServer(lParam);};
__declspec(dllexport)int  VSGetMyName(DWORD_PTR lParam,DWORD_PTR UserName,DWORD_PTR FirstName,DWORD_PTR LastName){return GetMyName(lParam,UserName,FirstName,LastName);};
__declspec(dllexport)void VSGetOtherName(DWORD_PTR lParam,DWORD_PTR UserName,DWORD_PTR FirstName,DWORD_PTR LastName){GetOtherName(lParam,UserName,FirstName,LastName);};
__declspec(dllexport)int  VSAcceptProtocolConnect(DWORD_PTR lParam){return AcceptProtocolConnect(lParam);};
__declspec(dllexport)void VSRejectProtocolConnect(DWORD_PTR lParam){RejectProtocolConnect(lParam);};
__declspec(dllexport)void VSHangupProtocolConnect(DWORD_PTR lParam,int Strong){HangupProtocolConnect(lParam,Strong);};
__declspec(dllexport)DWORD VSBwtStart(DWORD_PTR lParam, DWORD_PTR hwnd){return BwtStart(lParam, hwnd);}
__declspec(dllexport)DWORD VSBwtStop(DWORD_PTR lParam){return BwtStop(lParam);}
__declspec(dllexport)DWORD VSBwtGet(DWORD_PTR lParam, DWORD_PTR Status, DWORD_PTR Id){return BwtGet(lParam, Status, Id);}
__declspec(dllexport)void VSBwtWizardOn(DWORD_PTR lParam, int mode){BwtWizardOn(lParam, mode);}
__declspec(dllexport)DWORD VSSetProxySet(DWORD_PTR array){return SetProxySet(array);}
__declspec(dllexport)void  VSGetProxySet(DWORD_PTR array){GetProxySet(array);}
__declspec(dllexport)void  VSSetProxyDialog(DWORD_PTR func){SetProxyDialog(func);}
__declspec(dllexport)void  VSProxyDialogEnd(int ret){ProxyDialogEnd(ret);}
__declspec(dllexport)void  VSSetProxyNetMode(int* cfg){SetProxyNetMode(cfg);}
__declspec(dllexport)int   VSSetNetType(int mode){return SetNetType(mode);}
__declspec(dllexport)int   VSSetManualPort(unsigned short* port, char* host, int mode){return SetManualPort(port, host, mode);}
__declspec(dllexport)void VSGetSkinName(DWORD_PTR lParam,DWORD_PTR FileName){GetSkinName(lParam,FileName);};
__declspec(dllexport)void VSSetSkinName(DWORD_PTR lParam,DWORD_PTR FileName){SetSkinName(lParam,FileName);};
__declspec(dllexport)void VSSendMessage(DWORD_PTR lParam,DWORD_PTR pMessage,DWORD_PTR szOther){SendMessages(lParam,pMessage,(void*)szOther);};
__declspec(dllexport)void VSSendCommand(DWORD_PTR lParam, char* command, char* to){SendCommands(lParam, command, to);}
__declspec(dllexport)int  VSGetServers(DWORD_PTR lParam,DWORD_PTR size,DWORD_PTR pName,DWORD_PTR pDesk){return GetServers(lParam,size,pName,pDesk);};
__declspec(dllexport)int  VSGetDefaultServer(DWORD_PTR lParam, DWORD_PTR name){return GetDefaultServer(lParam, name);}
__declspec(dllexport)void VSSetCurrentServer(DWORD_PTR lParam,DWORD_PTR pName){SetCurrentServer(lParam,pName);};
__declspec(dllexport)void VSSetSaturation(DWORD_PTR lParam,DWORD_PTR iSaturation){SetSaturation(lParam,iSaturation);};
__declspec(dllexport)void VSGetLoggedUsersList(DWORD_PTR lParam,DWORD_PTR UserList,int Type){GetLoggedUsersList(lParam,UserList,Type);};
__declspec(dllexport)DWORD VSGetProperty(DWORD_PTR lParam,DWORD_PTR lParam1,DWORD_PTR lParam2){return GetProperty(lParam,lParam1,lParam2);};
__declspec(dllexport)int  VSGetChatMessage(DWORD_PTR lParam,int Id, char* From, char* Mess, char* to, char* Dn){return GetChatMessage(lParam,Id,From,Mess,to,Dn);};
__declspec(dllexport)int  VSGetChatMessageV2(DWORD_PTR lParam,int Id, char* From, char* Mess, char* to, char* Dn, long long *time){return GetChatMessageV2(lParam,Id,From,Mess,to,Dn, time);};
__declspec(dllexport)void  VSGetCommandMessage(DWORD_PTR lParam,int Id, char* From, char* Command){GetCommandMessage(lParam, Id, From, Command);}
__declspec(dllexport)void  VSBanUser(DWORD_PTR lParam,char*UserID,int Strong){BanUser(lParam,UserID,Strong);};
__declspec(dllexport)void  VSSetServer(DWORD_PTR lParam,char *szName,char *szIP,char*szPort){SetServer(lParam,szName,szIP,szPort);};
__declspec(dllexport)DWORD VSAddressBook(DWORD_PTR lParam,DWORD abCommand,char*param,long addressBook, long hash){return AddressBook(lParam,abCommand,param,addressBook, hash);};
__declspec(dllexport)DWORD VSQueryUserInfo(DWORD_PTR lParam,char*szUID){return QueryUserInfo(lParam,szUID);};
__declspec(dllexport)DWORD VSReadUserInfo(DWORD_PTR lParam,char*szUID,char**szFileds){return ReadUserInfo(lParam,szUID,szFileds);};
__declspec(dllexport)void  VSSetAutologin(DWORD_PTR lParam,DWORD ALogin){SetAutologin(lParam,ALogin);};
__declspec(dllexport)DWORD VSGetAutologin(DWORD_PTR lParam){return GetAutologin(lParam);};
__declspec(dllexport)void  VSCommOperation(DWORD_PTR lParam,char*szPort,int *pSpeed,int *pMode,int read){CommOperation(lParam,szPort,pSpeed,pMode,read);};
__declspec(dllexport)void  VSCommEnabled(DWORD_PTR lParam,int iEnabled){CommEnabled(lParam,iEnabled);};
__declspec(dllexport)void  VSSetJitter(DWORD_PTR lParam,int Mode){SetJitter(lParam,Mode);};
__declspec(dllexport)int   VSCreateLoopBackChannel(DWORD_PTR lParam,int bAccept,int iPort){return CreateLoopBackChannel(lParam,bAccept,iPort);};
__declspec(dllexport)void  VSCloseLoopBackChannel(DWORD_PTR lParam){CloseLoopBackChannel(lParam);};
__declspec(dllexport)DWORD VSGetReceiverState(DWORD_PTR lParam,int ID,char *name){return GetReceiverState(lParam,ID,name);};
__declspec(dllexport)void  VSSetTrackCallBack(DWORD_PTR lParam,void *pCallBack,void*pParam){SetTrackCallBack(lParam,pCallBack,pParam);};
__declspec(dllexport)void  VSSetExtTrackCallBack(DWORD_PTR lParam,void *pCallBack,void*pParam){SetExtTrackCallBack(lParam,pCallBack,pParam);};
__declspec(dllexport)void  VSHardwareTest(){VS_HardwareTest();}
__declspec(dllexport)int   VSCopyRegKeys(char *s, char* d){return VS_CopyRegKeys(s, d);}
__declspec(dllexport)int   VSDetectAVXSupport(){ return VS_CheckAVXOnInstall(); }
/**********************************************************/

#ifdef __cplusplus
}
#endif
