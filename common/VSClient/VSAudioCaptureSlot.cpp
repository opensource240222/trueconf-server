/**
 **************************************************************************
 * \file VSAudioCaptureSlot.cpp
 * (c) 2002-2005 Visicron Inc.  http://www.visicron.net/
 * \brief Implement
 * \b Project Client CVSAudioCaptureSlot class
 * \author Melechko Ivan
 * \date 14.01.2005
 *
 * $Revision: 2 $
 *
 * $History: VSAudioCaptureSlot.cpp $
 *
 * *****************  Version 2  *****************
 * User: Sanufriev    Date: 15.09.10   Time: 16:23
 * Updated in $/VSNA/VSClient
 * - fix DS bug (overflow)
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/VSClient
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 12  *****************
 * User: Smirnov      Date: 25.04.06   Time: 14:05
 * Updated in $/VS/VSClient
 * - sender and it devices classes documented, code cleared
 *
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 16.02.06   Time: 12:15
 * Updated in $/VS/VSClient
 * - new TheadBase class
 * - receiver now can be inited while in conference
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 6.10.05    Time: 14:18
 * Updated in $/VS/VSClient
 * - ipp audiocodecs now application depended
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 28.09.05   Time: 18:54
 * Updated in $/VS/VSClient
 * - g722 removed from CLient library
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 12.09.05   Time: 14:41
 * Updated in $/VS/VSClient
 * - added new codecs support in client: g728, g729a, g722.1
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 27.05.05   Time: 16:08
 * Updated in $/VS/VSClient
 * aded new IPP ver 4.1
 * added g711, g728, g729 from IPP
 *
 * *****************  Version 6  *****************
 * User: Melechko     Date: 3.03.05    Time: 14:05
 * Updated in $/VS/VSClient
 * Add reset event for AudioSlotExt
 *
 * *****************  Version 5  *****************
 * User: Melechko     Date: 2.03.05    Time: 12:07
 * Updated in $/VS/VSClient
 *
 * *****************  Version 4  *****************
 * User: Melechko     Date: 28.02.05   Time: 11:45
 * Updated in $/VS/VSClient
 * Add audio event in SlotEx
 *
 * *****************  Version 3  *****************
 * User: Melechko     Date: 24.02.05   Time: 12:39
 * Updated in $/VS/VSClient
 * Add CaptureSlotExt
 *
 * *****************  Version 2  *****************
 * User: Melechko     Date: 19.01.05   Time: 18:01
 * Updated in $/VS/VSClient
 * Add re-init audio format
 *
 * *****************  Version 1  *****************
 * User: Melechko     Date: 19.01.05   Time: 17:00
 * Created in $/VS/VSClient
 * some changes :)
 *
*/
/****************************************************************************
 * Includes
 ****************************************************************************/
#include "VSInterface.h"
#include "VSAudioCaptureSlot.h"
#include "Transcoder/VS_IppAudiCodec.h"
#include "Transcoder/VS_AudioReSampler.h"
/****************************************************************************
 * Static
 ****************************************************************************/
const char CAudioCaptureSlotExt::_funcSetFormat[]="SetFormat";
const char CAudioCaptureSlotExt::_funcPushData[]="PushData";

/**
 **************************************************************************
 ****************************************************************************/
CAudioCaptureSlot::CAudioCaptureSlot(const char *szSlotName,CVSInterface* pParentInterface,VS_MediaFormat &mf,CAudioCaptureList *pAudioCaptureList)
:CVSInterface(szSlotName,pParentInterface,NULL,true)
{
	m_pAudioCapture = 0;
	m_bValid = false;
}

/**
 **************************************************************************
 ****************************************************************************/
void CAudioCaptureSlot::_Init(VS_MediaFormat &mf,CAudioCaptureList *pAudioCaptureList){
	m_pAudioCapture = new VS_AudioCapture(this,pAudioCaptureList,&mf);
	m_bValid = true;
}

/**
 **************************************************************************
 ****************************************************************************/
void CAudioCaptureSlot::_Release(){
	m_bValid = false;
	if(m_pAudioCapture) delete m_pAudioCapture;	m_pAudioCapture = 0;
}

/**
 **************************************************************************
 * \param		buff	[out] pointer to data buffer
 * \param		size	[out] size of copied data
 * \return size of copied data
 ****************************************************************************/
int CAudioCaptureSlot::Capture(char* buff, int &size, bool use_audio){
	int res = m_pAudioCapture->Capture(buff,size,use_audio);
	if (res == -1) {
		m_pAudioCapture->ReInitDevice();
		size = 0;
		res = 0;
	}
	return res;
}

/**
 **************************************************************************
 ****************************************************************************/
HANDLE CAudioCaptureSlot::GetAudioEvent(){
	return m_pAudioCapture->GetCmpleteEvent();
}

/**
 **************************************************************************
 ****************************************************************************/
void CAudioCaptureSlot::Start(){
	m_pAudioCapture->Start();
}


/****************************************************************************
 * CAudioCaptureSlotExt
 ****************************************************************************/
/**
 **************************************************************************
 ****************************************************************************/
CAudioCaptureSlotExt::CAudioCaptureSlotExt(const char *szSlotName,CVSInterface* pParentInterface,VS_MediaFormat &mf,CAudioCaptureList *pAudioCaptureList)
:CAudioCaptureSlot(szSlotName,pParentInterface,mf,pAudioCaptureList)
{
	m_codec = 0;
	m_resampler = 0;
	m_pos=-1;
	m_data = m_size = 0;
	m_hEvent = 0;
	memset(m_Buffer, 0, sizeof(m_Buffer));
}

/**
 **************************************************************************
 ****************************************************************************/
void CAudioCaptureSlotExt::_Init(VS_MediaFormat &mf,CAudioCaptureList *pAudioCaptureList){
	m_resampler = new VS_AudioReSampler();
	m_hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	m_bValid = true;
}

/**
 **************************************************************************
 ****************************************************************************/
void CAudioCaptureSlotExt::_Release(){
	m_bValid = false;
	Release();
	if (m_resampler) delete m_resampler; m_resampler = 0;
	if(m_hEvent) CloseHandle(m_hEvent); m_hEvent = 0;
}

/**
 **************************************************************************
 ****************************************************************************/
int CAudioCaptureSlotExt::ProcessFunction(VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar){
	_variant_t* var=(_variant_t*)pVar;
	if(strncmp(pSection,VS_AudioCapture::_funcStart,strlen(VS_AudioCapture::_funcStart))==0) {
		switch(VS_OPERATION)
		{
		case RUN_COMMAND:
			Init(*(VS_MediaFormat *)(int)*var);
			return VS_INTERFACE_OK;
		}
	}
	else if(strncmp(pSection,VS_AudioCapture::_funcStop,strlen(VS_AudioCapture::_funcStop))==0) {
		switch(VS_OPERATION)
		{
		case RUN_COMMAND:
			Release();
			return VS_INTERFACE_OK;
		}
	}
	else if(strncmp(pSection,_funcSetFormat,sizeof(_funcSetFormat))==0){
		switch(VS_OPERATION)
		{
		case RUN_COMMAND:
			WAVEFORMATEX *pwf=(WAVEFORMATEX *)(int)*var;
			memcpy(&m_wf_in,pwf,sizeof(WAVEFORMATEX));
			return VS_INTERFACE_OK;
		}
	}
	else if(strncmp(pSection,_funcPushData,sizeof(_funcPushData))==0){
		switch(VS_OPERATION)
		{
		case RUN_COMMAND:
			if(var->vt==(VT_ARRAY|VT_VARIANT)){
				long l,u;
				SAFEARRAY *psa=var->parray;
				SafeArrayGetLBound(psa,1,&l);
				SafeArrayGetUBound(psa,1,&u);
				if(u-l==1){
					VARIANT vr,vr_h;
					SafeArrayGetElement(psa,&l,&vr);
					l++;
					SafeArrayGetElement(psa,&l,&vr_h);

					unsigned char *pBuff=(unsigned char *)int(vr_h.intVal);
					int iSize=int(vr.intVal);
					PrepareBuffer(pBuff,iSize);
					return VS_INTERFACE_OK;
				}
			}
		}
	}
	return VS_INTERFACE_NO_FUNCTION;
}

/**
 **************************************************************************
 ****************************************************************************/
void CAudioCaptureSlotExt::PrepareBuffer(unsigned char *pBuff,int iSize){
	if(m_pos<0)
		return;
	m_size=m_resampler->Process(pBuff,m_Buffer[m_pos],iSize,m_wf_in.nSamplesPerSec,m_wf.nSamplesPerSec);
	m_pos++;
	if(m_pos==BUFFER_MAX)
		m_pos=0;
	if(m_data<BUFFER_MAX)
		m_data++;
	SetEvent(m_hEvent);

}

/**
 **************************************************************************
 ****************************************************************************/
bool CAudioCaptureSlotExt::Init(VS_MediaFormat &mf){
	if (!mf.IsAudioValid()) return false;
	Release();

	m_wf.wFormatTag = WAVE_FORMAT_PCM;
	m_wf.nChannels = 1;
	m_wf.nSamplesPerSec = mf.dwAudioSampleRate;
	m_wf.wBitsPerSample = 16;
	m_wf.nBlockAlign = m_wf.nChannels*m_wf.wBitsPerSample / 8;
	m_wf.nAvgBytesPerSec = m_wf.nSamplesPerSec*m_wf.nBlockAlign;

	m_codec = VS_RetriveAudioCodec(mf.dwAudioCodecTag, true);
	if (!m_codec)
		return false;

	m_codec->Init(&m_wf);
	m_size=0x8000;//mf.dwAudioBufferLen;
	int i;
	for(i=0;i<BUFFER_MAX;i++){
		m_Buffer[i]=(unsigned char*)malloc(m_size);
	}
	m_pos=0;
	m_data=0;
	return true;
}


/**
 **************************************************************************
 ****************************************************************************/
void CAudioCaptureSlotExt::Release(){
	int i;
	ResetEvent(m_hEvent);
	m_pos=-1;
	m_data=0;
	for(i=0;i<BUFFER_MAX;i++){
		if(m_Buffer[i]){
			free(m_Buffer[i]);
			m_Buffer[i]=NULL;
		}
	}
	if (m_codec) delete m_codec; m_codec = 0;
}

/**
 **************************************************************************
 ****************************************************************************/
int CAudioCaptureSlotExt::Capture(char* buff, int &size, bool use_audio){
	size=0;
	if((m_data)&&(m_pos>=0)){
		int n=m_pos-m_data;
		if(n<0)
			n+=BUFFER_MAX;
		size = m_codec->Convert((BYTE*)m_Buffer[n], (BYTE*)buff, m_size);
		m_data--;
	}
	return 0;
}

/**
 **************************************************************************
 ****************************************************************************/
HANDLE CAudioCaptureSlotExt::GetAudioEvent(){
	return m_hEvent;
}
