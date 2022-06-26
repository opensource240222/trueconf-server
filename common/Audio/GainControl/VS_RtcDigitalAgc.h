class VS_RtcDigitalAgc
{
private:
	void* m_agc;
	unsigned char m_warn;
	bool  m_isInit;
	int   m_buf_len;
	int m_fs;
	int m_st1[6];
	int m_st2[6];
	int m_st3[6];
	int m_st4[6];
	short int m_Low_Band[160];
	short int m_High_Band[160];
public:
	 VS_RtcDigitalAgc();
	~VS_RtcDigitalAgc();

	int Init(int sample_rate, int agc_mode, int targetDb=20, int comprGain=20, bool limEnable=true);
	int Release();

	int Process(short int* data, int len);

};