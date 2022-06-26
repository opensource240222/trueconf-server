
#ifndef VPX_CODEC_INTERFACE_H
#define VPX_CODEC_INTERFACE_H

struct vpx_param
{
	int width;
	int height;
	int frame_rate;
	int rate_control_method;
	int bitrate;
	int i_maxinterval;
	int ref_frames;
	unsigned int deadline;
	int cpu_used;
	int me_static_threshold;
	int error_resilient;
	int num_threads;
	int postproc;
	int iframe_mode;
	unsigned int svc_mode;
	unsigned char snd_lvl;
	int type_codec;
};

class VPXCodec
{
	bool			m_bCoder;
	unsigned int	m_iTag;
protected:
	void			*m_pCodec;
	int				m_iSize;
	int				m_iWidth, m_iHeight;
    bool			m_bValid;
public:
	VPXCodec(unsigned int outTag, bool coder);
	virtual ~VPXCodec();
	virtual bool Init(vpx_param *par) { return false; }
	virtual void Release() {};
	virtual bool SetCodecOptions(vpx_param *par) { return false; }
	virtual bool GetCodecOptions(vpx_param *par) { return false; }
	virtual int	 GetFrame(unsigned char *invideo, unsigned char *outvideo, int *param) { return 0; }
	bool IsValid() { return m_bValid; }
	bool IsCoder() { return m_bCoder; }
	unsigned int GetTag() { return m_iTag; }
};

VPXCodec* VS_RetriveVPXCodec(int tag, bool isCoder);

#endif
