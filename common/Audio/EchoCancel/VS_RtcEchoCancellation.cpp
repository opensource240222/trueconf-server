#include "VS_RtcEchoCancellation.h"
#include "webrtc/modules/audio_processing/aec/include/echo_cancellation.h"
#include "webrtc/modules/audio_processing/aecm/include/echo_control_mobile.h"
#include "../NoiseSuppression/VS_RtcNoiseSuppression.h"
#include "webrtc/common_audio/signal_processing/include/signal_processing_library.h"
#include <memory.h>

VS_RtcEchoCancellation::VS_RtcEchoCancellation()
{
	m_aec=0;
	m_fs=16000;
	m_scfs=16000;
	m_isInit=false;
	m_type = AEC_WEBRTC;

}

VS_RtcEchoCancellation::~VS_RtcEchoCancellation()
{
	Release();

}


void VS_RtcEchoCancellation::Init(int sampFreq)
{
	Release();
	int result=0;
	WebRtcAec_Create(&m_aec);
	if(sampFreq<16000)
		m_fs=8000;
	else if((sampFreq>=16000)&&(sampFreq<32000))
		m_fs=16000;
	else
		m_fs=32000;
	m_scfs=m_fs;
	result=WebRtcAec_Init(m_aec,m_fs,m_scfs);
	if(result<0)
		return;
	m_isInit=true;
}
void VS_RtcEchoCancellation::Release()
{
	if(m_aec) WebRtcAec_Free(m_aec); m_aec=0;
	memset(&m_st1,0,6*sizeof(int));
	memset(&m_st2,0,6*sizeof(int));
	memset(&m_st1_far,0,6*sizeof(int));
	memset(&m_st2_far,0,6*sizeof(int));
	memset(&m_st3,0,6*sizeof(int));
	memset(&m_st4,0,6*sizeof(int));
	memset(&m_LB,0,160*sizeof(short int));
	memset(&m_HB,0,160*sizeof(short int));
	memset(&m_LB_far,0,160*sizeof(short int));
	memset(&m_HB_far,0,160*sizeof(short int));
	m_isInit=false;
}

int VS_RtcEchoCancellation::AddFarEnd(short int* farend,int len)
{
	if(!m_isInit)
		return -1;

	if(m_fs<32000)
	{
		if(len%80==0)
		{
			int subbuffer_count=len/80;
			for(int i=0;i<subbuffer_count;i++)
			{
				WebRtcAec_BufferFarend(m_aec,farend+i*80,80);
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
				WebRtcSpl_AnalysisQMF(farend+320*i,&m_LB_far[0],&m_HB_far[0],&m_st1_far[0],&m_st2_far[0]);
				WebRtcAec_BufferFarend(m_aec,&m_LB_far[0],160);
			}

		}

	}
	return 0;
}

void VS_RtcEchoCancellation::Cancellate(short* far_end, short* near_end, short* out, int len)
{
	if(!m_isInit)
		return;
	int msInSndCardBuf=10;
	int skew=0;

	if(m_fs==8000)
	{
		if(len%80==0)
		{
			int subbuffer_count=len/80;
			for(int i=0;i<subbuffer_count;i++)
			{
				AddFarEnd(far_end+80*i,80);
				WebRtcAec_Process(m_aec,near_end+80*i, 0, out+80*i,0, 80,msInSndCardBuf,skew);
			}
		}
	}else
	{
		if(m_fs==16000)
		{
			if(len%160==0)
			{
				int subbuffer_count=len/160;
				for(int i=0;i<subbuffer_count;i++)
				{
					AddFarEnd(far_end+160*i,160);
					WebRtcAec_Process(m_aec,near_end+160*i, 0, out+160*i,0, 160,msInSndCardBuf,skew);
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
					WebRtcSpl_AnalysisQMF(near_end+320*i,&m_LB[0],&m_HB[0],&m_st1[0],&m_st2[0]);

					AddFarEnd(far_end+320*i,320);
					WebRtcAec_Process(m_aec,&m_LB[0], &m_HB[0], &m_LB[0], &m_HB[0], 160,msInSndCardBuf,skew);

					WebRtcSpl_SynthesisQMF(&m_LB[0],&m_HB[0],out+320*i,&m_st3[0],&m_st4[0]);
				}
			}
		}
	}

}


int VS_RtcEchoCancellation::SetConfig(int aggressiveness,bool skew_mode, bool metric_mode, bool delay_logging)
{
	if(!m_isInit)
		return -1;
	AecConfig cfg;
	cfg.nlpMode=aggressiveness;
	cfg.metricsMode=metric_mode;
	cfg.delay_logging=delay_logging;
	cfg.skewMode=skew_mode;
	WebRtcAec_set_config(m_aec, cfg);

	return 0;
}




///////ECHO MOBILE///////////////////////////////


VS_RtcEchoCancellationMobile::VS_RtcEchoCancellationMobile()
{
	m_aec=0;
	m_fs=16000;
	m_isInit=false;
	m_type=AEC_WEBRTCFAST;
}

VS_RtcEchoCancellationMobile::~VS_RtcEchoCancellationMobile()
{
	Release();
}

void VS_RtcEchoCancellationMobile::Init(int sampFreq)
{
	Release();
	m_fs=sampFreq;
	if((m_fs!=8000)&& (m_fs!=16000))
		return;
	WebRtcAecm_Create(&m_aec);
	WebRtcAecm_Init(m_aec,m_fs);
	m_isInit=true;
}
void VS_RtcEchoCancellationMobile::Release()
{
	if(m_aec) WebRtcAecm_Free(m_aec); m_aec=0;
	m_isInit=false;
}


int VS_RtcEchoCancellationMobile::AddFarEnd(short int* farend,int len)
{
	if(!m_isInit)
		return -1;

	if(len%80==0)
	{
		int subbuffer_count=len/80;
		for(int i=0;i<subbuffer_count;i++)
			WebRtcAecm_BufferFarend(m_aec,farend+i*80,80);
	}
	return 0;
}

void VS_RtcEchoCancellationMobile::Cancellate(short* far_end, short* near_end, short* out, int len)
{
	if(!m_isInit)
		return;

	int msInSndCardBuf=10;

	if(m_fs==8000)
	{
		if(len%80==0)
		{
			int subbuffer_count=len/80;
			for(int i=0;i<subbuffer_count;i++)
			{
				 AddFarEnd(far_end+80*i,80);
				 WebRtcAecm_Process(m_aec,near_end+80*i,0,out+80*i,80,msInSndCardBuf);
			}
		}
	}
	else
	{
		if(len%160==0)
		{
			int subbuffer_count=len/160;
			for(int i=0;i<subbuffer_count;i++)
			{
				 AddFarEnd(far_end+160*i,160);
				 WebRtcAecm_Process(m_aec,near_end+160*i,0, out+160*i, 160,msInSndCardBuf);
			}
		}
	}


}




int VS_RtcEchoCancellationMobile::SetConfig(int cngMode, int echoMode)
{
	if(!m_isInit)
		return -1;
	AecmConfig cfg;
	cfg.cngMode=cngMode;
	cfg.echoMode=echoMode;

	WebRtcAecm_set_config(m_aec, cfg);

	return 0;
}