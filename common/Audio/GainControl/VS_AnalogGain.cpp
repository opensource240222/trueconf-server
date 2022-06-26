
#include "VS_AnalogGain.h"
#include <windows.h>
#include "../../VSClient/VS_VolControl.h"
#include "Transcoder/VSAudioVad.h"
#include "../VoiceActivity/SpecialDspFunctions.h"
#include "../../std/cpplib/VS_RegistryKey.h"
#include "std-generic/cpplib/utf8.h"
#include <math.h>



VS_AnalogAgc::VS_AnalogAgc()
{
	m_fs=16000;

	m_TimeAnShort=0;
	m_VoiceSampleCount=0;
	m_RmsStatVoice=0;
	m_RmsStatNoise=0;
	m_TimeHoldIncrease=0;
	m_TimeSlowIncrease=0;
	m_NumSampCalibr=0;
	m_vad=0;
	m_buff=0;
	m_IsVista=false;
	m_IsInit=false;
	m_CalibrationFlag=true;
	m_RmsStableFlag=false;
	m_SlowIncreaseFlag=false;
	m_BlockIncreaseByNoise=false;
	m_GlobalIncrease=0;
	m_VoiceFrameSeq=0;
}

VS_AnalogAgc::~VS_AnalogAgc()
{
	SaveToRegistry();
	Release();

}

int VS_AnalogAgc::Init(int fs, wchar_t* device_name, int devID, VS_VolControlBase * vol)
{
	Release();
	device_name[199]='\0';
	wcscpy(m_CurrenDeviceName,device_name);

	const int RmsDequeSize=500;
	m_fs=fs;
	OSVERSIONINFOEX osvi;
	bool	bOsVersionInfoEx;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
	{
		osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		GetVersionEx ( (OSVERSIONINFO *) &osvi);
	}
	if(osvi.dwMajorVersion>=6)
		m_IsVista=true;

	m_vol = vol;

	RestoreFromRegistry();

	m_vad=new VSAudioVAD;
	m_vad->Init(fs,16);
	m_vad->SetMode(ENABLE_VAD);
	m_buff= new short [32000];
	m_RmsStatVoice= new VS_FastSimpleStat();
	m_RmsStatVoice->Init(300);

	m_RmsStatNoise= new VS_FastSimpleStat();
	m_RmsStatNoise->Init(300);

	m_IsInit=true;


	if (m_vol && m_vol->IsValid())
	{
		float currBoost;

		m_vol->GetMicBoost(&currBoost);

		if (currBoost > 20.0f)
			m_vol->SetMicBoost(20.0f);
	}

return 0;
}



int VS_AnalogAgc::Release()
{


	m_IsVista=false;
	m_CalibrationFlag=true;
	memset(&m_hist,0,sizeof(int)*100);

	if(m_buff) delete [] m_buff; m_buff=0;
	if(m_RmsStatVoice) delete m_RmsStatVoice; m_RmsStatVoice=0;
	if(m_RmsStatNoise) delete m_RmsStatNoise; m_RmsStatNoise=0;
	if(m_vad) delete m_vad; m_vad=0;


	m_TimeAnShort=0;
	m_VoiceSampleCount=0;
	return 0;
}


int  VS_AnalogAgc::Analyse(short int* data, int len)
{
	if(!m_IsInit) return 0;
	const int frame_size=160;
	double RmsVoice=0;
	double RmsNoise=0;
	int curr_time=timeGetTime();
	int dt=curr_time-m_LastTime;

	memcpy(m_buff,data, len*sizeof(short));


	m_TimeAnShort=m_TimeAnShort+dt;
	m_TimeAnRms=m_TimeAnRms+dt;
	m_TimeSlowIncrease=m_TimeSlowIncrease+dt;
	m_TimeHoldIncrease=m_TimeHoldIncrease+dt;
	m_TimePollSliders=m_TimePollSliders+dt;

	if (m_TimeAnRms > TIME_ANALYSYS_RMS)
	{
		if (m_vol && m_vol->IsValid())
		{
			float currBoost;

			m_vol->GetMicBoost(&currBoost);

			if (currBoost > 20.0f)
			{
				m_vol->SetMicBoost(20.0f);
			}
		}
	}

	if (m_SlowIncreaseFlag)
	{
		if(m_TimeSlowIncrease>=TIME_SLOW_INCREASE)
		{
			m_TimeSlowIncrease=0;
			if(m_GlobalIncrease==0)
				m_SlowIncreaseFlag=false;
			if(m_GlobalIncrease>0)
			{
				if(m_GlobalIncrease>1)
				{
					ChangedB(1);
					m_GlobalIncrease=m_GlobalIncrease-1;
				}
				else
				{
					ChangedB(m_GlobalIncrease);
					m_GlobalIncrease=0;

				}
			}
		}
	}

	for (int i=0;i<len;i++)
	{
		if(m_buff[i]<0)
		{
			if(m_buff[i]==-32768)
				m_buff[i]=32767;
			else
				m_buff[i]=-m_buff[i];
		}
	}


	if(len%160==0)
	{
		int subbuf_count=len/frame_size;
		for(int i=0;i<subbuf_count;i++)
		{
			RmsVoice=0;
			RmsNoise=0;
			if(m_vad->IsVad(m_buff+frame_size*i,frame_size*2))
			{
				for(int k=i*frame_size;k<(i+1)*frame_size;k++)
				{
					m_hist[(m_buff[k]*25)>>13]++;
					RmsVoice=RmsVoice+m_buff[k]*m_buff[k];
				}
				m_VoiceSampleCount=m_VoiceSampleCount+frame_size;
				m_VoiceFrameSeq++;
				if(m_VoiceFrameSeq>5)
				{
					m_NumSampCalibr=m_NumSampCalibr+frame_size;
					m_RmsStatVoice->AddValue(RmsVoice/(double)frame_size);

				}

			}
			else
			{
				for(int k=i*frame_size;k<(i+1)*frame_size;k++)
				{
					RmsNoise=RmsNoise+m_buff[k]*m_buff[k];
				}
				m_RmsStatNoise->AddValue(RmsNoise/(double)frame_size);
				m_VoiceFrameSeq=0;
			}
		}
	}

	if((m_RmsStatNoise->GetMovingAverage()<100)&& m_RmsStatNoise->IsComplete()&&!m_BlockIncreaseByNoise)
	{
		PollingVolumeSliders();
		ChangedB(1);
	}

	if((m_TimeAnRms>TIME_ANALYSYS_RMS)&&(m_TimeHoldIncrease>TIME_HOLD_INCREASE)&&(!m_CalibrationFlag))
	{
		m_TimeAnRms=0;
		double RMSV=sqrt(m_RmsStatVoice->GetMovingAverage());
		double RMSN=sqrt(m_RmsStatNoise->GetMovingAverage());
		double SNR=0;
		if(RMSN!=0)
			SNR=RMSV/RMSN;

		if(!m_CalibrationFlag)
		{
			if(RMSV<1000)
				m_RmsStableFlag=false;

			if((!m_RmsStableFlag)&&(SNR>4)&&(m_VoiceFrameSeq>30))
			{
				PollingVolumeSliders();
				ChangedB(0.4);
			}
			if(RMSV>3000)
				m_RmsStableFlag=true;
		}
	}
	if(m_TimeAnShort>TIME_SHORT)
	{
		m_TimeAnShort=0;
		double prob=0;
		for(int i=0;i<12;i++)
			prob=prob+m_hist[99-i];
		if(m_VoiceSampleCount)
			prob=prob/(double) m_VoiceSampleCount;
		m_TimeAnShort=0;
		m_VoiceSampleCount=0;
		memset(&m_hist[0],0,sizeof(int)*100);

		if(prob>0.005)
		{
			m_SlowIncreaseFlag=false;
			m_CalibrationFlag=false;
			m_GlobalIncrease=0;
			m_BlockIncreaseByNoise=true;

			ChangedB(-1.2);
			m_TimeHoldIncrease=0;
		}
	}

	if(m_CalibrationFlag)
	{
		if(m_NumSampCalibr>NUM_SAMPLES_CALIBRATION)
		{
			m_CalibrationFlag=false;
			m_NumSampCalibr=0;
			double rms=sqrt(m_RmsStatVoice->GetMovingAverage());
			double increasedB=20*log10(2500/(double)rms);
			if(increasedB>4)
			{
				m_GlobalIncrease=increasedB;
				m_SlowIncreaseFlag=true;
			}
			else
			{
				ChangedB(increasedB);
			}
			m_CalibrationFlag=false;
		}
	}

	if(m_TimePollSliders>TIME_POLLING_SLIDERS)
		PollingVolumeSliders();


    m_LastTime=curr_time;
	return 0;
}


int  VS_AnalogAgc::PollingVolumeSliders()
{

	float  CurrVolumePos=0;
	float  CurrBoostPos=0;
	m_vol->GetMicVolumedB(&CurrVolumePos);
	m_vol->GetMicBoost(&CurrBoostPos);
	m_TimePollSliders=0;

	if((CurrVolumePos!=m_LastVolumePos)||(CurrBoostPos!=m_LastBoostPos))
	{
		m_CalibrationFlag=true;
		m_BlockIncreaseByNoise=false;
		m_NumSampCalibr=0;
		m_LastBoostPos=CurrBoostPos;
		m_LastVolumePos=CurrVolumePos;
	}

	return 0;
}

int  VS_AnalogAgc::ChangedB(double dB)
{

	m_vol->ChangedB(dB);
	m_vol->GetMicVolumedB(&m_LastVolumePos);
	m_vol->GetMicBoost(&m_LastBoostPos);

	return 0;

}


int  VS_AnalogAgc::SaveToRegistry()
{
	float CurrMicVol=-1;
	float CurrMicBoost=-1;
	if(m_vol)  m_vol->GetMicVolume(&CurrMicVol);
	if(m_vol) m_vol->GetMicBoost(&CurrMicBoost);
	CurrMicVol=CurrMicVol*100;
	int tmp_vol=(int) (CurrMicVol+0.5);
	int tmp_boost=(int)(CurrMicBoost+0.5)+ 1000; //offset to compensate negative values
	std::string dev_name = "Client\\AudioCaptureSlot\\";
	dev_name += vs::WideCharToUTF8Convert(m_CurrenDeviceName);
	VS_RegistryKey vol_key(true,dev_name,false,true);
	vol_key.SetValue(&tmp_vol, 4, VS_REG_INTEGER_VT, "AgcVol");
	vol_key.SetValue(&tmp_boost, 4, VS_REG_INTEGER_VT, "AgcBoost");

	return 0;
}


int  VS_AnalogAgc::RestoreFromRegistry()
{
	int tmp_vol=-1;
	int tmp_boost=-1;
	std::string dev_name = "Client\\AudioCaptureSlot\\";
	dev_name += vs::WideCharToUTF8Convert(m_CurrenDeviceName);
	VS_RegistryKey vol_key(true,dev_name,false,true);
	vol_key.GetValue(&tmp_vol, 4, VS_REG_INTEGER_VT, "AgcVol");
	vol_key.GetValue(&tmp_boost, 4, VS_REG_INTEGER_VT, "AgcBoost");
	if((tmp_vol>=0)&&(m_vol))   m_vol->SetMicVolume(tmp_vol/(float)100);
	if((tmp_boost>=0)&&(m_vol)) m_vol->SetMicBoost(tmp_boost-1000);

	return 0;
}




