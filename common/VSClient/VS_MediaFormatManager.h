
#ifndef _VS_MEDIAFORMAT_MANAGER_H_
#define _VS_MEDIAFORMAT_MANAGER_H_

#include "../std/cpplib/VS_MediaFormat.h"
#include "../std/cpplib/VS_VideoLevelCaps.h"
#include "../std/cpplib/VS_Lock.h"
#include "../std/cpplib/VS_Protocol.h"
#include "Transcoder/VS_SysBenchmarkBase.h"

class CVideoCaptureList;
class CVSTrClientProc;
class VS_ControlBandBase;

struct SwitchBitrateState
{
	int low_btr;
	int high_btr;
};

struct VS_VideoAutoFormatState
{
	const tc_AutoModeDesc_t	*pDescVM;
	int							last_bitrate;
};

enum eControlExt
{
	CTRL_EXT_HQAUTO = 0,
	CTRL_EXT_HDINPUTSOURCE = 1,
	CTRL_EXT_STEREO,
	CTRL_EXT_BITRATE,
	CTRL_EXT_SYSBENCH,
	CTRL_EXT_VIDEOCAPTURELIST,
	CTRL_EXT_VIDEODEVICENAME,
	CTRL_EXT_PROTOCOL,
	CTRL_EXT_LEVELCAPS,
	CTRL_EXT_CTRLBANDWIDTH,
};

class VS_MediaFormatManager : public VS_Lock
{
	/// external control
	bool m_bUseHQAuto;
	bool m_bUseHWEncoding;
	bool m_bUseHDInputSource;
	bool m_bUseStereo;
	bool m_bFixedRcvFormat;
	bool m_bCanRcvHWEncoding;
	bool m_bCanMul8;
	int m_iSetBitrate;
	int m_iConfType;
	eHardwareEncoder m_eTypeVideoEncoder;
	/// internal
	wchar_t					m_cVideoDeviceName[MAX_PATH];
	bool					m_bConnect;
	bool					m_bHQAutoCurrent;
	bool					m_bAllowHD;
	bool					m_bRcvDynamicFmt;
	bool					m_bScreenCapturer;
	unsigned char			m_uRcvLevel;
	long					m_iRcvScreenWidth;
	long					m_iRcvScreenHeight;
	VS_ClientType			m_clientType;
	VS_MediaFormat			m_confFmt;
	tc_AutoLevelCaps		*m_pAutoLevelCaps;
	int						m_iLastBitrate;
	unsigned char			m_uLevelBitrate;
	/// external interfaces
	CVideoCaptureList		*m_pVCaptureList;
	VS_SysBenchmarkBase		*m_pSysBench;
	tc_VideoLevelCaps		*m_pLevelCaps;
	CVSTrClientProc			*m_pProtocol;
	VS_ControlBandBase		*m_pBandCtrl;

	unsigned char CheckBitrate(int bitrate);
	void RestrictRcvScreenResolution(VS_MediaFormat *outFmt, int rcv_width, int rcv_height, tc_levelVideo_t descLvl);
public :
	VS_MediaFormatManager();
	~VS_MediaFormatManager();
	void SetControlExternal(eControlExt ctrl, int iVal);
	void SetControlExternal(eControlExt ctrl, void *pVal);
	VS_MediaFormat SetConnection();
	void ResetConnection();
	bool GetMediaFormat(VS_MediaFormat *inFmt, VS_MediaFormat *outFmt);
};

#endif /* _VS_MEDIAFORMAT_MANAGER_H_ */