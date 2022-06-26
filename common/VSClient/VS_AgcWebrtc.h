#pragma once

#include <windows.h>
#include <vector>
#include "webrtc/modules/audio_processing/agc/include/gain_control.h"

class VS_AudioMixerVolume;
class VSAudioVAD;
class VS_VolControl;
struct DeviceVolumeSettings;




class VS_AgcWebrtc
{
private:
	VSAudioVAD *vad;
	VS_AudioMixerVolume *micxp;
	VS_VolControl* mic;
	vector <DeviceVolumeSettings> dev_set;
	WebRtcAgc_config_t cfg;
	DWORD BoostControlID;
	DWORD BoostControlType;
	DWORD BoostChannels;
	HMIXEROBJ MIXER;
	unsigned char  saturationWarning;
	OSVERSIONINFOEX osvi;
	BOOL bOsVersionInfoEx;


	void* inst;
	wchar_t  curr_dev_name[MAX_PATH];
	int sample_rate;
	int samples_count2;
	int samples_rms;
	int current_step;
	int buffer_length;
	int buffer_num;
	int hist[100];
	int  hist2[100];
	int vad_count;
	int frsize2;
	int samples_noise;
	int	webrtc_init_ok;
	int frames_count;
	int set_vad;
	int mode;
	int step_slow_increase;
	int samples_count;
	int hist_int_count;
	int* state1;
	int* state2;
	int* state3;
	int* state4;
	short int* buffer;
	short int* buffer2;
	short int* Low_Band;
	short int* High_Band;
	float sys_vol;
	float sys_boost;
////////////////////////////////
	double T_analysis_increase;
	float T_slow_increase;
	float T_analysis;
	float T_analisis_polling;
	float T_analysis_rms;
	double global_increase;
	double dBs;
	double t_an_short;
	double time_step_incr;
	float z1;
	float rms_frames_gain;
	float t_an_rms;
	float mindB;
	float maxdB;
	float stepdB;
	float volmindB;
	float volmaxdB;
	float volstepdB;
	float currentdB;
	float an_vol_lev;
	float system_time;
	float vad_frames_gain;
	float vol_position;
	float boost_position;
	float *RMS_arr;
	float *RMS_arr_noise;
	float RMS;
	float time;
	float time_rms;
	float step;

	float T_analysis_noise;
	float RMS_noise;

///////////////////////////

	bool slow_increase_flag;
	bool calibration_flag;
	bool BoostFeature;
	bool rms_stable_flag;
	bool release_flag;
	bool vad_tresh;

////////////////////// METHODS///////////////////

	short int Diff(short int x0, short int x1);
	int Analysis(short int * data, int buffer_num);
	int XP_MixerGetBoost(HMIXEROBJ mixer, DWORD id, DWORD type, DWORD val, DWORD channels);
	int arr_shift(float *RMS_arr,int num,float add);
	int flush_rms(float *RMS_arr,int num);
	double calc_over(double p);
	double IncreasedB( double increasedB);
	double DecreasedB( double decreasedB);
	double DecreaseXP(double value);
	double IncreaseXP(double value);
	float mean_rms(float *RMS_arr,int num);
	bool XP_MixerSearchBoost(HMIXEROBJ mixer, MIXERLINE *pml, MIXERCONTROL *pmc, int opt);
	bool XP_MixerSetBoost(HMIXEROBJ mixer, DWORD id, DWORD type, DWORD val, DWORD channels);
	bool NoiseEnvCheck(short int* data, int size);
	void XP_MixerBoostInit(int GlobalMode);
	int Polling();

///////////////////////////////////////////////////////////////////////////////////////////////////

public:
	static int far_end_vad;
	~VS_AgcWebrtc();
	VS_AgcWebrtc();

	int Init(int AgcMode,int SampleRate,int BufferLen,int dev,wchar_t * szname);
	int Process(short int* m_AudioTemp);
	int SetParam(int TargetDb,int CompressGain,bool limiterEnable);
	int GetAgcMode(){ return mode;}
	int SaveSettings();
	int RestoreSettings();
	int SaveInternalSettings();
	int RestoreInternalSettings();


};
