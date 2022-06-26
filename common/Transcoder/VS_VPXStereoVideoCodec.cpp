#ifdef _WIN32
#include "VS_VPXStereoVideoCodec.h"
#include "std/cpplib/VS_Lock.h"
#include "std/cpplib/ThreadUtils.h"
#include "std/cpplib/VS_RegistryKey.h"
#include <process.h>

// TODO: get rid
#include "VSClient/VS_ApplicationInfo.h"

static  unsigned int __stdcall vs_vpx_thread_encode_routine(void *p)
{
	vs::SetThreadName("VPXEncoder");
	int k = 0;
	HANDLE handle[2];
	VS_VPXHDState *vpx_st = (VS_VPXHDState*)p;
	VPXCodec *vpx = vpx_st->vpx;
	handle[0] = vpx_st->handle_wait_coding;
	handle[1] = vpx_st->handle_wait_exit;
	while (1) {
		unsigned long res = WaitForMultipleObjects(2, handle, 0, -1);
		if (res == WAIT_OBJECT_0 + 0) {
			vpx_st->cmpsize = vpx->GetFrame(vpx_st->invideo, vpx_st->outvideo, &vpx_st->param);
			SetEvent(vpx_st->handle_end_coding);
		}
		else if (res == WAIT_OBJECT_0 + 1) {
			return 0;
		}
		else {
			return -1;
		}
	}
}

#define DELTAB (32)

VS_VPXHDVideoCodec::VS_VPXHDVideoCodec(int CodecId, bool IsCoder)
	: VideoCodec(CodecId, IsCoder)
{
	m_num_threads = 3;
	VS_RegistryKey key(true, REG_CurrentConfiguratuon);
	key.GetValue(&m_num_threads, 4, VS_REG_INTEGER_VT, "Number Threads");

	memset(m_VPXcodecSt, 0, MAX_NUM_THREADS * sizeof(VS_VPXHDState));
	for (int i = 0; i < m_num_threads; i++) {
		m_VPXcodecSt[i].vpx = VS_RetriveVPXCodec(CodecId, IsCoder);
	}
	m_valid = false;
}

VS_VPXHDVideoCodec::~VS_VPXHDVideoCodec()
{
	Release();
	for (int i = 0; i < m_num_threads; i++) {
		delete m_VPXcodecSt[i].vpx;
	}
}

int VS_VPXHDVideoCodec::Init(int w, int h, uint32_t ColorMode, unsigned char sndLvl, int numThreads, unsigned int framerate)
{
	int res = 0, i = 0;

	Release();

	m_width = w;
	m_height = h;

	for (i = 0; i < m_num_threads; i++) {
		if (m_VPXcodecSt[i].vpx) res++;
	}

	if (res == m_num_threads) {
		if (ColorMode != FOURCC_I420)
			return -1;

		vpx_param par;
		memset(&par, 0, sizeof(vpx_param));

		int bitrate = (int)((double)m_bitrate / (double)m_num_threads + 0.5);

		if (IsCoder()) {
			par.i_maxinterval = 1000;
			par.bitrate = bitrate;
			par.rate_control_method = 1;
			par.frame_rate = framerate;
			par.num_threads = 1;
			par.deadline = 0;
			par.cpu_used = -4;
			par.me_static_threshold = 800;
			par.error_resilient = 0;
			par.ref_frames = 1;
			par.iframe_mode = 0;
			par.svc_mode = 0;
		}
		else {
			par.postproc = 0;
		}

		m_valid = true;

		for (i = 0; i < m_num_threads; i++) {
			int hc = 0;
			int wc = w;
			int offset[3] = {};
			int sizePlane[3] = {};

			offset[0] = i * w * (h / m_num_threads);
			offset[1] = w * h + i * w / 2 * (h / m_num_threads / 2);
			offset[2] = 5 * w * h / 4 + i * w / 2 * (h / m_num_threads / 2);

			if (IsCoder()) {
				if (i == m_num_threads - 1) {
					hc = h - (h / m_num_threads) * (m_num_threads - 1) + DELTAB;
					offset[0] -= DELTAB * wc;
					offset[1] -= DELTAB / 2 * wc / 2;
					offset[2] -= DELTAB / 2 * wc / 2;
				}
				else if (i == 0) {
					hc = h / m_num_threads + DELTAB;
				}
				else {
					hc = h / m_num_threads + 2 * DELTAB;
					offset[0] -= DELTAB * wc;
					offset[1] -= DELTAB / 2 * wc / 2;
					offset[2] -= DELTAB / 2 * wc / 2;
				}
				sizePlane[0] = wc * hc;
				sizePlane[1] = (wc / 2) * (hc / 2);
				sizePlane[2] = (wc / 2) * (hc / 2);
			}
			else {
				if (i == m_num_threads - 1) {
					hc = h - (h / m_num_threads) * (m_num_threads - 1);
					sizePlane[0] = wc * hc;
					sizePlane[1] = (wc / 2) * (hc / 2);
					sizePlane[2] = (wc / 2) * (hc / 2);
					hc += DELTAB;
				}
				else if (i == 0) {
					hc = h / m_num_threads;
					sizePlane[0] = wc * hc;
					sizePlane[1] = (wc / 2) * (hc / 2);
					sizePlane[2] = (wc / 2) * (hc / 2);
					hc += DELTAB;
				}
				else {
					hc = h / m_num_threads;
					sizePlane[0] = wc * hc;
					sizePlane[1] = (wc / 2) * (hc / 2);
					sizePlane[2] = (wc / 2) * (hc / 2);
					hc += 2 * DELTAB;
				}
			}

			m_VPXcodecSt[i].offset[0] = offset[0];
			m_VPXcodecSt[i].offset[1] = offset[1];
			m_VPXcodecSt[i].offset[2] = offset[2];

			m_VPXcodecSt[i].sizePlane[0] = sizePlane[0];
			m_VPXcodecSt[i].sizePlane[1] = sizePlane[1];
			m_VPXcodecSt[i].sizePlane[2] = sizePlane[2];

			par.width = wc;
			par.height = hc;
			m_VPXcodecSt[i].width = wc;
			m_VPXcodecSt[i].height = hc;

			m_VPXcodecSt[i].outvideo = new unsigned char[wc * hc * 3 / 2];

			m_valid = m_valid && m_VPXcodecSt[i].vpx->Init(&par);

			if (IsCoder()) {
				m_VPXcodecSt[i].invideo = new unsigned char[wc * hc * 3 / 2];
				m_VPXcodecSt[i].Plane[0] = m_VPXcodecSt[i].invideo;
				m_VPXcodecSt[i].Plane[1] = m_VPXcodecSt[i].invideo + wc * hc;
				m_VPXcodecSt[i].Plane[2] = m_VPXcodecSt[i].invideo + 5 * wc * hc / 4;

				m_VPXcodecSt[i].handle_wait_coding = CreateEvent(NULL, FALSE, FALSE, NULL);
				m_VPXcodecSt[i].handle_end_coding = CreateEvent(NULL, FALSE, FALSE, NULL);
				m_VPXcodecSt[i].handle_wait_exit = CreateEvent(NULL, FALSE, FALSE, NULL);
				m_VPXcodecSt[i].handle_thread = (HANDLE)_beginthreadex(0, 0, vs_vpx_thread_encode_routine, &m_VPXcodecSt[i], 0, 0);
			}
			else {
				if (i == 0) {
					m_VPXcodecSt[i].Plane[0] = m_VPXcodecSt[i].outvideo;
					m_VPXcodecSt[i].Plane[1] = m_VPXcodecSt[i].outvideo + wc * hc;
					m_VPXcodecSt[i].Plane[2] = m_VPXcodecSt[i].outvideo + 5 * wc * hc / 4;
				}
				else if (i == m_num_threads - 1) {
					m_VPXcodecSt[i].Plane[0] = m_VPXcodecSt[i].outvideo + DELTAB * wc;
					m_VPXcodecSt[i].Plane[1] = m_VPXcodecSt[i].outvideo + wc * hc + DELTAB / 2 * wc / 2;
					m_VPXcodecSt[i].Plane[2] = m_VPXcodecSt[i].outvideo + 5 * wc * hc / 4 + DELTAB / 2 * wc / 2;
				}
				else {
					m_VPXcodecSt[i].Plane[0] = m_VPXcodecSt[i].outvideo + DELTAB * wc;
					m_VPXcodecSt[i].Plane[1] = m_VPXcodecSt[i].outvideo + wc * hc + DELTAB / 2 * wc / 2;
					m_VPXcodecSt[i].Plane[2] = m_VPXcodecSt[i].outvideo + 5 * wc * hc / 4 + DELTAB / 2 * wc / 2;
				}
			}
		}
	}
	return m_valid ? 0 : -1;
}

int VS_VPXHDVideoCodec::ReInitDecoderHD(int num_threads)
{
	int i = 0, res = 0;
	m_valid = false;

	for (i = 0; i < m_num_threads; i++) {
		delete m_VPXcodecSt[i].outvideo; m_VPXcodecSt[i].outvideo = 0;
		delete m_VPXcodecSt[i].vpx;
		memset(&m_VPXcodecSt[i], 0, sizeof(VS_VPXHDState));
	}

	m_num_threads = num_threads;

	for (i = 0; i < m_num_threads; i++) {
		m_VPXcodecSt[i].vpx = VS_RetriveVPXCodec(VS_VCODEC_VPX, false);
		if (m_VPXcodecSt[i].vpx) res++;
	}

	if (res == m_num_threads) {
		vpx_param par;
		memset(&par, 0, sizeof(vpx_param));

		m_valid = true;

		for (i = 0; i < m_num_threads; i++) {
			int hc = 0;
			int wc = m_width;
			int sizePlane[3] = {};

			if (i == m_num_threads - 1) {
				hc = m_height - (m_height / m_num_threads) * (m_num_threads - 1);
				sizePlane[0] = wc * hc;
				sizePlane[1] = (wc / 2) * (hc / 2);
				sizePlane[2] = (wc / 2) * (hc / 2);
				hc += DELTAB;
			}
			else if (i == 0) {
				hc = m_height / m_num_threads;
				sizePlane[0] = wc * hc;
				sizePlane[1] = (wc / 2) * (hc / 2);
				sizePlane[2] = (wc / 2) * (hc / 2);
				hc += DELTAB;
			}
			else {
				hc = m_height / m_num_threads;
				sizePlane[0] = wc * hc;
				sizePlane[1] = (wc / 2) * (hc / 2);
				sizePlane[2] = (wc / 2) * (hc / 2);
				hc += 2 * DELTAB;
			}

			m_VPXcodecSt[i].offset[0] = i * m_width * (m_height / m_num_threads);
			m_VPXcodecSt[i].offset[1] = m_width * m_height + i * m_width / 2 * (m_height / m_num_threads / 2);
			m_VPXcodecSt[i].offset[2] = 5 * m_width * m_height / 4 + i * m_width / 2 * (m_height / m_num_threads / 2);

			m_VPXcodecSt[i].sizePlane[0] = sizePlane[0];
			m_VPXcodecSt[i].sizePlane[1] = sizePlane[1];
			m_VPXcodecSt[i].sizePlane[2] = sizePlane[2];

			par.width = wc;
			par.height = hc;
			m_VPXcodecSt[i].width = wc;
			m_VPXcodecSt[i].height = hc;

			m_VPXcodecSt[i].outvideo = new unsigned char[wc * hc * 3 / 2];

			if (i == 0) {
				m_VPXcodecSt[i].Plane[0] = m_VPXcodecSt[i].outvideo;
				m_VPXcodecSt[i].Plane[1] = m_VPXcodecSt[i].outvideo + wc * hc;
				m_VPXcodecSt[i].Plane[2] = m_VPXcodecSt[i].outvideo + 5 * wc * hc / 4;
			}
			else if (i == m_num_threads - 1) {
				m_VPXcodecSt[i].Plane[0] = m_VPXcodecSt[i].outvideo + DELTAB * wc;
				m_VPXcodecSt[i].Plane[1] = m_VPXcodecSt[i].outvideo + wc * hc + DELTAB / 2 * wc / 2;
				m_VPXcodecSt[i].Plane[2] = m_VPXcodecSt[i].outvideo + 5 * wc * hc / 4 + DELTAB / 2 * wc / 2;
			}
			else {
				m_VPXcodecSt[i].Plane[0] = m_VPXcodecSt[i].outvideo + DELTAB * wc;
				m_VPXcodecSt[i].Plane[1] = m_VPXcodecSt[i].outvideo + wc * hc + DELTAB / 2 * wc / 2;
				m_VPXcodecSt[i].Plane[2] = m_VPXcodecSt[i].outvideo + 5 * wc * hc / 4 + DELTAB / 2 * wc / 2;
			}

			m_valid = m_valid && m_VPXcodecSt[i].vpx->Init(&par);
		}
	}

	return m_valid ? 0 : -1;
}

void VS_VPXHDVideoCodec::Release()
{
	int i = 0;
	m_valid = false;
	m_bitrate = 128;
	m_bitrate_prev = m_bitrate;

	for (i = 0; i < m_num_threads; i++) {
		if (IsCoder()) {
			if (m_VPXcodecSt[i].handle_wait_exit)
			{
				SetEvent(m_VPXcodecSt[i].handle_wait_exit);
				if (WaitForSingleObject(m_VPXcodecSt[i].handle_thread, -1) == WAIT_OBJECT_0) {
					CloseHandle(m_VPXcodecSt[i].handle_thread);
					CloseHandle(m_VPXcodecSt[i].handle_end_coding);
					CloseHandle(m_VPXcodecSt[i].handle_wait_coding);
					CloseHandle(m_VPXcodecSt[i].handle_wait_exit);
					m_VPXcodecSt[i].handle_thread = 0;
					m_VPXcodecSt[i].handle_end_coding = 0;
					m_VPXcodecSt[i].handle_wait_coding = 0;
					m_VPXcodecSt[i].handle_wait_exit = 0;
				}
			}
			delete[] m_VPXcodecSt[i].invideo; m_VPXcodecSt[i].invideo = 0;
		}
		delete[] m_VPXcodecSt[i].outvideo; m_VPXcodecSt[i].outvideo = 0;
	}
}

int VS_VPXHDVideoCodec::Convert(uint8_t *in, uint8_t* out, VS_VideoCodecParam* param)
{
	int i = 0, k = 0, ret = 0;
	unsigned char *pY, *pU, *pV;

	if (IsCoder()) {
		UpdateBitrate();

		HANDLE handle_end_coding[MAX_NUM_THREADS];

		for (i = 0; i < m_num_threads; i++) {
			m_VPXcodecSt[i].param = param->cmp.KeyFrame != 1;
			handle_end_coding[i] = m_VPXcodecSt[i].handle_end_coding;
			memcpy(m_VPXcodecSt[i].Plane[0], in + m_VPXcodecSt[i].offset[0], m_VPXcodecSt[i].sizePlane[0]);
			memcpy(m_VPXcodecSt[i].Plane[1], in + m_VPXcodecSt[i].offset[1], m_VPXcodecSt[i].sizePlane[1]);
			memcpy(m_VPXcodecSt[i].Plane[2], in + m_VPXcodecSt[i].offset[2], m_VPXcodecSt[i].sizePlane[2]);
		}

		for (i = 0; i < m_num_threads; i++) {
			SetEvent(m_VPXcodecSt[i].handle_wait_coding);
		}

		WaitForMultipleObjects(m_num_threads, handle_end_coding, 1, -1);

		ret = m_VPXcodecSt[0].cmpsize + 4 + 4;
		param->cmp.IsKeyFrame = (m_VPXcodecSt[0].param == 0);
		unsigned char *pOut = out;
		*(int*)(pOut) = m_num_threads;
		pOut += 4;
		*(int*)(pOut) = m_VPXcodecSt[0].cmpsize;
		pOut += 4;
		memcpy(pOut, m_VPXcodecSt[0].outvideo, m_VPXcodecSt[0].cmpsize);
		pOut += m_VPXcodecSt[0].cmpsize;
		for (i = 1; i < m_num_threads; i++) {
			ret += m_VPXcodecSt[i].cmpsize + 4;
			param->cmp.IsKeyFrame = param->cmp.IsKeyFrame && (m_VPXcodecSt[i].param == 0);
			*(int*)(pOut) = m_VPXcodecSt[i].cmpsize;
			pOut += 4;
			memcpy(pOut, m_VPXcodecSt[i].outvideo, m_VPXcodecSt[i].cmpsize);
			pOut += m_VPXcodecSt[i].cmpsize;
		}

		return ret;
	}
	else {
		unsigned char *pIn = in;
		int size = 0;
		int num_threads = *(int*)(pIn);
		pIn += 4;
		if (num_threads != m_num_threads) ret = ReInitDecoderHD(num_threads);

		if (ret == 0) {
			for (i = 0; i < m_num_threads; i++) {
				int cmpsize = *(int*)pIn;
				pIn += 4;
				ret += m_VPXcodecSt[i].vpx->GetFrame(pIn, m_VPXcodecSt[i].outvideo, &cmpsize);
				if (ret > 0) {
					pIn += cmpsize;
					pY = out + m_VPXcodecSt[i].offset[0];
					pU = out + m_VPXcodecSt[i].offset[1];
					pV = out + m_VPXcodecSt[i].offset[2];
					memcpy(pY, m_VPXcodecSt[i].Plane[0], m_VPXcodecSt[i].sizePlane[0]);
					memcpy(pU, m_VPXcodecSt[i].Plane[1], m_VPXcodecSt[i].sizePlane[1]);
					memcpy(pV, m_VPXcodecSt[i].Plane[2], m_VPXcodecSt[i].sizePlane[2]);
					size += m_VPXcodecSt[i].sizePlane[0] + m_VPXcodecSt[i].sizePlane[1] + m_VPXcodecSt[i].sizePlane[2];
				}
			}
			return size;
		}

		return 0;
	}

	return 0;
}

bool VS_VPXHDVideoCodec::UpdateBitrate()
{
	if (m_bitrate_prev == m_bitrate) return true;
	m_bitrate_prev = m_bitrate;
	if (IsCoder()) {
		int i = 0;
		vpx_param par;
		for (i = 0; i < m_num_threads; i++) {
			memset(&par, 0, sizeof(vpx_param));
			if (m_VPXcodecSt[i].vpx->GetCodecOptions(&par)) {
				par.bitrate = (int)((double)m_bitrate_prev / (double)m_num_threads + 0.5);
				m_VPXcodecSt[i].vpx->SetCodecOptions(&par);
			}
		}
		return true;
	}
	return false;
}

bool VS_VPXHDVideoCodec::SetCoderOption(void *param)
{
	if (IsCoder()) {
		int i = 0;
		vpx_param *inpar = (vpx_param*)param;
		vpx_param par;
		memset(&par, 0, sizeof(vpx_param));
		for (i = 0; i < m_num_threads; i++) {
			if (m_VPXcodecSt[i].vpx->GetCodecOptions(&par)) {
				par.bitrate = (int)((double)inpar->bitrate / (double)m_num_threads + 0.5);
				par.rate_control_method = inpar->rate_control_method;
				par.error_resilient = inpar->error_resilient;
				par.deadline = inpar->deadline;
				par.ref_frames = inpar->ref_frames;
				par.cpu_used = inpar->cpu_used;
				par.me_static_threshold = inpar->me_static_threshold;
				m_VPXcodecSt[i].vpx->SetCodecOptions(&par);
			}
		}
		return true;
	}
	return false;
}

VS_VPXStereoVideoCodec::VS_VPXStereoVideoCodec(int CodecId, bool IsCoder)
	: VS_VPXHDVideoCodec(CodecId, IsCoder)
{
	int i;
	m_num_threads = 2;
	for (i = 0; i < m_num_threads; i++) {
		if (m_VPXcodecSt[i].vpx == NULL) {
			m_VPXcodecSt[i].vpx = VS_RetriveVPXCodec(CodecId, IsCoder);
		}
	}
	for (; i < MAX_NUM_THREADS; i++) {
		delete m_VPXcodecSt[i].vpx;
		memset(&m_VPXcodecSt[i], 0, sizeof(VS_VPXHDState));
	}
	m_valid = false;
}

VS_VPXStereoVideoCodec::~VS_VPXStereoVideoCodec()
{

}

int VS_VPXStereoVideoCodec::Init(int w, int h, uint32_t ColorMode, unsigned char sndLvl, int numThreads, unsigned int framerate)
{
	int res = 0, i = 0;

	Release();

	for (i = 0; i < m_num_threads; i++) {
		if (m_VPXcodecSt[i].vpx) res++;
	}

	if (res == m_num_threads) {
		if (ColorMode != FOURCC_I420)
			return -1;

		vpx_param par;
		memset(&par, 0, sizeof(vpx_param));

		int hc = h / m_num_threads;
		int wc = w;
		par.width = wc;
		par.height = hc;

		m_bitrate = (int)((double)m_bitrate / (double)m_num_threads + 0.5);

		if (IsCoder()) {
			par.i_maxinterval = 1000;
			par.bitrate = m_bitrate;
			par.rate_control_method = 1;
			par.frame_rate = framerate;
			par.num_threads = 1;
			par.deadline = 0;
			par.cpu_used = -4;
			par.me_static_threshold = 800;
			par.error_resilient = 0;
			par.ref_frames = 1;
			par.iframe_mode = 0;
			par.svc_mode = 0;
		}
		else {
			par.postproc = 0;
		}

		m_valid = true;

		for (i = 0; i < m_num_threads; i++) {
			m_VPXcodecSt[i].offset[0] = i * w * (h / m_num_threads);
			m_VPXcodecSt[i].offset[1] = w * h + i * w / 2 * (h / m_num_threads / 2);
			m_VPXcodecSt[i].offset[2] = 5 * w * h / 4 + i * w / 2 * (h / m_num_threads / 2);

			m_VPXcodecSt[i].sizePlane[0] = wc * hc;
			m_VPXcodecSt[i].sizePlane[1] = wc * hc / 4;
			m_VPXcodecSt[i].sizePlane[2] = wc * hc / 4;

			m_VPXcodecSt[i].width = wc;
			m_VPXcodecSt[i].height = hc;

			m_VPXcodecSt[i].outvideo = new unsigned char[wc * hc * 3 / 2];

			m_valid = m_valid && m_VPXcodecSt[i].vpx->Init(&par);

			if (IsCoder()) {
				m_VPXcodecSt[i].invideo = new unsigned char[wc * hc * 3 / 2];
				m_VPXcodecSt[i].Plane[0] = m_VPXcodecSt[i].invideo;
				m_VPXcodecSt[i].Plane[1] = m_VPXcodecSt[i].invideo + wc * hc;
				m_VPXcodecSt[i].Plane[2] = m_VPXcodecSt[i].invideo + 5 * wc * hc / 4;

				m_VPXcodecSt[i].handle_wait_coding = CreateEvent(NULL, FALSE, FALSE, NULL);
				m_VPXcodecSt[i].handle_end_coding = CreateEvent(NULL, FALSE, FALSE, NULL);
				m_VPXcodecSt[i].handle_wait_exit = CreateEvent(NULL, FALSE, FALSE, NULL);
				m_VPXcodecSt[i].handle_thread = (HANDLE)_beginthreadex(0, 0, vs_vpx_thread_encode_routine, &m_VPXcodecSt[i], 0, 0);
			}
			else {
				m_VPXcodecSt[i].Plane[0] = m_VPXcodecSt[i].outvideo;
				m_VPXcodecSt[i].Plane[1] = m_VPXcodecSt[i].outvideo + wc * hc;
				m_VPXcodecSt[i].Plane[2] = m_VPXcodecSt[i].outvideo + 5 * wc * hc / 4;
			}
		}
	}
	return m_valid ? 0 : -1;
}
#endif
