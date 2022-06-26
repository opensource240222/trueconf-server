#pragma once

#include "std/cpplib/VS_MediaFormat.h"

union VS_VideoCodecParam
{
	struct Compress {
		uint32_t		KeyFrame;
		uint32_t		FrameSize;
		uint32_t		Quality;
		uint32_t		IsKeyFrame;
	} cmp;
	struct Decompress {
		uint32_t		Flags;
		uint32_t		FrameSize;
	} dec;
};

struct base_Param
{
	int32_t width = 0;
	int32_t height = 0;
	int32_t out_width = 0;
	int32_t out_height = 0;
	uint32_t color_space = 0;
	bool device_memory = false;
};

struct h264_Param
{
	int i_maxinterval;				///< max i-frame interval
	int p_maxinterval;				///< max p-frame interval
	int Width;						///< width of source image
	int Height;						///< height of source image
	int rate_control_method;		///< method of rate control
	int quantizer;					///< quantizer
	int bitrate;					///< bitrate
	int me_split_mode;				///< split blocks mode
	int me_search_x;				///< search range on direction X
	int me_search_y;				///< search range on direction Y
	int entropy_coding_mode;		///< entropy coding mode
	int cabac_init_idc;				///< cabac model coding
	int time_resolution;			///< frame rate of source bitstream
	int	num_ref_frames;				///< number of reference frames
	int level_idc;					///< profile level
	int rate_control_type;			///< type of rate control (currently always equal 0)
	int deblocking_mode;			///< in-loop debocking filter
	int	deblocking_filter_alpha;	///< debocking filter parameter
	int	deblocking_filter_beta;		///< debocking filter parameter
	int slice_lenght;				///< slice lenght (in bytes)
	int pre_filtering;				///< prefiltering
	int	mv_search_idc;				///< motion estimation algorithm type
	int direct_mode;				///< type of direct mode for B-slice
	int idr_interval;				///< max idr-frame interval
	int me_quality;					///< motion estimation algorithm quality
	int num_threads;				///< encoding threads number
	int max_dec_buffer;				///< max decoder store buffer
	int error_concealate_mode;		///< mode of the decoder error concealate mechanism
};

class VideoCodec
{
private:
	uint32_t			m_fcc;
	bool				m_coder;

protected:
	bool				m_valid;
	uint32_t			m_bitrate, m_bitrate_prev;
	unsigned int		m_num_phcores, m_num_lcores;
	uint32_t			m_num_threads = 1;

	virtual bool		UpdateBitrate() { return false; }

public:
	VideoCodec(uint32_t fcc, bool coder);
	virtual ~VideoCodec();

	virtual int Init(int w, int h, uint32_t ColorMode = FOURCC_I420, unsigned char sndLvl = 0, int numThreads = 1, unsigned int framerate = 10) = 0;
	virtual int InitExtended(const base_Param &settings);
	virtual void Release() {};
	virtual int Convert(uint8_t* in, uint8_t* out, VS_VideoCodecParam* param) = 0;
	virtual bool SetCoderOption(uint32_t param);
	virtual bool SetBitrate(uint32_t param);

	virtual bool SetSVCMode(uint32_t& /*param*/) { return false; }
	virtual bool SetCoderOption(void* /*param*/) { return false; }

	int GetBitrate();
	uint32_t GetNumThreads() const;

	bool IsCoder(){return m_coder;}
	bool IsValid(){return m_valid;}
	uint32_t GetFcc(){return m_fcc;}
};
