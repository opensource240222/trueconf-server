#pragma once

#include <Windows.h>
#include <mmsystem.h>

#include <vector>

struct IAudioVolumeLevel;
struct IAudioEndpointVolume;
struct IPart;
struct cAudioVolumeCallback;


class VS_VolControlBase
{
protected:
	bool volumeOK;

public:
	static VS_VolControlBase * Factory();

	virtual ~VS_VolControlBase() {};

	virtual int Init(wchar_t *szname, int devID, bool isRender)=0;
	virtual void Release()=0;

	virtual int GetMicVolume( float *vol)=0;
	virtual int GetMicVolumedB(float *volumedB)=0;
	virtual int GetMicVolumeRangedB(float *mindB,float *maxdB,float *stepdB)=0;      //mic
	virtual int SetMicVolume(float volume)=0;
	virtual int SetMicVolumedB(float volumedB)=0;

	virtual int ChangedB(double dB)=0;

	virtual int GetMicBoost(float *boost)=0;
	virtual int GetBoostParam(float *minDb,float *maxDb, float *stepDb)=0;           //boost
	virtual int SetMicBoost(float Boost_Db)=0;

	virtual int SetAgcToDeviceVolume(float vol) = 0; // AgcToDevice

	bool IsValid();
};

class VS_VolumeCaptureControlWin
{
public:

	VS_VolumeCaptureControlWin();
	~VS_VolumeCaptureControlWin();

	HRESULT CollectInf(IPart *pPart, std::vector <IPart*> & list, int recursiveLimit);
	bool InitInternal(const wchar_t * name);
	void ReleaseInternal();
	bool GetBoost(float *boost);
	void SetBoost(float boost);
	void SetClientToDeviceVolume(float vol);
	void SetAgcToDeviceVolume(float vol);
	bool GetDeviceInfo(float *lvl, float *agcLvl);
	bool IsValid();

private:

	GUID m_guidMyCtx;
	IAudioVolumeLevel *m_audioBoost;
	IAudioEndpointVolume *m_audioVolume;
	cAudioVolumeCallback* m_AudioVolumeCallback;
	float m_minBoostDb;
	float m_maxBoostDb;
	float m_stepBoostDb;
	float m_halBoostDb;
	float m_minMicDb;
	float m_maxMicDb;
	float m_stepMicDb;
	float m_minMicScalar;
	bool m_Init = false;
};

class VS_VolumeRendererControlWin
{
public:

	VS_VolumeRendererControlWin();
	~VS_VolumeRendererControlWin();

	bool InitInternal(const wchar_t* name);
	void ReleaseInternal();
	void SetClientToDeviceVolume(float vol);
	bool GetDeviceInfo(float *lvl);
	bool IsValid();

private:

	GUID m_guidMyCtx;
	IAudioEndpointVolume *m_audioVolume;
	cAudioVolumeCallback* m_AudioVolumeCallback;
	bool m_Init = false;
};

class VS_VolControlNew : public VS_VolControlBase
{
private:

	VS_VolumeRendererControlWin* m_Render = nullptr;
	VS_VolumeCaptureControlWin* m_Capture = nullptr;

public:

	VS_VolControlNew();
	~VS_VolControlNew();

	int Init(wchar_t *szname, int devID, bool isRender);
	void Release();

	int GetMicBoost(float *boost);
	int SetMicBoost(float Boost_Db);

	int GetBoostParam(float *minDb, float *maxDb, float *stepDb);
	int GetMicVolume(float *vol);


	int GetMicVolumeRangedB(float *mindB, float *maxdB, float *stepdB);
	int GetMicVolumedB(float *volumedB);


	int SetMicVolume(float volume); // ClientToDevice
	int SetMicVolumedB(float volumedB);
	int SetAgcToDeviceVolume(float volumedB); // AgcToDevice

	int ChangedB(double voldB);


};




class VS_AudioMixerVolume;

class VS_VolControlXP:public VS_VolControlBase
{
private:
	VS_AudioMixerVolume*    m_vol;
	bool				    m_isInit;
	bool					m_BoostFeature;
	DWORD				    m_BoostControlID;
	DWORD				    m_BoostControlType;
	DWORD					m_BoostChannels;
	HMIXEROBJ				m_mixer;

	double					m_HIGH_DB;
	double					m_LOW_DB;
	double					m_STEP_DB;

	float					m_BoostLowBarrier;

	int XP_MixerBoostInit(wchar_t *devName);
	int XP_MixerSearchBoost(HMIXEROBJ mixer, MIXERLINE *pml, MIXERCONTROL *pmc, int opt);
	int XP_MixerGetBoost(HMIXEROBJ mixer, DWORD id, DWORD type, DWORD val, DWORD channels);
	int XP_MixerSetBoost(HMIXEROBJ mixer, DWORD id, DWORD type, DWORD val, DWORD channels);

public:

	 VS_VolControlXP();
	~VS_VolControlXP();

	int Init(wchar_t *szname, int devID, bool isRender);
	void Release();

	int GetBoostParam(float *minDb,float *maxDb, float *stepDb);

	int GetMicVolume( float *vol);
	int GetMicBoost(float *boost);

	int GetMicVolumeRangedB(float *mindB,float *maxdB,float *stepdB);
	int GetMicVolumedB(float *volumedB);
	int SetMicBoost(float Boost_Db);
	int SetMicVolume(float volume);

	int SetMicVolumedB(float volumedB);

	int SetAgcToDeviceVolume(float vol); // AgcToDevice

	int ChangedB(double voldB);

};

