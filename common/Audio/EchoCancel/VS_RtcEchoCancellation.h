#pragma once

#include "SpeexEchoCancel.h"

class VS_RtcNoiseSuppression;

class VS_RtcEchoCancellation:public VS_EchoCancelBase
{
	private:
	void*				m_aec;
	bool				m_isInit;
	int					m_fs;
	int					m_scfs;
	int					m_st1[6];
	int					m_st2[6];
	int					m_st1_far[6];
	int					m_st2_far[6];
	int					m_st3[6];
	int					m_st4[6];
	short int			m_LB[160];
	short int			m_HB[160];
	short int			m_LB_far[160];
	short int			m_HB_far[160];
	eTypeAEC			m_type;
	int					AddFarEnd(short int* farend,int len);
	int					SetConfig(int aggressiveness,bool skew_mode=false, bool metric_mode=false, bool delay_logging=false);
public:
	VS_RtcEchoCancellation();
	~VS_RtcEchoCancellation();

	void Init(int fs);
	void Init(int frequency, int num_mic, int num_spk){}

	void Release();


	void Cancellate(short* far_end, short* near_end, short* out, int samples);

	eTypeAEC GetType(){ return m_type;}
};



class VS_RtcEchoCancellationMobile:public VS_EchoCancelBase
{
	private:
	void*					m_aec;
	bool					m_isInit;
	int						m_fs;
	eTypeAEC				m_type;
	int						SetConfig(int cngMode=1, int echoMode=3);
	int						AddFarEnd(short int* farend,int len);
public:
	VS_RtcEchoCancellationMobile();
	~VS_RtcEchoCancellationMobile();

	void Init(int sampFreq);
	void Init(int frequency, int num_mic, int num_spk){}

	void Release();


	void Cancellate(short* far_end, short* near_end, short* out, int samples);


	eTypeAEC GetType(){ return m_type;}

};
