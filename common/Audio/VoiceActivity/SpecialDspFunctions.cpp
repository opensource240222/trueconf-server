#include "SpecialDspFunctions.h"
#include <math.h>
#include "ipps90legacy.h"
#include "ippm90legacy.h"
#include <stdlib.h>

const double pi=3.14159265359;



double Log2( double n )
{
	return log( n ) / log((double) 2 );
}

/*
*
*
*/

VoiceChanger::VoiceChanger()
{
	isInit=false;
	framelen=320;
	sa=80;
	ss=160;
	Kmax=200;
	overlap_len=framelen-ss;
	tem_outbuf=0;
	overlap_buf=0;
	window=0;
	ipp_buf=0;
	ipp_corr=0;
	rxx_buf=0;
	tem_inbuf=0;
}

VoiceChanger::~VoiceChanger()
{
	Release();
}
double VoiceChanger::Norma(short int* buf, int len)
{
	double norm=0;
	for(int i=0;i<len;i++)
		norm=norm+buf[i]*buf[i];
	norm=sqrt(norm);
	return norm;

}
double VoiceChanger::xcorr(short int* buf1, short* buf2,int len)
{
	double corr=0;
	for(int i=0;i<len;i++)
		corr=corr+buf1[i]*buf2[i];
	return corr;
}

int VoiceChanger::Release()
{
	if(window)			delete [] window; window=0;
	if(overlap_buf)		delete [] overlap_buf; window=0;
	if(tem_outbuf)		delete [] tem_outbuf;tem_outbuf=0;
	if(tem_inbuf)		delete [] tem_inbuf; tem_inbuf=0;
	if(ipp_buf)			delete [] ipp_buf; ipp_buf=0;
	if(ipp_corr)		delete [] ipp_corr; ipp_corr=0;
	if(rxx_buf)			delete [] rxx_buf; rxx_buf=0;
	isInit=false;
	return 0;
}

/*
*  ratio=1   corresponds to normal voice
*  ratio>1   corresponds to higher pitch frequency
*  ratio<1 corresponds to lower  pitch frequency
*/

int VoiceChanger::Init(double ratio,int delta_search)
{
	VS_AutoLock lock(this);
	Release();

	if((ratio>=0.25)&&(koeff<=4))
		koeff=ratio;
	else
		koeff=1;
	if(delta_search>=0)
		Kmax=delta_search;
	else
		Kmax=320;

	sa=160/(double)koeff;
	ss=160;
	overlap_len=framelen-ss;
	ipp_buf=new double[overlap_len+Kmax];
	ipp_corr=new double[Kmax];
	rxx_buf= new double[Kmax];
	tem_inbuf=new short[framelen];
	tem_outbuf=new short[overlap_len];
	overlap_buf=new short [overlap_len];
	window=new double [framelen];
	for(int i=0;i<framelen;i++)
		window[i]=0.5-0.5*cos(2*pi*i/(framelen-1));
	isInit=true;
	return 0;
}
// add+overlap implementation
int VoiceChanger::AddOverlap(short* data,int len)
{
	for(int i=0;i<overlap_len;i++)
		tem_outbuf[i]=overlap_buf[i]+data[i];
	for(int i=0;i<overlap_len-ss;i++)
		overlap_buf[i]=tem_outbuf[i+ss];
	for(int i=overlap_len-ss,j=0;i<overlap_len;i++,j++)
		overlap_buf[i]=data[j+overlap_len];
	outbuf.insert(outbuf.end(),tem_outbuf, tem_outbuf+ss);
	return 0;
}

int VoiceChanger::Process(short* data, int &size)
{
	VS_AutoLock lock(this);
	if(!isInit)
		return 1;
	if(koeff==1)
		return 0;
	int len=size/2;
	double ryy=0;
	double Rxy=0;
	inque.insert(inque.end(),data, data+len);
	int total_size=0;
	short* pData=0;
	while(inque.size()>=framelen+Kmax)
	{
		pData=&inque.front();
		ryy=Norma(overlap_buf,overlap_len);
		for(int j=0;j<overlap_len+Kmax;j++)
			ipp_buf[j]=pData[j];
		for(int i=0;i<Kmax;i++)
			ippmL2Norm_v_64f(ipp_buf+i, 8, rxx_buf+i, overlap_len);
		ippsCrossCorr_16s64s(overlap_buf, overlap_len,pData, overlap_len+Kmax,(Ipp64s *) ipp_corr, Kmax,0);
		double max=-1000000000;
		int max_ind=0;
		for(int i=0;i<Kmax;i++)
		{
			Rxy=(double)ipp_corr[i]/(double)(rxx_buf[i]+ryy);
			if(Rxy>max)
			{
				max=Rxy;
				max_ind=i;
			}
		}
		memcpy(tem_inbuf,pData+max_ind,framelen*sizeof(short));
		for(int i=0;i<framelen;i++)
			tem_inbuf[i]=tem_inbuf[i]*window[i];
		AddOverlap(tem_inbuf,framelen);
		total_size=total_size+ss;
		inque.erase(inque.begin(), inque.begin()+sa);
	}
	int out_len=outbuf.size();
	if(out_len>0)
	{
		short* pdata = &outbuf.front();
		out_len=res.Process(pdata,data,outbuf.size()*2, 48000, (48000/koeff));
		size=out_len;
		outbuf.clear();
	}

	return 0;
}

double VoiceChanger::GetRatio()
{
	return koeff;
}




/*////////////////////////////////////////////////////////////////
****************** VS_FastSimpleStat ****************************
///////////////////////////////////////////////////////////////*/

VS_FastSimpleStat::VS_FastSimpleStat()
{
	m_data=0;
	m_data_square=0;
	m_mav=0;
	m_std=0;
	m_currcount=0;
	m_sum=0;
	m_sum_square=0;
	m_period=1;
	m_IsInit=false;
}

VS_FastSimpleStat::~VS_FastSimpleStat()
{
	Release();
}


int VS_FastSimpleStat::Init(int period)
{
	Release();
	if(period>0)
		m_period=period;
	m_data=new double [m_period];
	m_data_square=new double [m_period];
	m_IsInit=true;
	ClearStat();
	return 0;
}

int VS_FastSimpleStat::Release()
{
	if(m_data)			delete [] m_data; m_data=0;
	if(m_data_square)	delete [] m_data_square; m_data_square=0;
	m_period=1;
	m_mav=0;
	m_std=0;
	m_sum=0;
	m_sum_square=0;
	m_currcount=0;
	m_IsInit=false;
	return 0;
}


double VS_FastSimpleStat::GetMovingAverage()
{
	return m_mav;
}

double VS_FastSimpleStat::GetMovingStd()
{
	return m_std;
}

int VS_FastSimpleStat::AddValue(double value)
{
	if(!m_IsInit) return -1;

	m_sum=m_sum+value - m_data[m_currcount%m_period];
	m_sum_square=m_sum_square+value*value - m_data_square[m_currcount%m_period];
	m_data[m_currcount%m_period]=value;
	m_data_square[m_currcount%m_period]=value*value;
	m_mav=(m_sum/m_period);
	m_std=sqrt(m_sum_square/m_period - m_mav*m_mav);
	m_currcount++;
	return 0;
}

int VS_FastSimpleStat::ClearStat()
{
	if(!m_IsInit) return -1;

	if(m_data)
		memset(m_data,0,sizeof(double)*m_period);
	if(m_data_square)
		memset(m_data_square,0,sizeof(double)*m_period);
	m_sum=0;
	m_sum_square=0;
	m_mav=0;
	m_std=0;
	m_currcount=0;
	return 0;
}

bool VS_FastSimpleStat::IsComplete()
{
	if(m_currcount>=m_period)
		return true;
	else
		return false;
}