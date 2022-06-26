#include "VS_AgcWebrtc.h"
#include "VS_VolControl.h"
#include <math.h>
#include <windows.h>
#include "../Audio/VoiceActivity/VS_Mixer.h"
#include "../VSClient/VSAudioNew.h"
#include "webrtc/common_audio/signal_processing/include/signal_processing_library.h"
#include <Mmsystem.h>
#include "../Audio/VoiceActivity/VSAudioVad.h"

int VS_AgcWebrtc::far_end_vad=0;

struct DeviceVolumeSettings
{
	float sys_vol;
	float sys_boost;
	float agc_vol;
	float agc_boost;
	wchar_t name[MAX_PATH];
};

VS_AgcWebrtc::VS_AgcWebrtc()
{
	vad=new VSAudioVAD;
	micxp=new VS_AudioMixerVolume;
	mic= new VS_VolControl;
	webrtc_init_ok=1;
	time=3;
	time_rms=10;
	t_an_short=0.1;
	t_an_rms=1;
	dBs=3;
	time_step_incr=1/dBs;
	step_slow_increase=1;
	system_time=0;
	inst=0;
	mindB=0;
	maxdB=0;
	stepdB=0;
	volmindB=0;
	volmaxdB=0;
	volstepdB=0;
	samples_noise=0;
	RMS_noise=0;
	T_analysis_noise=0;
	release_flag=false;
	RMS_arr=new float[(unsigned int)(time_rms/t_an_rms)];
	RMS_arr_noise=new float[(unsigned int)(time_rms/t_an_rms)];
	for(int i=0;i<time_rms/t_an_rms;i++)
	{
		RMS_arr[i]=0;
		RMS_arr_noise[i]=0;
	}
	T_slow_increase=0;
	slow_increase_flag=false;
	calibration_flag=true;
	global_increase=0;
	Low_Band=new short int[160];
	High_Band=new short int[160];
	state1=new int [6];
	state2=new int [6];
	state3=new int [6];
	state4=new int [6];
	memset(hist,0,100*sizeof(int));
	memset(hist2,0,100*sizeof(int));
	vad_count=0;
	hist_int_count=100;
	step=328;
	set_vad = 1;
	current_step=0;
	samples_count2=0;
	samples_rms=0;
	T_analysis_increase=0;
	samples_count=0;
	T_analysis=0;
	T_analisis_polling=0;
	T_analysis_rms=0;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
	{
		osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		GetVersionEx ( (OSVERSIONINFO *) &osvi);

	}




}
VS_AgcWebrtc::~VS_AgcWebrtc()
{
	delete [] Low_Band;
	delete [] High_Band;
	delete [] state1;
	delete [] state2;
	delete [] state3;
	delete [] state4;
	delete [] RMS_arr;
	delete [] RMS_arr_noise;
	vad->Release();
	delete  vad;
	delete  micxp;
	delete mic;
	if(sample_rate!=0)
	{
		if(webrtc_init_ok==0)
			WebRtcAgc_Free(inst);
	}
	if(BoostFeature==true)
		mixerClose((HMIXER) MIXER);
	if(release_flag==true)
	{
		delete [] buffer;
		delete [] buffer2;
		release_flag=false;
	}



}

int VS_AgcWebrtc::SaveSettings()
{
	sys_vol=0;
	sys_boost=0;

	//mic->GetMicVolumedB(&sys_vol);
	//mic->GetMicBoost(&sys_boost);
	for(int i=0;i<(int)dev_set.size();i++)
		if( wcscmp(curr_dev_name,dev_set[i].name)==0)
		{
			if(osvi.dwMajorVersion>=6)
			{
				mic->GetMicVolumedB(&dev_set[i].sys_vol);
				mic->GetMicBoost(&dev_set[i].sys_boost);
			}
			else
			{
				dev_set[i].sys_vol=(float)micxp->GetVolume();
				dev_set[i].sys_boost=(float)XP_MixerGetBoost(MIXER,BoostControlID,BoostControlType,1,BoostChannels);
			}

		}
		return 0;
}

int VS_AgcWebrtc::RestoreSettings()
{
	//mic->SetMicVolumedB(sys_vol);
	//mic->SetMicBoost(sys_boost);
	for(int i=0;i<(int)dev_set.size();i++)
		if( wcscmp(curr_dev_name,dev_set[i].name)==0)
		{
			if(osvi.dwMajorVersion>=6)
			{
				mic->SetMicVolumedB(dev_set[i].sys_vol);
				mic->SetMicBoost(dev_set[i].sys_boost);
			}
			else
			{
				micxp->SetVolume((DWORD)dev_set[i].sys_vol);
				XP_MixerSetBoost(MIXER,BoostControlID,BoostControlType,(DWORD)dev_set[i].sys_boost,BoostChannels);
			}

		}

		return 0;
}


int VS_AgcWebrtc::SaveInternalSettings()
{

	for(int i=0;i<(int)dev_set.size();i++)
		if( wcscmp(curr_dev_name,dev_set[i].name)==0)
		{
			if(osvi.dwMajorVersion>=6)
			{
				mic->GetMicVolumedB(&dev_set[i].agc_vol);
				mic->GetMicBoost(&dev_set[i].agc_boost);
			}
			else
			{
				dev_set[i].agc_vol=(float)micxp->GetVolume();
				dev_set[i].agc_boost=(float)XP_MixerGetBoost(MIXER,BoostControlID,BoostControlType,1,BoostChannels);
			}
		}


		return 0;

}

int VS_AgcWebrtc::RestoreInternalSettings()
{


	//mic->SetMicVolumedB(vol_position);
	// mic->SetMicBoost(boost_position);
	for(int i=0;i<(int)dev_set.size();i++)
		if( wcscmp(curr_dev_name,dev_set[i].name)==0)
		{
			if(osvi.dwMajorVersion>=6)
			{
				mic->SetMicVolumedB(dev_set[i].agc_vol);
				mic->SetMicBoost(dev_set[i].agc_boost);
			}
			else
			{
				micxp->SetVolume((DWORD)dev_set[i].agc_vol);
				XP_MixerSetBoost(MIXER,BoostControlID,BoostControlType,(DWORD)dev_set[i].agc_boost,BoostChannels);

			}
		}

		return 0;

}


/*//////////////////////////////////////////////////




////////////////////////////////////////////////*/
int VS_AgcWebrtc::Init(int AgcMode,int SampleRate,int BufferLen,int dev, wchar_t * szname)
{
	szname[MAX_PATH-1]='\0';
	if(release_flag==true)
	{
		delete [] buffer;
		delete [] buffer2;
		release_flag=false;
	}
	vad->Init(SampleRate,16);
	vad->SetMode(ENABLE_VAD);
	sample_rate=SampleRate;
	buffer_length=BufferLen;
	buffer_num=buffer_length/2;
	mode=AgcMode;
	vad_tresh=true;
	if(SampleRate==0)
		return 0;
	vad_frames_gain=0;
	rms_frames_gain=0;
	RMS=0;
	rms_stable_flag=false;

	buffer=new short int [buffer_num];
	buffer2=new short int [buffer_num];
	release_flag=true;

	if((sample_rate==8000)||(sample_rate==16000)||(sample_rate==11025)||(sample_rate==22050))
		frsize2=160;
	else
		frsize2=320;

	z1=frsize2/(float)sample_rate;


	if(osvi.dwMajorVersion<=5)
	{
		BoostFeature=false;
		XP_MixerBoostInit(VS_MIXER_BOOST_MIC);
		boost_position=(float)XP_MixerGetBoost(MIXER,BoostControlID,BoostControlType,1,BoostChannels);
		micxp->Init(dev, VS_AudioMixerVolume::MTYPE_MIC);

		/*micxp->SetVolume(32768);
		if((BoostFeature==true)&&(boost_position==0))
		XP_MixerSetBoost(MIXER,BoostControlID,BoostControlType,1,BoostChannels);*/


	}
	else
	{

		mic->Init(szname, dev, false);
		//mic->SetMicVolume(0.5);
		mic->GetBoostParam(&mindB,&maxdB,&stepdB);
		mic->GetMicVolumeRangedB(&volmindB,&volmaxdB,&volstepdB);
		mic->GetMicVolumedB(&vol_position);
		mic->GetMicBoost(&boost_position);
		mic->GetMicVolume(&an_vol_lev);

		/*	if(boost_position==mindB)
		mic->SetMicBoost(maxdB);*/

	}


	wcscpy_s(curr_dev_name,MAX_PATH,szname);
	bool isNewDev=true;
	for(int i=0;i<(int)dev_set.size();i++)
	{
		if(wcscmp(szname,dev_set[i].name)==0)      //// переделать, возможно через map
			isNewDev=false;
	}
	if(isNewDev==true)
	{
		DeviceVolumeSettings buf_dev;
		if(osvi.dwMajorVersion>=6)
		{
			mic->GetMicVolumedB(&buf_dev.agc_vol);
			mic->GetMicBoost(&buf_dev.agc_boost);
		}
		else
		{
			buf_dev.agc_vol=(float)micxp->GetVolume();
			buf_dev.agc_boost=boost_position;
		}
		wcscpy_s(buf_dev.name,MAX_PATH,szname);
		dev_set.push_back(buf_dev);
	}
	memset(&hist,0,sizeof(int)*100);
	memset(&hist2,0,sizeof(int)*100);
	flush_rms(RMS_arr,(int)(time_rms/t_an_rms));
	T_analysis_increase=0;
	T_analysis=0;
	T_analisis_polling=0;
	T_analysis_rms=0;
	T_slow_increase=0;
	system_time=0;
	samples_count2=0;
	samples_rms=0;
	global_increase=0;
	slow_increase_flag=false;
	calibration_flag=true;
	current_step=0;
	webrtc_init_ok=WebRtcAgc_Create(&inst);
	if(webrtc_init_ok!=0)
	{
		return 1;
	}
	if((SampleRate==8000)||(SampleRate==16000)||(SampleRate==11025)||(SampleRate==22050))
		webrtc_init_ok=WebRtcAgc_Init(inst,0,100,AgcMode,16000);

	if((SampleRate==32000)||(SampleRate==44100)||(SampleRate==48000))
		webrtc_init_ok=WebRtcAgc_Init(inst,0,100,AgcMode,32000);

	if(webrtc_init_ok!=0)
	{
		return 1;
	}
	return 0;
}


int VS_AgcWebrtc::Process(short int* m_AudioTemp)
{
	if(sample_rate==0)
		return 0;

	//if(far_end_vad==0)
	Analysis(m_AudioTemp,buffer_num);		// работает аналоговое АРУ


	if((mode<2)||(mode>3)||(webrtc_init_ok!=0))
		return 0;
	int micIn=0;
	int micOut=0;
	if((sample_rate==8000)||(sample_rate==16000)||(sample_rate==11025)||(sample_rate==22050))
	{

		if(buffer_num%160==0)
		{
			int subbuffer_count=buffer_num/160;
			for(int i=0;i<subbuffer_count;i++)
			{
				WebRtcAgc_AddMic(inst,m_AudioTemp+160*i,0,160);
				WebRtcAgc_Process(inst,m_AudioTemp+160*i,0,160,m_AudioTemp+160*i,0,micIn,&micOut,0,&saturationWarning);

			}
		}

	}
	if((sample_rate==32000)||(sample_rate==44100)||(sample_rate==48000))
	{
		if(buffer_num%320==0)
		{
			int subbuffer_count=buffer_num/320;
			for(int i=0;i<subbuffer_count;i++)

			{
				WebRtcSpl_AnalysisQMF(m_AudioTemp+320*i,Low_Band,High_Band,state1,state2);

				WebRtcAgc_AddMic(inst,Low_Band,High_Band,160);

				WebRtcAgc_Process(inst,Low_Band,High_Band,160,Low_Band,High_Band,micIn,&micOut,0,&saturationWarning);

				WebRtcSpl_SynthesisQMF(Low_Band,High_Band,m_AudioTemp+320*i,state3,state4);


			}

		}
	}


	return 0;
}
int VS_AgcWebrtc::SetParam(int TargetDb,int CompressGain,bool limiterEnable)
{
	cfg.targetLevelDbfs=TargetDb;
	cfg.compressionGaindB=CompressGain;
	cfg.limiterEnable=limiterEnable;
	WebRtcAgc_set_config(inst,cfg);
	return 0;
}

int VS_AgcWebrtc::Analysis(short int * data,int buffer_num)
{

	if(slow_increase_flag==true)
	{
		T_slow_increase=T_slow_increase+ buffer_num/(float)sample_rate;

		if(T_slow_increase>=time_step_incr)
		{
			T_slow_increase=0;
			if(global_increase>0)
			{
				if(global_increase>=step_slow_increase)
				{
					if(osvi.dwMajorVersion>=6)
						IncreasedB(step_slow_increase);
					else
						IncreaseXP(step_slow_increase);

					global_increase=global_increase-step_slow_increase;
					if(global_increase==0)
						slow_increase_flag=false;
				}
				else
				{
					if(osvi.dwMajorVersion>=6)
						IncreasedB(global_increase);
					else
						IncreaseXP(global_increase);

					global_increase=0;
					slow_increase_flag=false;

				}
			}

		}
	}


	memcpy(buffer,data, buffer_length);
	memcpy(buffer2,data, buffer_length);

	for (int i=0;i<buffer_num;i++)
	{
		if(buffer[i]<0)
		{
			if(buffer[i]==-32768)
				buffer[i]=32767;
			else
				buffer[i]=-buffer[i];

		}

	}



	if(buffer_num%frsize2==0)
	{
		int subbuffer_count=buffer_num/frsize2;

		for(int i=0;i<subbuffer_count;i++)

		{
			if(vad->IsVad(buffer2+frsize2*i,frsize2*2)==1)
			{
				vad_count++;
				samples_count=samples_count+frsize2;
				samples_count2=samples_count2+frsize2;
				samples_rms=samples_rms+frsize2;


				for(int k=i*frsize2;k<(i+1)*frsize2;k++)
				{
					hist[(buffer[k]*25)>>13]++;
					hist2[(buffer[k]*25)>>13]++;
					RMS=RMS+buffer[k]*buffer[k];

				}
				T_analysis=T_analysis+z1;
				T_analysis_increase=T_analysis_increase+z1;
				T_analisis_polling=T_analisis_polling+z1;
				T_analysis_rms=T_analysis_rms+z1;
				system_time=system_time+z1;


			}
			else
			{
				vad_frames_gain++;
				T_analysis_noise=T_analysis_noise+z1;
				float sum_buf=0;
				for(int k=i*frsize2;k<(i+1)*frsize2;k++)
				{
					sum_buf=sum_buf+buffer[k]*buffer[k];
					samples_noise++;
				}
				rms_frames_gain=rms_frames_gain+sum_buf;
				RMS_noise=RMS_noise+sum_buf;
				if(vad_frames_gain==5)
				{

					rms_frames_gain=5*sqrt((float)rms_frames_gain/(frsize2*vad_frames_gain));

					if(rms_frames_gain<70)
					{
						Polling();
						if(vad_tresh==true)
						{
							if(osvi.dwMajorVersion>=6)
								IncreasedB(0.7);
							else
								IncreaseXP(0.7);
						}
					}
					vad_frames_gain=0;
					rms_frames_gain=0;

				}


			}
		}
	}


	if(T_analisis_polling>1)
	{
		Polling();
	}


	if(T_analysis_noise>1)
	{
		T_analysis_noise=0;
		RMS_noise=sqrt(RMS_noise/(float)samples_noise);
		arr_shift(RMS_arr_noise,(int)(time_rms/t_an_rms),RMS_noise);
		samples_noise=0;
		RMS_noise=0;

	}

	if(T_analysis_rms>=t_an_rms)
	{
		T_analysis_rms=0;

		RMS=sqrt(RMS/(float)samples_rms);
		arr_shift(RMS_arr,(int)(time_rms/t_an_rms),RMS);
		RMS=0;
		samples_rms=0;
		//////////////////////////////
		float SNR=0;
		if(mean_rms(RMS_arr_noise,(int)(time_rms/t_an_rms))!=0)
			SNR=mean_rms(RMS_arr,(int)(time_rms/t_an_rms))/mean_rms(RMS_arr_noise,int(time_rms/t_an_rms));

		/////////////////////////////
		if(calibration_flag==false)
		{

			float rms=4*mean_rms(RMS_arr,(int)(time_rms/t_an_rms));
			if((rms<7000)&&(system_time>time_rms))
			{
				if((rms_stable_flag==true)&&(rms<4000)) //7000
					rms_stable_flag=false;

				if((rms_stable_flag==false)&&(SNR>3))
				{
					if(osvi.dwMajorVersion>=6)
					{
						IncreasedB(0.8);

					}
					else
						IncreaseXP(0.8);
				}

			}
			if((rms>=7000)&&(system_time>time_rms))   //10000
				rms_stable_flag=true;

		}

	}




	if(T_analysis>t_an_short)
	{

		if(system_time>100)
			system_time=30;
		T_analysis=0;

		int sum=0;

		vad_count=0;
		double probability=0;
		for(int i=0;i<12;i++)
			probability=probability+hist[hist_int_count-1-i];

		if(samples_count!=0)
			probability=probability/(double)samples_count;

		if(probability>0.005)
		{
			for(int i=0;i<100;i++)
				hist2[i]=0;
			T_analysis_increase=0;
			system_time=0;
			samples_count2=0;
			samples_rms=0;
			global_increase=0;
			slow_increase_flag=false;
			T_slow_increase=0;
			calibration_flag=false;
			vad_tresh=false;

			if(osvi.dwMajorVersion>=6)
				DecreasedB(1.2);
			else
				DecreaseXP(1.2);
			flush_rms(RMS_arr,(int)(time_rms/t_an_rms));


		}


		memset(&hist,0, sizeof(int)*hist_int_count);
		samples_count=0;

	}


	if(T_analysis_increase>=time)
	{

		float prob=0;
		float max_amp=0;
		float z=1/(float)samples_count2;

		for(int i=hist_int_count-1;i>=0;i--)
		{
			prob=prob+hist2[i]*z;
			if(prob>=0.01)
			{
				max_amp=step*(i+1);
				break;
			}

		}

		if((max_amp<7500)&&(calibration_flag==true))
		{

			double increasedB;
			if(max_amp>2*step)

			{
				increasedB=20*log10(12000/(double)max_amp);

			}
			else
			{
				float rms=mean_rms(RMS_arr,(int)(time_rms/t_an_rms));/// уточнить
				increasedB=20*log10(3000/(double)rms);
			}



			system_time=0;
			flush_rms(RMS_arr,(int)(time_rms/t_an_rms));
			if(increasedB>4)
			{
				slow_increase_flag=true;
				global_increase=increasedB;
			}
			else
			{
				if(osvi.dwMajorVersion>=6)
					IncreasedB(increasedB);
				else
					IncreaseXP(increasedB);

			}


		}
		else
		{
			if(calibration_flag==true)
			{
				system_time=0;
				flush_rms(RMS_arr,(int)(time_rms/t_an_rms));
			}
		}

		if(calibration_flag==true)
			calibration_flag=false;

		memset(&hist2,0, sizeof(int)*hist_int_count);

		samples_count2=0;
		T_analysis_increase=0;
	}


	return 0;
}


double VS_AgcWebrtc::IncreasedB( double increasedB)
{

	float voldB=0;
	mic->GetMicVolumedB(&voldB);
	float incrdB=volmaxdB-voldB;
	float decrdB=voldB-volmindB;
	if(volmaxdB-voldB>=increasedB)
	{
		mic->SetMicVolumedB(voldB+(float)increasedB);

	}
	else
	{
		if(stepdB!=0)
		{
			float currentdB;
			float boost_num=0;
			float voldBchange=0;
			mic->GetMicBoost(&currentdB);
			int n_steps_aval=(int)((maxdB-currentdB)/stepdB);
			for(int i=1;i<=n_steps_aval;i++)
			{
				if((increasedB>=i*stepdB-decrdB)&&(increasedB<=i*stepdB+incrdB))
				{
					boost_num=i*stepdB;
					voldBchange=(float)(increasedB-i*stepdB);
					mic->SetMicVolumedB(voldB+voldBchange);
					mic->SetMicBoost(currentdB+boost_num);
					break;
				}

			}
			if(boost_num==0)
			{
				if(increasedB>=n_steps_aval*stepdB+incrdB)
				{
					if(!((incrdB==0)&&(currentdB==maxdB)))
					{
						mic->SetMicVolumedB(volmaxdB);
						mic->SetMicBoost(currentdB+n_steps_aval*stepdB);
						global_increase=0;
						slow_increase_flag=false;
					}
				}
				else
				{
					if(!((incrdB==0)&&(currentdB==maxdB)))
					{
						mic->SetMicVolumedB(volmaxdB);
						global_increase=0;
						slow_increase_flag=false;
					}
				}
			}
		}
		else
		{
			if(incrdB!=0)
			{
				mic->SetMicVolumedB(volmaxdB);
				global_increase=0;
				slow_increase_flag=false;
			}

		}
	}

	mic->GetMicVolumedB(&vol_position);
	mic->GetMicBoost(&boost_position);
	return 0;
}


double VS_AgcWebrtc::DecreasedB( double decreasedB)
{
	float voldB=0;
	mic->GetMicVolumedB(&voldB);
	float incrdB=volmaxdB-voldB;
	float decrdB=voldB-volmindB;
	if(voldB-volmindB>=decreasedB)
	{
		mic->SetMicVolumedB(voldB-(float)decreasedB);
	}

	else
	{
		if(stepdB!=0)
		{
			float currentdB;
			float boost_num=0;
			float voldBchange=0;
			mic->GetMicBoost(&currentdB);
			int n_steps_aval=(int)((currentdB-mindB)/stepdB);
			for(int i=1;i<=n_steps_aval;i++)
			{
				if((i*stepdB-decreasedB)<=incrdB)                              ///внес изменения
				{
					boost_num=i*stepdB;
					voldBchange=(float)(decreasedB-i*stepdB);
					mic->SetMicVolumedB(voldB-voldBchange);
					mic->SetMicBoost(currentdB-boost_num);
					break;
				}

			}
			if(boost_num==0)
				mic->SetMicVolumedB(volmindB);
		}

	}

	mic->GetMicVolumedB(&vol_position);
	mic->GetMicBoost(&boost_position);
	return 0;
}

double VS_AgcWebrtc::DecreaseXP(double value)
{
	const int boost_xp_value=20;

	float decreasedB=(float)value;
	double currvol=micxp->GetVolume();
	double incrdB= 20*log10(65535/(float)currvol);
	double decrdB= 20*log10(currvol/(float)512);
	if(decreasedB<decrdB)
	{
		micxp->SetVolume((DWORD)(currvol/pow(10,decreasedB/(float)20)));
	}
	else
	{
		if(XP_MixerGetBoost(MIXER,BoostControlID,BoostControlType,1,BoostChannels)==1)
		{
			if(BoostFeature==true)
			{
				XP_MixerSetBoost(MIXER,BoostControlID,BoostControlType,0,BoostChannels);
				float residual_bost=decreasedB-boost_xp_value;
				micxp->SetVolume((DWORD)(currvol/pow(10,residual_bost/(float)20)));
			}
			else
				micxp->SetVolume(512);
		}
		else
			micxp->SetVolume(512);


	}

	vol_position=(float)micxp->GetVolume();
	boost_position=(float)XP_MixerGetBoost(MIXER,BoostControlID,BoostControlType,1,BoostChannels);
	return 0;
}

double VS_AgcWebrtc::IncreaseXP(double value)
{
	const int boost_xp_value=20;
	double increasedB=value;
	double currvol=micxp->GetVolume();
	if(currvol<512)
	{
		micxp->SetVolume(512);
		currvol=512;

	}

	double incrdB= -20*log10(currvol/(float)65535);
	double decrdB= 20*log10(currvol/(float)512);
	value=pow(10,value/(float)20);
	double newvol=(int)(currvol*value);
	if(increasedB<incrdB)
	{
		micxp->SetVolume((DWORD)(currvol*pow(10,increasedB/(float)20)));
	}
	else
	{
		if(XP_MixerGetBoost(MIXER,BoostControlID,BoostControlType,1,BoostChannels)==0)
		{
			if((increasedB>=boost_xp_value-decrdB)&&(increasedB<=boost_xp_value+incrdB)&&(BoostFeature==true))
			{
				XP_MixerSetBoost(MIXER,BoostControlID,BoostControlType,1,BoostChannels);
				double residual_bost=increasedB-boost_xp_value;
				micxp->SetVolume((DWORD)(currvol*pow(10,residual_bost/(float)20)));
			}
			else
				micxp->SetVolume(65535);
		}
		else
			micxp->SetVolume(65535);


	}

	vol_position=(float)micxp->GetVolume();
	boost_position=(float)XP_MixerGetBoost(MIXER,BoostControlID,BoostControlType,1,BoostChannels);

	return 0;
}


bool VS_AgcWebrtc::NoiseEnvCheck(short int *data,int size)
{
	float diff_energy=0;
	float noise_energy=0;
	float mean=0;
	float std=0;
	short int buf;
	for(int i=0;i<size-1;i++)
	{
		buf=Diff(data[i],data[i+1]);

		diff_energy=diff_energy+buf*buf;
		noise_energy=noise_energy+ data[i]*data[i];
		mean=mean+fabs((float)buf);
	}
	mean=mean/(float)(size-1);
	float ratio=noise_energy/diff_energy;
	if((ratio>=400)/*||(mean<80)*/)
		return true;
	else
		return false;
}

short int VS_AgcWebrtc::Diff(short int x0, short int x1)
{
	return (x1-x0);
}


int VS_AgcWebrtc::arr_shift(float *RMS_arr,int num,float add)
{
	for(int i=0;i<num-1;i++)
	{
		RMS_arr[i]=RMS_arr[i+1];
	}
	RMS_arr[num-1]=add;

	return 0;
}

float VS_AgcWebrtc::mean_rms(float *RMS_arr,int num)
{
	float buf=0;
	int k=0;
	for(int i=0;i<num;i++)
	{
		if(RMS_arr[i]!=0)
		{
			k++;
			buf=buf+RMS_arr[i];
		}

	}
	//buf=buf/(float)num;
	if(k!=0)
		buf=buf/(float)k;
	return buf;

}


int VS_AgcWebrtc::flush_rms(float *RMS_arr,int num)

{
	//memset(RMS_arr,0, sizeof(float)*num);
	return 0;
}

int VS_AgcWebrtc::Polling()
{

	T_analisis_polling=0;
	float cur_vol_pos=0;
	float cur_boost_pos=0;
	if(osvi.dwMajorVersion>=6)
	{
		mic->GetMicVolumedB(&cur_vol_pos);
		mic->GetMicBoost(&cur_boost_pos);
	}
	else
	{
		cur_vol_pos=(float)micxp->GetVolume();
		cur_boost_pos=(float)XP_MixerGetBoost(MIXER,BoostControlID,BoostControlType,1,BoostChannels);
	}


	if((cur_vol_pos!=vol_position )||(cur_boost_pos!=boost_position))
	{
		vol_position=cur_vol_pos;
		boost_position=cur_boost_pos;

		memset(&hist2,0, sizeof(int)*hist_int_count);
		T_analysis_increase=0;
		T_analysis_rms=0;
		system_time=0;
		flush_rms(RMS_arr,(int)(time_rms/t_an_rms));
		samples_count2=0;
		samples_rms=0;
		global_increase=0;
		slow_increase_flag=false;
		T_slow_increase=0;
		calibration_flag=true;
		vad_tresh=true;

	}

	return 0;
}



int VS_AgcWebrtc::XP_MixerGetBoost(HMIXEROBJ mixer, DWORD id, DWORD type, DWORD val, DWORD channels)
{
  if(BoostFeature==false)
		return -1;
	MMRESULT mmres = -1;
	MIXERCONTROLDETAILS mcd;
	mcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mcd.dwControlID = id;
	mcd.cChannels = channels;
	mcd.cMultipleItems = 0;

	MIXERCONTROLDETAILS_BOOLEAN *control = new MIXERCONTROLDETAILS_BOOLEAN[channels];
	mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
	mcd.paDetails = control;
	mmres = mixerGetControlDetails(mixer, &mcd, MIXER_GETCONTROLDETAILSF_VALUE);
    LONG buf_val=control[0].fValue;
	delete [] control;
	return buf_val;
}

bool VS_AgcWebrtc::XP_MixerSetBoost(HMIXEROBJ mixer, DWORD id, DWORD type, DWORD val, DWORD channels)
{
	if(BoostFeature==false)
		return false;
	MMRESULT mmres = -1;
	MIXERCONTROLDETAILS mcd;
	mcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mcd.dwControlID = id;
	mcd.cChannels = channels;
	mcd.cMultipleItems = 0;
	DWORD i;

	MIXERCONTROLDETAILS_BOOLEAN *control = new MIXERCONTROLDETAILS_BOOLEAN[channels];
	mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
	mcd.paDetails = control;
	for (i =0; i< channels; i++)
		control[i].fValue = val;
	mmres = mixerSetControlDetails(mixer, &mcd, MIXER_SETCONTROLDETAILSF_VALUE);
    delete [] control;
	return true;
}


bool VS_AgcWebrtc::XP_MixerSearchBoost(HMIXEROBJ mixer, MIXERLINE *pml, MIXERCONTROL *pmc, int opt)
{
	bool ret = true;
	if (pml->cControls==0) return false;
	// check uniform state
	DWORD channels = (pmc->fdwControl&MIXERCONTROL_CONTROLF_UNIFORM) ? 1 : pml->cChannels;

	if ((opt&VS_MIXER_BOOST_MIC)||(opt&VS_MIXER_UNBOOST_MIC))													// - boost mic
		if (pml->dwComponentType == MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE ||	// exactly Mic
			pml->dwComponentType == MIXERLINE_COMPONENTTYPE_SRC_ANALOG)			// possible Mic
			if (pmc->dwControlType == MIXERCONTROL_CONTROLTYPE_ONOFF ||			// some switch
				pmc->dwControlType == MIXERCONTROL_CONTROLTYPE_LOUDNESS) {		// real boost
					VS_SimpleStr name(pmc->szName);
					_strlwr(name);
					if (strstr(name, "boost") ||strstr(name, "усил")||strstr(name, "Усил"))
					{
						MIXER=mixer;
						BoostControlID=pmc->dwControlID;
						BoostControlType=pmc->dwControlType;
						BoostChannels=channels;
						BoostFeature=true;
					}
			}

			return true;
}


void VS_AgcWebrtc::XP_MixerBoostInit(int GlobalMode)
{

	UINT numdev = mixerGetNumDevs();
	MIXERCAPS mcaps;
	MMRESULT mmres;
	for (UINT i = 0; i< numdev; i++) {
		mixerGetDevCaps(i, &mcaps, sizeof(MIXERCAPS));

		HMIXER mixer;
		mmres = mixerOpen(&mixer, i, 0, 0, MIXER_OBJECTF_HMIXER);
		if (mmres== MMSYSERR_NOERROR)
		{
			for (DWORD j = 0; j< mcaps.cDestinations; j++) {
				MIXERLINE mline;
				mline.cbStruct = sizeof(MIXERLINE);
				mline.dwDestination = j;
				mmres = mixerGetLineInfo((HMIXEROBJ)mixer, &mline, MIXER_GETLINEINFOF_DESTINATION);
				if (mmres==MMSYSERR_NOERROR)
				{
					DWORD ConnNum = mline.cConnections;
					for (DWORD m = 0; m< (ConnNum+1); m++) {
						mline.dwSource = m-1;

						mmres = mixerGetLineInfo((HMIXEROBJ)mixer, &mline, m==0? MIXER_GETLINEINFOF_DESTINATION : MIXER_GETLINEINFOF_SOURCE);
						if (mmres==MMSYSERR_NOERROR)
						{
							if (mline.cControls!=0) {
								MIXERLINECONTROLS mlc;
								MIXERCONTROL * pmcs = new MIXERCONTROL[mline.cControls];
								mlc.cbStruct = sizeof(MIXERLINECONTROLS);
								mlc.dwLineID = mline.dwLineID;
								mlc.cControls = mline.cControls;
								mlc.cbmxctrl = sizeof(MIXERCONTROL);
								mlc.pamxctrl = pmcs;
								mmres = mixerGetLineControls((HMIXEROBJ)mixer, &mlc, MIXER_GETLINECONTROLSF_ALL);
								if (mmres== MMSYSERR_NOERROR)
								{
									for (DWORD k = 0; k<mline.cControls; k++) {
										MIXERCONTROL * pp = &pmcs[k];
										XP_MixerSearchBoost((HMIXEROBJ)mixer, &mline, pp, GlobalMode);
									}
								}
								delete[] pmcs;
							}
						}
					}
				}
			}
		}
	}
}
