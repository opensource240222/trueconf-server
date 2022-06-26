#pragma once

#include "modules/audio_processing/ns/noise_suppression.h"

class VS_RtcNoiseSuppression
{
private:
	NsHandle* m_ns;
	bool  m_isInit;
	int m_fs;
	int  m_st1[6];
	int  m_st2[6];
	int  m_st3[6];
	int  m_st4[6];
	short int  m_LB[160];
	short int  m_HB[160];
public:
	VS_RtcNoiseSuppression();
   ~VS_RtcNoiseSuppression();

	int Init(int fs);
	int Release();

	int Process(short int* data, int len);  //splitting Low and High bands inside
	int Process(short int *dataLB, short int *dataHB, int len);// you should  slpit bands manually before (only for 32kHz)

	int SetPolicy(int policy);

};