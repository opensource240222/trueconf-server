/**
 **************************************************************************
 * \file VSAudio.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Implement Volume Control, TestAudio Render, AudioRenders wraper
 *
 * \b Project Client
 * \author Melecko Ivan
 * \author SMirnovK
 * \date 01.11.2002
 *
 * $Revision: 3 $
 *
 * $History: VSAudio.h $
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 23.08.10   Time: 22:19
 * Updated in $/VSNA/VSClient
 * - long names in devices
 * - corrected Wide names for devices
 * - init devices section rewrited
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 18.01.08   Time: 17:02
 * Updated in $/VSNA/VSClient
 * - bigfixes from old architecture.
 * - ambiguous code marked as "old code"
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 24.12.07   Time: 17:37
 * Updated in $/VS2005/VSClient
 * - master volume added
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 44  *****************
 * User: Smirnov      Date: 15.12.06   Time: 17:18
 * Updated in $/VS/vsclient
 * - level and volume returned in audiotest
 *
 * *****************  Version 43  *****************
 * User: Smirnov      Date: 29.09.06   Time: 14:02
 * Updated in $/VS/VSClient
 * - programm audio mute
 *
 * *****************  Version 42  *****************
 * User: Smirnov      Date: 25.07.06   Time: 18:09
 * Updated in $/VS/VSClient
 * - Added system waveout Mute
 *
 * *****************  Version 41  *****************
 * User: Smirnov      Date: 13.07.06   Time: 18:56
 * Updated in $/VS/VSClient
 * - new audio controls (via mixer)
 *
 * *****************  Version 40  *****************
 * User: Smirnov      Date: 20.04.06   Time: 13:46
 * Updated in $/VS/VSClient
 * - new audio hardware test
 *
 * *****************  Version 39  *****************
 * User: Smirnov      Date: 16.02.06   Time: 12:15
 * Updated in $/VS/VSClient
 * - new TheadBase class
 * - receiver now can be inited while in conference
 *
 * *****************  Version 38  *****************
 * User: Smirnov      Date: 12.09.05   Time: 14:41
 * Updated in $/VS/VSClient
 * - added new codecs support in client: g728, g729a, g722.1
 *
 * *****************  Version 37  *****************
 * User: Admin        Date: 16.12.04   Time: 20:08
 * Updated in $/VS/VSClient
 * doxigen comments
 *
 * *****************  Version 36  *****************
 * User: Smirnov      Date: 30.11.04   Time: 20:54
 * Updated in $/VS/VSClient
 * removed aold audio
 *
 * *****************  Version 35  *****************
 * User: Melechko     Date: 24.11.04   Time: 15:57
 * Updated in $/VS/VSClient
 * Move read/write paramets VideeoCapture AudioCapture
 *
 * *****************  Version 34  *****************
 * User: Smirnov      Date: 29.07.04   Time: 14:04
 * Updated in $/VS/VSClient
 * added support for many audio codecs
 * Added AudioCodec G723
 *
 * *****************  Version 33  *****************
 * User: Melechko     Date: 30.03.04   Time: 18:37
 * Updated in $/VS/VSClient
 * Add audio prepare call
 *
 * *****************  Version 32  *****************
 * User: Melechko     Date: 22.03.04   Time: 10:32
 * Updated in $/VS/VSClient
 * no noise if micropone off
 *
 * *****************  Version 31  *****************
 * User: Melechko     Date: 15.03.04   Time: 15:19
 * Updated in $/VS/VSClient
 * Fix receivers operation
 *
 * *****************  Version 30  *****************
 * User: Melechko     Date: 10.03.04   Time: 15:47
 * Updated in $/VS/VSClient
 * Move volume control and audio test
 *
 * *****************  Version 29  *****************
 * User: Melechko     Date: 10.03.04   Time: 11:54
 * Updated in $/VS/VSClient
 * Add ReceiversPool class
 *
 * *****************  Version 28  *****************
 * User: Melechko     Date: 9.03.04    Time: 12:24
 * Updated in $/VS/VSClient
 * Audio capture with new interface
 *
 * *****************  Version 27  *****************
 * User: Smirnov      Date: 2.03.04    Time: 19:21
 * Updated in $/VS/VSClient
 * corrected buffer duration costants
 *
 * *****************  Version 26  *****************
 * User: Smirnov      Date: 2.03.04    Time: 16:34
 * Updated in $/VS/VSClient
 * removed vs_trace, calc Levels func,
 * different buffer sizes
 *
 * *****************  Version 25  *****************
 * User: Melechko     Date: 11.02.04   Time: 14:59
 * Updated in $/VS/VSClient
 * New Sender:loop
 *
 * *****************  Version 24  *****************
 * User: Melechko     Date: 21.01.04   Time: 17:56
 * Updated in $/VS/VSClient
 * Add Low latency mode
 *
 * *****************  Version 23  *****************
 * User: Smirnov      Date: 24.11.03   Time: 19:34
 * Updated in $/VS/VSClient
 * added every audiorender setVolukme support
 *
 * *****************  Version 22  *****************
 * User: Smirnov      Date: 19.11.03   Time: 13:32
 * Updated in $/VS/VSClient
 * general volume control
 *
 * *****************  Version 21  *****************
 * User: Smirnov      Date: 3.09.03    Time: 15:20
 * Updated in $/VS/VSClient
 * bounds checker
 *
 * *****************  Version 20  *****************
 * User: Melechko     Date: 2.04.03    Time: 16:31
 * Updated in $/VS/VSClient
 * Hardware comp
 *
 ****************************************************************************/

#ifndef _VSAUDIO_
#define _VSAUDIO_

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "VSClientBase.h"
#include "VSAudioNew.h"
#include "VS_VolControl.h"
#include "AudioProcessing\VS_AudioProcessing.h"

/****************************************************************************
 * Classes
 ****************************************************************************/

/**
 **************************************************************************
 * \brief Special class to test audio render
 ****************************************************************************/
class VSTestAudio: public CVSThread
{
public:
	enum ETestMode
	{
		TM_NONE,
		TM_CAPTURE,
		TM_RENDER,
		TM_CAPTURE_RENDER,
		TM_ECHO_DELAY
	};

	/// set to zero members
	VSTestAudio();
	/// Close runing thread
	~VSTestAudio();
	/// start testing thread
	DWORD Start(VS_MediaFormat *fmt, int RenderId, int CaptureId, ETestMode mode);
	/// stop testing thread
	DWORD Stop();
	/// Set capture volume
	void SetVolume(DWORD vol);
	/// Get capture volume
	int GetVolume();
	/// Get capture level
	DWORD GetLevel();
	/// Get delay test status
	int GetDelayTestStatus();
private:
	ETestMode m_Mode;
	int m_DelayTestStatus = DTS_NOT_STARTED;

	enum EDelayTestStatus
	{
		DTS_NOT_STARTED,
		DTS_ON_PROCESS,
		DTS_DELAY_FOUNDED,
		DTS_NO_ECHO,
		DTS_MULTIPLE_ECHO
	};

	VS_ARenderDevice*	m_render;
	VS_ACaptureDevice*	m_capture;

	std::wstring m_RenderName;
	std::wstring m_CaptureName;

	/// see CVSThread
	DWORD Loop(LPVOID lpParameter);
};


/**
 **************************************************************************
 * \brief Control System volume and VSTestAudioRender and retrive avaliable
 * devices
 ****************************************************************************/
class CRenderAudioDevices: public CDeviceList, public CVSInterface
{
protected:
	VSTestAudio 			m_TestAudio;		///< VSTestAudioRender
	VS_VolControlBase*		m_VolumeControl;	///< System volume class (Wave Volume)
	VS_AudioMixerVolume		m_MasterVolume;		///< System Master volume class (Wave Volume)

	/// see CVSInterface
	int ProcessFunction(VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar) override;
public:
	/// init variables
	CRenderAudioDevices(CVSInterface* pParentInterface);
	/// release resources
	~CRenderAudioDevices();
	/// retrive audio render modes
	int iGetDeviceModeList(int device_number);
	/// retrive avaliable devices
	int iGetDeviceList();
	static int		     m_LevelOut;		    ///< audio level (temporary solution with static member)
};

#endif