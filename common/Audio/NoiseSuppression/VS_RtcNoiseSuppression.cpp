#include "VS_RtcNoiseSuppression.h"
#include "common_audio/signal_processing/include/signal_processing_library.h"
#include <memory.h>

VS_RtcNoiseSuppression::VS_RtcNoiseSuppression()
{
	m_ns=0;
	m_fs=16000;
	m_isInit=false;
}

VS_RtcNoiseSuppression::~VS_RtcNoiseSuppression()
{
	Release();
}

int VS_RtcNoiseSuppression::Init(int fs)
{
	Release();
	m_ns = WebRtcNs_Create();
	int result=0;
	if(fs<16000)
		m_fs=8000;
	else if((fs>=16000)&&(fs<32000))
		m_fs=16000;
	else
		m_fs=32000;
	result=WebRtcNs_Init(m_ns, m_fs);
	if(result<0)
		return -1;
	m_isInit=true;
	SetPolicy(1);
	return 0;
}

int VS_RtcNoiseSuppression::Release()
{
	if(m_ns) WebRtcNs_Free(m_ns); m_ns=0;
	memset(&m_st1,0,6*sizeof(int));
	memset(&m_st2,0,6*sizeof(int));
	memset(&m_st3,0,6*sizeof(int));
	memset(&m_st4,0,6*sizeof(int));
	memset(&m_LB,0,160*sizeof(short int));
	memset(&m_HB,0,160*sizeof(short int));
	m_isInit=false;
	return 0;
}

int  VS_RtcNoiseSuppression::SetPolicy(int policy)
{
	if(!m_isInit)
		return -1;
	WebRtcNs_set_policy(m_ns, policy);
	return 0;
}


int VS_RtcNoiseSuppression::Process(short int *data, int len)
{
	if(!m_isInit)
		return -1;

	if(m_fs==8000)
	{
		if(len%80==0)
		{
			int subbuffer_count=len/80;
			for(int i=0;i<subbuffer_count;i++)
			{
			}
		}
	} else if(m_fs==16000)
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

// if you already have 2 splitted band you might use this function
int VS_RtcNoiseSuppression::Process(short int *dataLB, short int *dataHB,int len)
{
	if((!m_isInit)||(m_fs!=32000))
		return -1;
	if(len%160==0)
	{
		int subbuffer_count=len/160;
		for(int i=0;i<subbuffer_count;i++)
		{
		}
	}
	return 0;
}