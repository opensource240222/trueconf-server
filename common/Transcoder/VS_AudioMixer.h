#pragma once

enum VS_AudioMixerMode
{
        AMM_NONE,
        AMM_SUMM,
        AMM_SUMMKOEF,
        AMM_SATURATE,
        AMM_FRAMENORM
};

class VS_AudioMixer
{
private:
	VS_AudioMixerMode mode;
	bool InitOK;
	int sign(int a)
		{
			if(a>0)
				return 1;
			if(a<0)
				return -1;
			else
				return 0;
		}


public:
	VS_AudioMixer();
        bool Init(VS_AudioMixerMode MODE);
        bool  Mix(short** in,  long  num,  short*  out, unsigned long size);
		bool  Mix(short** in,  long  num_input,  short** out,  long num_ouput, unsigned long size);

};