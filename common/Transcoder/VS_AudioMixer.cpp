#include <string.h>
#include <math.h>
#include "VS_AudioMixer.h"

VS_AudioMixer::VS_AudioMixer()
	: mode(AMM_NONE)
	, InitOK(false)
{
}

bool VS_AudioMixer::Init(VS_AudioMixerMode MODE)
{
	if(MODE<5)
	{
		mode=MODE;
		InitOK=true;
		return true;
	}
	else
	{
		InitOK=false;
		return false;
	}
}

bool  VS_AudioMixer:: Mix(short**  in,  long  num,  short*  out, unsigned long size)
{
	if(InitOK==false)
		return false;
	if(mode==AMM_NONE)
		return true;
	if(mode==AMM_SUMM)
	{
		for(int i=0;i<(int)size;i++)
		{
			int sum=0;
			for(int j=0;j<num;j++)
				sum=sum+in[j][i];
			if(sum>32767)
				out[i]=32767;
			else
				if(sum<-32768)
					out[i]=-32768;
				else
					out[i]=sum;

		}
		return true;
	}
	if(mode==AMM_SUMMKOEF)
	{
		int sum=0;
		short koeff=(short)(sqrt((sqrt(1.0/(float)num)*0.99)) *(1 << 14));
		for(int i=0;i<(int)size;i++)
		{
			sum=0;
			for(int j=0;j<num;j++)
				sum=sum+in[j][i];
			sum=(sum*koeff)>>14;
			if(sum>32767)
				out[i]=32767;
			else
				if(sum<-32768)
					out[i]=-32768;
				else
					out[i]=sum;
		}
		return true;
	}
	if(mode==AMM_SATURATE)
	{
		int  threshold=16000;
		int sum_abs=1;
		int sum;
		for(int i=0;i<(int)size;i++)
		{
			sum=0;
			for(int j=0;j<num;j++)
				sum=sum+in[j][i];
			if(sum<0)
				sum_abs=~sum+1;
			else
				sum_abs=sum;
			if(sum_abs>threshold)
			{
				out[i]=(short)(32767-(32767-threshold)*exp(-(sum_abs-threshold)/(float)(32767-threshold)));
				out[i]=sign(sum)*out[i];
			}
			else
				out[i]=sum;

		}
		return true;
	}
	if(mode==AMM_FRAMENORM)
	{
		int max=0;
		int sum;
		int *out_buf= new  int[size];
		for(int i=0;i<(int)size;i++)
		{
			sum=0;
			for(int j=0;j<num;j++)
				sum=sum+in[j][i];
			out_buf[i]=sum;
			if(sum<0)
			sum=~sum+1;
			if(sum>max)
				max=sum;
		}

		if(max>32767)
			for(int i=0;i<(int)size;i++)
			{
				out[i]=out_buf[i]*32766/max;
			}

		else
			for(int i=0;i<(int)size;i++)
			{
				out[i]=out_buf[i];
			}



			delete [] out_buf;
			return true;
	}

	return false;
}

bool VS_AudioMixer::Mix(short** in, long num_input, short** out, long num_ouput, unsigned long size)
{
	if (!InitOK) {
		return false;
	}
	if (mode == AMM_NONE) {
		return true;
	}
	if (num_input == 0 || num_ouput == 0) {
		return false;
	}
	if (mode != AMM_SUMMKOEF) {
		return false;
	}
	if (num_ouput > 1) {
		short k_norm[2];
		k_norm[0] = (short)( sqrt(sqrt(1.0 / (float)num_input) * 0.99) * (1 << 14) ); // all
		k_norm[1] = (short)( sqrt(sqrt(1.0 / (float)(num_input - 1)) * 0.99) * (1 << 14) ); // minus 1
		for(int i = 0; i < (int)size; i++) {
			int sum = 0;
			for (int j = 0; j < num_ouput; j++) {
				if (in[j] == nullptr) {
					continue;
				}
				sum += in[j][i];
			}
			int v;
			for (int j = 0; j <= num_ouput; j++) {
				if (in[j] == nullptr || j == num_ouput) {
					v = (sum * k_norm[0]) >> 14;
				}
				else {
					v = ((sum - in[j][i]) * k_norm[1]) >> 14;
				}
				if (v > 32767) {
					out[j][i] = 32767;
				}
				else if (v < -32768) {
					out[j][i] = -32768;
				}
				else {
					out[j][i] = v;
				}
			}
		}
	} else {
		memset(out[0], 0, size * sizeof(short));
		memcpy(out[1], in[0], size * sizeof(short));
	}
	return true;
}