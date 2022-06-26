#include "VPXCodec.h"
#include <memory.h>
#include <malloc.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/// for MT
#include "vpx_config.h"

#define VPX_CODEC_DISABLE_COMPAT 1
#include "vpx/vpx_encoder.h"

//#if CONFIG_VP8_ENCODER && !defined(interface_cx)

#include "vpx/vp8cx.h"
#define interface_cx (&vpx_codec_vp8_cx_algo)

//#endif

#define VPX_CODEC_DISABLE_COMPAT 1
#include "vpx/vpx_decoder.h"

//#if CONFIG_VP8_DECODER && !defined(interface_dx)

#include "vpx/vp8dx.h"
#define interface_dx (&vpx_codec_vp8_dx_algo)

//#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

//#define TEST_LOST_SLICE

#include <stdio.h>

#ifdef TEST_LOST_SLICE
#include <windows.h>
#include <windef.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define MAX_NUM_LOST_SLICE (3)
#define MAX_INTERVAL_SLICE (30)

struct vs_test_lost_slice {
	int		frame_interval_lost;
	int		num_lost_slice;
	int		count_slice;
	int		all_count_slice;
	int		slice_lost[MAX_NUM_LOST_SLICE];
	int		is_intra;
	int		num_frames;
	int		num_i_frames;
	int		max_num_lost_slice;
	int		max_interval_lost_slice;
	FILE*	log_slice;
} test_lost;

#endif

struct VS_SVCState
{
	int iMaxSpatial;
	int iTemporal;
	int iSpatial;
	int iQuality;
};

class VPXEncoder : public VPXCodec
{
	vpx_image_t  		*m_pImage;
	vpx_codec_enc_cfg_t	m_Cfg;
	unsigned int		m_iDeadline;
	int					m_iMeThreshold;
	int					m_iCpuUsed;
	VS_SVCState			m_stSVC;
	bool				m_bSVCEnable;
	unsigned char		m_uSndLevel;
	uint64_t				m_iFrameCnt, m_iFrameGOPCnt;
	void PrepareFrame(unsigned char *pSrcFrame);
public:
	VPXEncoder(unsigned int outTag);
	~VPXEncoder();
	bool Init(vpx_param *par);
	void Release();
	bool SetCodecOptions(vpx_param *par);
	bool GetCodecOptions(vpx_param *par);
	int	 GetFrame(unsigned char *invideo, unsigned char *outvideo, int *param);
};

class VPXDecoder : public VPXCodec
{
public:
	VPXDecoder(unsigned int outTag);
	~VPXDecoder();
	bool Init(vpx_param *par);
	void Release();
	int	 GetFrame(unsigned char *invideo, unsigned char *outvideo, int *param);
};

VPXEncoder::VPXEncoder(unsigned int outTag) : VPXCodec(outTag, true)
{
	m_pImage = 0;
	m_iDeadline = VPX_DL_REALTIME;
	m_iFrameCnt = 0;
	m_iFrameGOPCnt = 0;
	m_iMeThreshold = 800;
	m_iCpuUsed = -4;
	memset(&m_stSVC, 0, sizeof(VS_SVCState));
	m_bSVCEnable = false;

#ifdef TEST_LOST_SLICE
	test_lost.frame_interval_lost = 10;
	test_lost.all_count_slice = 0;
	test_lost.is_intra = 1;
	test_lost.num_lost_slice = 0;
	test_lost.count_slice = 0;
	test_lost.log_slice = fopen("vpx_slice_log.txt", "w");
	test_lost.num_frames = 0;
	test_lost.num_i_frames = 0;
	test_lost.max_num_lost_slice = MAX_NUM_LOST_SLICE;
	test_lost.max_interval_lost_slice = MAX_INTERVAL_SLICE;
	srand((unsigned)time(NULL));
#endif

}

VPXEncoder::~VPXEncoder()
{
	Release();

#ifdef TEST_LOST_SLICE
	fclose(test_lost.log_slice);
#endif

}

bool VPXEncoder::Init(vpx_param *par)
{
	int flag = 0;
	vpx_codec_err_t		res = VPX_CODEC_OK;

	Release();

	vpx_codec_enc_cfg_t cfg;

	res = vpx_codec_enc_config_default(interface_cx, &cfg, 0);
	if (res != VPX_CODEC_OK) return false;

	cfg.g_usage = 0;
	cfg.g_profile = 0;
	cfg.g_w = par->width;
	cfg.g_h = par->height;
	cfg.g_timebase.num = 1;
	cfg.g_timebase.den = par->frame_rate;
	cfg.rc_target_bitrate = par->bitrate;
	cfg.rc_end_usage = (par->rate_control_method == 0) ? VPX_VBR : VPX_CBR;
	cfg.rc_min_quantizer = 4;
	cfg.rc_max_quantizer = 63;
	if (cfg.rc_end_usage == VPX_CBR) {
		cfg.rc_buf_initial_sz = 2000;
		cfg.rc_buf_optimal_sz = 2000;
		cfg.rc_buf_sz = 3000;
	}
	cfg.rc_dropframe_thresh = 0;
	cfg.rc_overshoot_pct = 0;
	cfg.rc_undershoot_pct = 0;
	cfg.g_pass = VPX_RC_ONE_PASS;
	cfg.g_error_resilient = par->error_resilient;
	cfg.g_threads = par->num_threads;
	if (par->svc_mode == 0) {
		cfg.kf_mode = (par->iframe_mode == 0) ? VPX_KF_DISABLED : VPX_KF_AUTO;
		cfg.kf_max_dist = par->i_maxinterval;
	} else {
		cfg.kf_mode = VPX_KF_DISABLED;
		cfg.kf_min_dist = cfg.kf_max_dist = par->i_maxinterval;
	}
	cfg.g_lag_in_frames = (par->ref_frames > 0) ? (par->ref_frames - 1) : 0;

	m_uSndLevel = par->snd_lvl;

	if (par->svc_mode != 0) {
		/// temporal svc
		if (par->svc_mode & 0x00000800) {
			// 2-layers, 2-frame period
			int ids[4] = {0,1};
			cfg.ts_number_layers     = 2;
			cfg.ts_periodicity       = 2;
			cfg.ts_rate_decimator[0] = 2;
			cfg.ts_rate_decimator[1] = 1;
			cfg.ts_target_bitrate[1] = cfg.rc_target_bitrate;
			cfg.ts_target_bitrate[0] = cfg.ts_target_bitrate[1] * 6 / 10;
			memcpy(cfg.ts_layer_id, ids, sizeof(ids));
			m_stSVC.iTemporal = 8;
		}
		else if (par->svc_mode & 0x00000200) {
			// 3-layers, 4-frame period, not vp8 method
			int ids[4] = {0,1,2};
			cfg.ts_number_layers = 0;
			cfg.ts_periodicity = 3;
			memcpy(cfg.ts_layer_id, ids, sizeof(ids));
			m_stSVC.iTemporal = 4;
		}
		else if (par->svc_mode & 0x00000400) {
			// 3-layers, 4-frame period
			int ids[4] = {0,2,1,2};
			cfg.ts_number_layers     = 3;
			cfg.ts_periodicity       = 4;
			cfg.ts_rate_decimator[0] = 4;
			cfg.ts_rate_decimator[1] = 2;
			cfg.ts_rate_decimator[2] = 1;
			cfg.ts_target_bitrate[2] = cfg.rc_target_bitrate;
			cfg.ts_target_bitrate[1] = 3 * cfg.ts_target_bitrate[2] / 4;
			cfg.ts_target_bitrate[0] = cfg.ts_target_bitrate[2] / 2;
			memcpy(cfg.ts_layer_id, ids, sizeof(ids));
			m_stSVC.iTemporal = 2;
		}
		else if (par->svc_mode & 0x00000100) {
			// 2-layers, 2-frame period, not vp8 method
			int ids[4] = {0,1};
			cfg.ts_number_layers = 0;
			cfg.ts_periodicity = 2;
			memcpy(cfg.ts_layer_id, ids, sizeof(ids));
			m_stSVC.iTemporal = 1;
		}
		m_bSVCEnable = true;
	}

	switch (par->deadline)
	{
		case 0: m_iDeadline = VPX_DL_REALTIME; break;
		case 1: m_iDeadline = VPX_DL_GOOD_QUALITY; break;
		case 2: m_iDeadline = VPX_DL_BEST_QUALITY; break;
		default: m_iDeadline = par->deadline; break;
	}
	m_iCpuUsed = par->cpu_used;
	m_iMeThreshold = par->me_static_threshold;

#ifdef TEST_LOST_SLICE
	cfg.g_error_resilient = VPX_ERROR_RESILIENT_PARTITIONS;
	flag |= VPX_CODEC_USE_OUTPUT_PARTITION;
#endif

	if (m_bSVCEnable && m_uSndLevel != 0) {
		if (m_uSndLevel >= 58) m_iCpuUsed = 3;
		m_iMeThreshold /= 4;
	}
	m_pImage = (vpx_image_t*)malloc(sizeof(vpx_image_t));
	res = vpx_codec_enc_init((vpx_codec_ctx_t*)m_pCodec, interface_cx, &cfg, flag);
	if (res != VPX_CODEC_OK) {
		Release();
		return false;
	}
	vpx_codec_control_((vpx_codec_ctx_t*)m_pCodec, VP8E_SET_CPUUSED, m_iCpuUsed);
	vpx_codec_control_((vpx_codec_ctx_t*)m_pCodec, VP8E_SET_STATIC_THRESHOLD, m_iMeThreshold);

#ifdef TEST_LOST_SLICE
	vpx_codec_control_(pCodec, VP8E_SET_TOKEN_PARTITIONS, VP8_EIGHT_TOKENPARTITION);
#endif

	memcpy(&m_Cfg, &cfg, sizeof(vpx_codec_enc_cfg_t));

	m_iSize = par->width * par->height;
	m_bValid = true;

	return true;
}

void VPXEncoder::Release()
{
	if (m_bValid) vpx_codec_destroy((vpx_codec_ctx_t*)m_pCodec);
	if (m_pImage) {
		free(m_pImage);
		m_pImage = 0;
	}
	m_iSize = 0;
	m_iFrameCnt = 0;
	m_iFrameGOPCnt = 0;
	m_iDeadline = VPX_DL_REALTIME;
	m_iMeThreshold = 800;
	m_iCpuUsed = -4;
	memset(&m_stSVC, 0, sizeof(VS_SVCState));
	m_bSVCEnable = false;
	m_bValid = false;
}

bool VPXEncoder::SetCodecOptions(vpx_param *par)
{
	if (!m_bValid) return false;

	m_Cfg.g_timebase.num = 1;
	m_Cfg.g_timebase.den = par->frame_rate;
	m_Cfg.rc_target_bitrate = par->bitrate;
	m_Cfg.rc_end_usage = (par->rate_control_method == 0) ? VPX_VBR : VPX_CBR;
	if (m_Cfg.rc_end_usage == VPX_CBR) {
		m_Cfg.rc_buf_initial_sz = 2000;
		m_Cfg.rc_buf_optimal_sz = 2000;
		m_Cfg.rc_buf_sz = 3000;
	}
	m_Cfg.g_error_resilient = par->error_resilient;
	m_Cfg.g_threads = par->num_threads;
	if (par->svc_mode == 0) {
		m_Cfg.kf_mode = (par->iframe_mode == 0) ? VPX_KF_DISABLED : VPX_KF_AUTO;
		m_Cfg.kf_max_dist = par->i_maxinterval;
	} else {
		m_Cfg.kf_mode = VPX_KF_DISABLED;
		m_Cfg.kf_min_dist = m_Cfg.kf_max_dist = par->i_maxinterval;
	}
	m_Cfg.g_lag_in_frames = (par->ref_frames > 0) ? (par->ref_frames - 1) : 0;

	switch (par->deadline)
	{
		case 0: m_iDeadline = VPX_DL_REALTIME; break;
		case 1: m_iDeadline = VPX_DL_GOOD_QUALITY; break;
		case 2: m_iDeadline = VPX_DL_BEST_QUALITY; break;
		default: m_iDeadline = par->deadline; break;
	}

	m_iMeThreshold = par->me_static_threshold;
	m_iCpuUsed = par->cpu_used;
	m_uSndLevel = par->snd_lvl;

	m_stSVC.iSpatial = 0;
	m_stSVC.iTemporal = 0;
	m_stSVC.iQuality = 0;
	m_bSVCEnable = false;
	if (par->svc_mode != 0) {
		/// temporal svc
		if (par->svc_mode & 0x00000800) {
			// 2-layers, 2-frame period
			int ids[4] = {0,1};
			m_Cfg.ts_number_layers     = 2;
			m_Cfg.ts_periodicity       = 2;
			m_Cfg.ts_rate_decimator[0] = 2;
			m_Cfg.ts_rate_decimator[1] = 1;
			m_Cfg.ts_target_bitrate[1] = m_Cfg.rc_target_bitrate;
			m_Cfg.ts_target_bitrate[0] = m_Cfg.ts_target_bitrate[1] * 6 / 10;
			memcpy(m_Cfg.ts_layer_id, ids, sizeof(ids));
			m_stSVC.iTemporal = 8;
		}
		else if (par->svc_mode & 0x00000200) {
			// 3-layers, 4-frame period, not vp8 method
			int ids[4] = {0,1,2};
			m_Cfg.ts_number_layers = 0;
			m_Cfg.ts_periodicity = 3;
			memcpy(m_Cfg.ts_layer_id, ids, sizeof(ids));
			m_stSVC.iTemporal = 4;
		}
		else if (par->svc_mode & 0x00000400) {
			// 3-layers, 4-frame period
			int ids[4] = {0,2,1,2};
			m_Cfg.ts_number_layers     = 3;
			m_Cfg.ts_periodicity       = 4;
			m_Cfg.ts_rate_decimator[0] = 4;
			m_Cfg.ts_rate_decimator[1] = 2;
			m_Cfg.ts_rate_decimator[2] = 1;
			m_Cfg.ts_target_bitrate[2] = m_Cfg.rc_target_bitrate;
			m_Cfg.ts_target_bitrate[1] = 3 * m_Cfg.ts_target_bitrate[2] / 4;
			m_Cfg.ts_target_bitrate[0] = m_Cfg.ts_target_bitrate[2] / 2;
			memcpy(m_Cfg.ts_layer_id, ids, sizeof(ids));
			m_stSVC.iTemporal = 2;
		}
		else if (par->svc_mode & 0x00000100) {
			// 2-layers, 2-frame period, not vp8 method
			int ids[4] = {0,1};
			m_Cfg.ts_number_layers = 0;
			m_Cfg.ts_periodicity = 2;
			memcpy(m_Cfg.ts_layer_id, ids, sizeof(ids));
			m_stSVC.iTemporal = 1;
		}
		m_bSVCEnable = true;
	}

#ifdef TEST_LOST_SLICE
	m_Cfg.g_error_resilient = VPX_ERROR_RESILIENT_PARTITIONS;
#endif

	vpx_codec_err_t res = VPX_CODEC_OK;
	res = vpx_codec_enc_config_set((vpx_codec_ctx_t*)m_pCodec, &m_Cfg);
	vpx_codec_control_((vpx_codec_ctx_t*)m_pCodec, VP8E_SET_CPUUSED, m_iCpuUsed);
	vpx_codec_control_((vpx_codec_ctx_t*)m_pCodec, VP8E_SET_STATIC_THRESHOLD, m_iMeThreshold);

	return (res == VPX_CODEC_OK);
}

bool VPXEncoder::GetCodecOptions(vpx_param *par)
{
	if (!m_bValid) return false;

	par->width = m_Cfg.g_w;
	par->height = m_Cfg.g_h;
	par->frame_rate = m_Cfg.g_timebase.den;
	par->bitrate = m_Cfg.rc_target_bitrate;
	par->rate_control_method = (m_Cfg.rc_end_usage == VPX_VBR) ? 0 : 1;
	par->num_threads = m_Cfg.g_threads;
	par->i_maxinterval = m_Cfg.kf_max_dist;
	par->ref_frames = m_Cfg.g_lag_in_frames + 1;
	par->error_resilient = m_Cfg.g_error_resilient;
	par->iframe_mode = (m_Cfg.kf_mode == VPX_KF_DISABLED) ? 0 : 1;
	par->num_threads = m_Cfg.g_threads;

	switch (m_iDeadline)
	{
		case VPX_DL_REALTIME: par->deadline = 0; break;
		case VPX_DL_GOOD_QUALITY: par->deadline = 1; break;
		case VPX_DL_BEST_QUALITY: par->deadline = 2; break;
		default: par->deadline = m_iDeadline; break;
	}

	par->cpu_used = m_iCpuUsed;
	par->me_static_threshold = m_iMeThreshold;
	par->snd_lvl = m_uSndLevel;

	par->svc_mode = 0x00000000;
	par->svc_mode |= ((unsigned int)m_stSVC.iTemporal)    <<  8;
	par->svc_mode |= ((unsigned int)m_stSVC.iQuality)     <<  0;

	return true;
}

static unsigned int vpx_enc_flags_0[3] =
{
	VP8_EFLAG_NO_UPD_ARF | VP8_EFLAG_NO_UPD_GF | VP8_EFLAG_NO_REF_ARF,
	VP8_EFLAG_FORCE_ARF | VP8_EFLAG_NO_UPD_GF | VP8_EFLAG_NO_UPD_LAST | VP8_EFLAG_NO_REF_ARF,
	VP8_EFLAG_NO_UPD_GF | VP8_EFLAG_NO_UPD_ARF | VP8_EFLAG_NO_UPD_LAST,
};

static unsigned int vpx_enc_flags_1[4] =
{
	VP8_EFLAG_NO_REF_GF | VP8_EFLAG_NO_REF_ARF | VP8_EFLAG_NO_UPD_GF | VP8_EFLAG_NO_UPD_ARF,
	VP8_EFLAG_NO_REF_ARF | VP8_EFLAG_NO_UPD_LAST | VP8_EFLAG_NO_UPD_GF | VP8_EFLAG_NO_UPD_ARF,
	VP8_EFLAG_NO_REF_GF | VP8_EFLAG_NO_REF_ARF | VP8_EFLAG_NO_UPD_ARF | VP8_EFLAG_NO_UPD_LAST,
	VP8_EFLAG_NO_REF_ARF | VP8_EFLAG_NO_UPD_LAST | VP8_EFLAG_NO_UPD_GF | VP8_EFLAG_NO_UPD_ARF,
};

static unsigned int vpx_enc_flags_2[2] =
{
	VP8_EFLAG_NO_UPD_ARF | VP8_EFLAG_NO_UPD_GF | VP8_EFLAG_NO_REF_ARF,
	VP8_EFLAG_FORCE_ARF | VP8_EFLAG_NO_UPD_GF | VP8_EFLAG_NO_UPD_LAST | VP8_EFLAG_NO_REF_ARF,
};

static unsigned int vpx_enc_flags_3[2] =
{
	VP8_EFLAG_NO_UPD_ARF | VP8_EFLAG_NO_UPD_GF | VP8_EFLAG_NO_REF_GF | VP8_EFLAG_NO_REF_ARF,
	VP8_EFLAG_NO_UPD_ARF | VP8_EFLAG_NO_UPD_LAST | VP8_EFLAG_NO_REF_ARF/* | VP8_EFLAG_NO_REF_LAST*/,
};

void VPXEncoder::PrepareFrame(unsigned char *pSrcFrame)
{
	vpx_img_wrap(m_pImage, VPX_IMG_FMT_I420, m_Cfg.g_w, m_Cfg.g_h, 1, pSrcFrame);
}

int	VPXEncoder::GetFrame(unsigned char *invideo, unsigned char *outvideo, int *param)
{
	if (!m_bValid) return false;

	int size_all = 0, i = 0, sl = 0;
	vpx_codec_err_t	res = VPX_CODEC_OK;

	int flags = 0;

	if (m_bSVCEnable) {
		int temporal_id = 0;
		if (m_stSVC.iTemporal > 0) {
			if (*param == 0) m_iFrameGOPCnt = 0;
			if (m_stSVC.iTemporal > 0) {
				int idx = m_iFrameGOPCnt % m_Cfg.ts_periodicity;
				temporal_id = m_Cfg.ts_layer_id[idx];
				switch (m_stSVC.iTemporal) {
					case 8: flags = vpx_enc_flags_3[idx]; break;
					case 4: flags = vpx_enc_flags_0[idx]; break;
					case 2: flags = vpx_enc_flags_1[idx]; break;
					case 1: flags = vpx_enc_flags_2[idx]; break;
				}
				if (m_iFrameGOPCnt == 0) {
					flags = VPX_EFLAG_FORCE_KF | VP8_EFLAG_FORCE_GF | VP8_EFLAG_FORCE_ARF |
							VP8_EFLAG_NO_REF_GF | VP8_EFLAG_NO_REF_ARF | VP8_EFLAG_NO_REF_LAST;
				}
			}
		} else {
			if (*param == 0) flags = VPX_EFLAG_FORCE_KF;
		}
		*param = (0x000000ff & temporal_id);
	} else {
		if (*param == 0) flags = VPX_EFLAG_FORCE_KF;
		*param = 0x00000000;
	}

	PrepareFrame(invideo);
	res = vpx_codec_encode((vpx_codec_ctx_t*)m_pCodec, m_pImage, m_iFrameCnt, 1, flags, m_iDeadline);

	int size = 0;
	int i_size = 0;

#ifdef TEST_LOST_SLICE
	fprintf(test_lost.log_slice,
			"####################  %d frame  ####################\n",
			test_lost.num_frames);
	//printf("####################  %d frame  ####################\n",
	//		test_lost.num_frames);
	int lost_frame = 0;
#endif

	const vpx_codec_cx_pkt_t *pkt;
	vpx_codec_iter_t iter = NULL;
	while( (pkt = vpx_codec_get_cx_data((vpx_codec_ctx_t*)m_pCodec, &iter)) ) {
		switch(pkt->kind) {
			case VPX_CODEC_CX_FRAME_PKT:
				*param &= 0x00ffffff;
				if (pkt->data.frame.flags & VPX_FRAME_IS_KEY) {
					m_iFrameGOPCnt = 0;
				} else {
					*param |= 0x01000000;
				}
				i_size = pkt->data.frame.sz;

#ifdef TEST_LOST_SLICE
				if (test_lost.num_lost_slice == 0) {
					int tmp = rand();
					if ((tmp % test_lost.max_interval_lost_slice) == 0) {
						tmp = rand();
						test_lost.num_lost_slice = tmp % test_lost.max_num_lost_slice;
					}
				}
				if (test_lost.num_lost_slice == 0) {
					fprintf(test_lost.log_slice, "%d slice: f = %d, l = %d \n", pkt->data.frame.partition_id, *param, i_size);
				} else {
					test_lost.num_lost_slice--;
					if (pkt->data.frame.partition_id == 0) {
						fprintf(test_lost.log_slice, "!!! Lost Frame !!!\n");
						lost_frame = 1;
					} else {
						fprintf(test_lost.log_slice, "!!! Lost %d slice !!! f = %d, l = %d \n", pkt->data.frame.partition_id, *param, i_size);
					}
					i_size = 0;
				}
				//if (test_lost.num_lost_slice == 0) {
				//	printf("%d slice: f = %d, l = %d \n", pkt->data.frame.partition_id, *param, i_size);
				//} else {
				//	test_lost.num_lost_slice--;
				//	if (pkt->data.frame.partition_id == 0) {
				//		printf("!!! Lost Frame !!!");
				//		lost_frame = 1;
				//	} else {
				//		printf("!!! Lost %d slice !!! f = %d, l = %d \n", pkt->data.frame.partition_id, *param, i_size);
				//	}
				//	i_size = 0;
				//}
#endif

#ifdef TEST_LOST_SLICE
				if (lost_frame != 1) {
#endif

					memcpy(outvideo + size, pkt->data.frame.buf, i_size);
					size += i_size;

#ifdef TEST_LOST_SLICE
				}
#endif

				break;
			default:
				break;
		}

#ifdef TEST_LOST_SLICE
		if (lost_frame == 1) break;
#endif

	}

	m_iFrameCnt++;
	m_iFrameGOPCnt++;

#ifdef TEST_LOST_SLICE
	test_lost.num_frames++;
#endif

	return size;
}

VPXDecoder::VPXDecoder(unsigned int outTag) : VPXCodec(outTag, false)
{

}

VPXDecoder::~VPXDecoder()
{
	Release();
}

bool VPXDecoder::Init(vpx_param *par)
{
	vpx_codec_err_t		res = VPX_CODEC_OK;

	Release();

	vpx_codec_dec_cfg_t cfg;

	m_iWidth = par->width;
	m_iHeight = par->height;

	cfg.w = par->width;
	cfg.h = par->height;
	cfg.threads = par->num_threads;

	int flag = 0;
	if (par->postproc == 1) {
		flag |= VPX_CODEC_USE_POSTPROC;
	}

#ifdef TEST_LOST_SLICE
	flag |= VPX_CODEC_USE_ERROR_CONCEALMENT;
#endif

	res = vpx_codec_dec_init((vpx_codec_ctx_t*)m_pCodec, interface_dx, &cfg, flag);
	if (res != VPX_CODEC_OK) return false;

	m_iSize = par->width * par->height;
	m_bValid = true;

	return true;
}

void VPXDecoder::Release()
{
	if (m_bValid) {
		vpx_codec_destroy((vpx_codec_ctx_t*)m_pCodec);
	}
	m_iSize = 0;
	m_bValid = false;
}

int	VPXDecoder::GetFrame(unsigned char *invideo, unsigned char *outvideo, int *param)
{
	if (!m_bValid) return false;

	int size = 0, i = 0;
	unsigned char *pY, *pU, *pV;
	vpx_codec_err_t	res = VPX_CODEC_OK;

    vpx_codec_iter_t  iter = NULL;
    vpx_image_t      *img;
	res = vpx_codec_decode((vpx_codec_ctx_t*)m_pCodec, invideo, *param, NULL, 0);
	img = vpx_codec_get_frame((vpx_codec_ctx_t*)m_pCodec, &iter);
	if (img && img->fmt == VPX_IMG_FMT_I420) {
		pY = outvideo,
		pU = outvideo + m_iSize,
		pV = pU + (img->d_w / 2) * (img->d_h / 2);
		for (i = 0; i < img->d_h; i++) {
			memcpy(pY, img->planes[0] + i * img->stride[0], img->d_w);
			pY += m_iWidth;
			if (!(i & 1)) {
				memcpy(pU, img->planes[1] + (i / 2) * img->stride[1], img->d_w / 2);
				memcpy(pV, img->planes[2] + (i / 2) * img->stride[2], img->d_w / 2);
				pU += m_iWidth / 2;
				pV += m_iWidth / 2;
			}
		}
		size = m_iSize * 3 / 2;
	}

	return size;
}

VPXCodec::VPXCodec(unsigned int outTag, bool coder)
{
	m_bCoder = coder;
	m_iTag = outTag;
	m_bValid = false;
	m_pCodec = malloc(sizeof(vpx_codec_ctx_t));
}

VPXCodec::~VPXCodec()
{
	free(m_pCodec);
	m_pCodec = 0;
	m_bCoder = false;
	m_bValid = false;
	m_iTag = 0;
}

VPXCodec* VS_RetriveVPXCodec(int tag, bool isCoder)
{
	if (isCoder) {
		return new VPXEncoder(tag);
	} else {
		return new VPXDecoder(tag);
	}
}
