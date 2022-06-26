#include "VS_RtcDigitalAgc.h"
#include "modules/audio_processing/agc/legacy/gain_control.h"
#include "common_audio/signal_processing/include/signal_processing_library.h"
#include <memory.h>

VS_RtcDigitalAgc::VS_RtcDigitalAgc()
{
	m_agc=0;
	m_warn=0;
	m_isInit=false;
	m_buf_len=160;
}

VS_RtcDigitalAgc::~VS_RtcDigitalAgc()
{
 Release();

}
int VS_RtcDigitalAgc::Init(int sample_rate, int agc_mode, int targetDb, int comprGain, bool limEnable)
{
	Release();
	int webrtc_init=0;
	m_fs=sample_rate;
	m_agc = WebRtcAgc_Create();
	if(sample_rate<=22050)
		webrtc_init=WebRtcAgc_Init(m_agc,0,100,agc_mode,16000);
	else
		webrtc_init=WebRtcAgc_Init(m_agc,0,100,agc_mode,32000);
	WebRtcAgcConfig  cfg;
	cfg.targetLevelDbfs=targetDb;
	cfg.compressionGaindB=comprGain;
	cfg.limiterEnable=limEnable;
	WebRtcAgc_set_config(m_agc,cfg);
	if(!webrtc_init)
		m_isInit=true;
	return 0;
}

int VS_RtcDigitalAgc::Release()
{
	if(m_agc) WebRtcAgc_Free(m_agc);
	m_warn=0;
	memset(&m_st1,0,6*sizeof(int));
	memset(&m_st2,0,6*sizeof(int));
	memset(&m_st3,0,6*sizeof(int));
	memset(&m_st4,0,6*sizeof(int));
	memset(&m_Low_Band,0,160*sizeof(short int));
	memset(&m_High_Band,0,160*sizeof(short int));
	m_isInit=false;
	return 0;
}


int VS_RtcDigitalAgc::Process(short int* data, int len)
{
	if(!m_isInit)
		return -1;

	int micIn=0;
	int micOut=0;
	if(m_fs<=22050)
	{
		if(len%160==0)
		{
			int subbuffer_count=len/160;
			for(int i=0;i<subbuffer_count;i++)
			{

			}
		}
	}
	else
	{
		if(len%320==0)
		{
			int subbuffer_count=len/320;
			for(int i=0;i<subbuffer_count;i++)
			{

			}
		}
	}
return 0;
}