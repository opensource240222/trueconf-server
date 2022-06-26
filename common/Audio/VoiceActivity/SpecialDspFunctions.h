
#include "../../Transcoder/VS_AudioReSampler.h"
#include "../../std/cpplib/VS_Lock.h"
#include <queue>

/*
Estimate spectrum entropy  of frame or sequence.

[in]   frame;
[in]   framesize, must be framesize%2==0;

[out] return value - spectrum entropy fo frame or -1 if incorrect framesize.

*/




class VoiceChanger:public VS_Lock
{
private:
	VS_AudioReSamplerSpeex		res;
	std::vector<short>			inque;
	std::vector<short>			outbuf;
	short*						overlap_buf;
	double						*ipp_buf;
	double						*ipp_corr;
	double						*rxx_buf;
	double						*window;
	short*						tem_outbuf;
	short*						tem_inbuf;
	int							ss;
	int							sa;
	int							framelen;
	int							overlap_len;
	int							Kmax;
	double						koeff;
	bool						isInit;
    int							AddOverlap(short* data,int len);
	double						Norma(short int* buf, int len);
	double						xcorr(short int* buf1, short* buf2, int len);
	int							Release();
public:
	VoiceChanger();
	~VoiceChanger();
	int Init(double ratio, int delta_samples=240);
	int Process(short* data, int &size);
	double GetRatio();
};


class VS_FastSimpleStat
{
	double*			m_data;
	double*			m_data_square;
	double			m_mav;
	double			m_std;
	double			m_sum;
	double			m_sum_square;
	int				m_period;
	unsigned int	m_currcount;
	bool			m_IsInit;

public:
	VS_FastSimpleStat();
	~VS_FastSimpleStat();
	int			AddValue(double value);
	int			Init(int period);
	int			Release();

	double		GetMovingAverage();
	double		GetMovingStd();
	bool		IsComplete();

	int			ClearStat();

};