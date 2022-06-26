/**
 **************************************************************************
 * \file VSAudioCaptureSlot.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief VideoAudioSlot class declaration
 * \b Project Client
 * \author Melechko Ivan
 * \date 14.01.2005
 *
 * $Revision: 1 $
 *
 * $History: VSAudioCaptureSlot.h $
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/VSClient
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 25.04.06   Time: 14:05
 * Updated in $/VS/VSClient
 * - sender and it devices classes documented, code cleared
 *
 * *****************  Version 2  *****************
 * User: Melechko     Date: 24.02.05   Time: 12:39
 * Updated in $/VS/VSClient
 * Add CaptureSlotExt
 *
 * *****************  Version 1  *****************
 * User: Melechko     Date: 19.01.05   Time: 17:00
 * Created in $/VS/VSClient
 * some changes :)
 *
*/
#ifndef _VSAUDCAP_SLOT_H
#define _VSAUDCAP_SLOT_H
/****************************************************************************
 * Includes
 ****************************************************************************/
#include "VSInterface.h"
#include "VSThinkClient.h"
/****************************************************************************
 * Declarations
 ****************************************************************************/
class CAudioCaptureList;
class VS_AudioReSampler;

#define BUFFER_MAX 32

/**
 **************************************************************************
 * \brief Contain audio device
 ****************************************************************************/
class CAudioCaptureSlot: public CVSInterface{
	VS_AudioCapture *	m_pAudioCapture;///< audio device
protected:
	bool				m_bValid;		///< validity flag
public:
	/// zero members
	CAudioCaptureSlot(const char *szSlotName,CVSInterface* pParentInterface,VS_MediaFormat &mf,CAudioCaptureList *pAudioCaptureList);
	/// do nothing
	virtual ~CAudioCaptureSlot() {}
	/// create audio device
	virtual void _Init(VS_MediaFormat &mf,CAudioCaptureList *pAudioCaptureList);
	/// delte audio device
	virtual void _Release();
	/// read data from audio device
	virtual int Capture(char* buff, int &size, bool use_audio);
	/// return validity state
	bool IsValid(){return m_bValid;};
	/// Start audiodevice
	void Start();
	/// return incoming data event
	virtual HANDLE GetAudioEvent();
};

/**
 **************************************************************************
 * \brief	can receive uncompressed audio from GUI, convert
 *			to appropriate format, compress it
 ****************************************************************************/
class CAudioCaptureSlotExt: public CAudioCaptureSlot{
	const static char _funcSetFormat[];
	const static char _funcPushData[];
	WAVEFORMATEX		m_wf;				///< current audio format
	WAVEFORMATEX		m_wf_in	;			///< current audio format
	AudioCodec*			m_codec;			///< pointer to audio coder
	VS_AudioReSampler*  m_resampler;		///< audio data resampler
	int					m_pos;				///< last number of ressampled buffer
	int					m_data;				///< number of ressampled buffers
	int					m_size;				///< sizeof one buffer
	HANDLE				m_hEvent;			///< incoming data event
	unsigned char *		m_Buffer[BUFFER_MAX];///< buffers to ressampled data

	/// resample incoming data and put it to buffer
	void PrepareBuffer(unsigned char *pBuff,int iSize);
	/// init audio codec
	bool Init(VS_MediaFormat &mf);
	/// delete codec, reset buffers
	void Release();
protected:
	int ProcessFunction(VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar) override;
public:
	/// zero and reset members
	CAudioCaptureSlotExt(const char *szSlotName,CVSInterface* pParentInterface,VS_MediaFormat &mf,CAudioCaptureList *pAudioCaptureList);
	/// create resampler and incoming data event
	virtual void _Init(VS_MediaFormat &mf,CAudioCaptureList *pAudioCaptureList);
	/// detlete resampler and incoming data event
	virtual void _Release();
	/// read data from intrnal buffer and compress it to input buffer
	virtual int Capture(char* buff, int &size, bool use_audio);
	/// return incoming data event
	virtual HANDLE GetAudioEvent();
	/// check type of class
	virtual bool IsInterfaceSupport(VS_INTERFACE_TYPE TypeChecked){return TypeChecked==IT_AUDIOCAPTURE;};
};

#endif
