#include "VS_RtcVoiceActivity.h"


VS_RtcVoiceActivity::VS_RtcVoiceActivity()
{
	m_isInit=false;
	m_vad=0;
	m_fs=16000;
}


VS_RtcVoiceActivity::~VS_RtcVoiceActivity()
{
	Release();
}


int VS_RtcVoiceActivity::Init(int fs, int mode)
{
	Release();
	WebRtcVad_Create(&m_vad);
    WebRtcVad_Init(m_vad);
	WebRtcVad_set_mode(m_vad, mode);
	if(fs<16000)
		m_fs=8000;
	if((fs>=16000)&&(fs<32000))
		m_fs=16000;
	if(fs>=32000)
		m_fs=32000;
	m_isInit=true;
	return 0;
}



int VS_RtcVoiceActivity::Release()
{
	if(m_vad) WebRtcVad_Free(m_vad); m_vad=0;
	m_isInit=false;
	return 0;
}


int VS_RtcVoiceActivity::SetMode(int mode)
{
	if(!m_isInit)
		return -1;
	WebRtcVad_set_mode(m_vad, mode);
	return 0;
}


int  VS_RtcVoiceActivity::Process(short int* data, int len)
{
	if(!m_isInit)
		return -1;
	double prob=0;

	if(m_fs==8000)
	{
		int subbufer_count=len/80;
		for(int i=0;i<subbufer_count;i++)
			prob=prob+WebRtcVad_Process(m_vad, m_fs, data+i*80, 80);
		prob=prob/subbufer_count;
	}
	if(m_fs==16000)
	{
		int subbufer_count=len/160;
		for(int i=0;i<subbufer_count;i++)
			prob=prob+WebRtcVad_Process(m_vad, m_fs, data+i*160, 160);
		prob=prob/subbufer_count;
	}

	if(m_fs==32000)
	{
		int subbufer_count=len/320;
		for(int i=0;i<subbufer_count;i++)
			prob=prob+WebRtcVad_Process(m_vad, m_fs, data+i*320, 320);
		prob=prob/subbufer_count;
	}
   if(prob>0.5)
	   return 1;
   else
	   return 0;
}