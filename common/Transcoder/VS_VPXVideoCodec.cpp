#include "VS_VPXVideoCodec.h"

VS_VPXVideoCodec::VS_VPXVideoCodec(int CodecId, bool IsCoder)
	: VideoCodec(CodecId, IsCoder)
{
	m_VPXcodec = VS_RetriveVPXCodec(CodecId, IsCoder);
	m_valid = false;
}

VS_VPXVideoCodec::~VS_VPXVideoCodec()
{
	Release();
	if (m_VPXcodec) delete m_VPXcodec; m_VPXcodec = 0;
}

int VS_VPXVideoCodec::Init(int w, int h, uint32_t ColorMode, unsigned char sndLvl, int numThreads, unsigned int framerate)
{
	Release();
	if (m_VPXcodec) {
		if (ColorMode != FOURCC_I420)
			return -1;

		vpx_param par;
		memset(&par, 0, sizeof(vpx_param));
		par.width = w;
		par.height = h;

		int num_treads = numThreads;

		if (IsCoder()) {
			if (num_treads == 1) {
				if (w >= 1280 && h >= 720) {
					num_treads = m_num_phcores + (m_num_lcores / (m_num_phcores << 1)) * (m_num_phcores >> 1);
					if (num_treads > 4) num_treads = 4;
				}
			}
			m_num_threads = num_treads;
			par.num_threads = num_treads;
			par.i_maxinterval = 1000;
			par.bitrate = m_bitrate;
			par.rate_control_method = 1;
			par.frame_rate = framerate;
			par.deadline = 1;
			par.cpu_used = 5;
			par.me_static_threshold = 800;
			par.error_resilient = 0;
			par.ref_frames = 1;
			par.iframe_mode = 1;
			par.svc_mode = 0;
			par.snd_lvl = sndLvl;
		}
		else {
			par.num_threads = num_treads;
			par.postproc = 0;
		}

		m_valid = m_VPXcodec->Init(&par);
		SetBitrate(m_bitrate);
	}
	return m_valid ? 0 : -1;
}

void VS_VPXVideoCodec::Release()
{
	m_valid = false;
	m_bitrate = 128;
	m_bitrate_prev = m_bitrate;
	m_num_threads = 1;
}

int VS_VPXVideoCodec::Convert(uint8_t *in, uint8_t* out, VS_VideoCodecParam* param)
{
	if (IsCoder()) {
		UpdateBitrate();
		int prm = param->cmp.KeyFrame != 1;
		int ret = m_VPXcodec->GetFrame(in, out, &prm);
		int type = ((prm & 0xff000000) >> 24);
		param->cmp.IsKeyFrame = (type == 0);
		param->cmp.Quality = (prm & 0x000000ff);
		return ret;
	}
	else
		return m_VPXcodec->GetFrame(in, out, (int*)&param->dec.FrameSize);
	return 0;
}

bool VS_VPXVideoCodec::UpdateBitrate()
{
	if (m_bitrate_prev == m_bitrate) return true;
	m_bitrate_prev = m_bitrate;
	vpx_param par;
	memset(&par, 0, sizeof(vpx_param));
	if (m_VPXcodec->GetCodecOptions(&par)) {
		par.bitrate = m_bitrate_prev;
		return m_VPXcodec->SetCodecOptions(&par);
	}
	return false;
}

bool VS_VPXVideoCodec::SetSVCMode(uint32_t &param)
{
	vpx_param par;
	memset(&par, 0, sizeof(vpx_param));
	if (m_VPXcodec->GetCodecOptions(&par)) {
		if (param > 0) {
			par.iframe_mode = 0;
			par.error_resilient = 1;
			par.i_maxinterval = 450;
			par.svc_mode = (param == 0xffffffff) ? 0 : param;
		}
		else {
			par.iframe_mode = 1;
			par.error_resilient = 0;
			par.i_maxinterval = 1000;
			par.svc_mode = 0;
		}
		if (m_VPXcodec->SetCodecOptions(&par)) {
			if (m_VPXcodec->GetCodecOptions(&par)) {
				param = par.svc_mode;
			}
			return true;
		}
	}
	return false;
}

bool VS_VPXVideoCodec::SetCoderOption(void *param)
{
	vpx_param *inpar = (vpx_param*)param;
	vpx_param par;
	memset(&par, 0, sizeof(vpx_param));
	if (m_VPXcodec->GetCodecOptions(&par)) {
		par.bitrate = inpar->bitrate;
		par.rate_control_method = inpar->rate_control_method;
		par.error_resilient = inpar->error_resilient;
		par.deadline = inpar->deadline;
		par.ref_frames = inpar->ref_frames;
		par.cpu_used = inpar->cpu_used;
		par.me_static_threshold = inpar->me_static_threshold;
		par.i_maxinterval = inpar->i_maxinterval;
		return m_VPXcodec->SetCodecOptions(&par);
	}
	return false;
}
