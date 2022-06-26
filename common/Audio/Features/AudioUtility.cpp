#include "AudioUtility.h"
#include "common_audio/signal_processing/include/signal_processing_library.h"



VS_BandSplitterCombiner::VS_BandSplitterCombiner()
{
	Refresh();
}

VS_BandSplitterCombiner::~VS_BandSplitterCombiner()
{
}

int VS_BandSplitterCombiner::Refresh()
{
	memset(&m_st1,0,sizeof(int)*6);
	memset(&m_st2,0,sizeof(int)*6);
	memset(&m_st3,0,sizeof(int)*6);
	memset(&m_st4,0,sizeof(int)*6);
	return 0;
}

int VS_BandSplitterCombiner::Split(short int* InData,int InLen, short int* LowBand, short int* HighBand)
{
	int OutLen = 0;
	if(InLen % 320 == 0)
	{
		int subbuffer_count = InLen / 320;
		for(int i = 0;i < subbuffer_count;i++)
		{
			WebRtcSpl_AnalysisQMF(InData+320*i, 320, LowBand+160*i, HighBand+160*i, &m_st1[0], &m_st2[0]);
		}
		OutLen = InLen / 2;
	}

	return OutLen;
}


int VS_BandSplitterCombiner::Combine(short int* LowBand, short int* HighBand, int InLen, short int* OutData)
{
	int OutLen=0;
	if(InLen % 160==0)
	{
		int subbuffer_count = InLen / 160;
		for(int i = 0; i<subbuffer_count; i++)
		{
			WebRtcSpl_SynthesisQMF(LowBand+160*i, HighBand+160*i, 160, OutData+320*i, &m_st3[0], &m_st4[0]);
		}
		OutLen = InLen * 2;
	}
	return OutLen;
}