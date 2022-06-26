#pragma once

#include <deque>

class VS_VolControlBase;
class VSAudioVAD;
class VS_FastSimpleStat;

class VS_AnalogAgc
{
private:
	VSAudioVAD*					m_vad;
	VS_VolControlBase*			m_vol;
	VS_FastSimpleStat*          m_RmsStatVoice;
	VS_FastSimpleStat*          m_RmsStatNoise;
	wchar_t						m_CurrenDeviceName[200];
	short int*					m_buff;
	int							m_hist[100];
	int							m_fs;

	bool						m_IsVista;
	bool						m_IsInit;
	bool						m_CalibrationFlag;
	bool						m_RmsStableFlag;
	bool						m_SlowIncreaseFlag;
	bool						m_BlockIncreaseByNoise;


	int							m_LastTime;
	int							m_VoiceSampleCount;
	int							m_TimeAnShort;
	int							m_TimePollSliders;
	int							m_TimeAnRms;
	int							m_TimeHoldIncrease;
	int							m_TimeSlowIncrease;

	int							m_NumSampCalibr;
	int							m_VoiceFrameSeq;



	static const int			TIME_SHORT=100;
	static const int			TIME_POLLING_SLIDERS=1000;
	static const int			TIME_ANALYSYS_RMS=1500;
	static const int			TIME_SLOW_INCREASE=250;
	static const int			TIME_HOLD_INCREASE=20000;
	static const int			NUM_SAMPLES_CALIBRATION=100000;

	float						m_LastVolumePos;
	float 						m_LastBoostPos;
	double						m_GlobalIncrease;



	int							PollingVolumeSliders();
	int							ChangedB(double dB);
	int							SaveToRegistry();
	int							RestoreFromRegistry();



public:
	VS_AnalogAgc();
	~VS_AnalogAgc();

	int Init(int fs, wchar_t* device_name, int devID, VS_VolControlBase * vol);
	int Release();

	int Analyse(short int* data, int len);



};