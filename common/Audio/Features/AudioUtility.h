#pragma once


class VS_BandSplitterCombiner
{
private:
	int m_st1[6];
	int m_st2[6];
	int m_st3[6];
	int m_st4[6];

public:
	VS_BandSplitterCombiner();
	~VS_BandSplitterCombiner();

	int Split(short int* InData,int InLen, short int* LowBand, short int* HighBand);
	int Combine(short int* LowBand, short int* HighBand, int InLen, short int* OutData);

	int Refresh(); //refresh fiter states
};

