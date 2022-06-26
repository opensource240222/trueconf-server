#include "VS_iSACAudioCodec.h"

#include "modules/audio_coding/codecs/isac/main/include/isac.h"
#include "modules/audio_coding/codecs/isac/main/source/settings.h"

VS_iSACAudioCodec::VS_iSACAudioCodec(uint32_t tag, bool coder):AudioCodec(tag, coder, 640)
{
	m_state = 0;
}

VS_iSACAudioCodec::~VS_iSACAudioCodec()
{
	Release();
}

int VS_iSACAudioCodec::Init(WAVEFORMATEX *in)
{
	Release();

	int res = 0;

	m_cdcfmt = (WAVEFORMATEX*)malloc(sizeof(WAVEFORMATEX));
	m_pcmfmt = (WAVEFORMATEX*)malloc(sizeof(WAVEFORMATEX));
	m_inBuff = (uint8_t*)malloc(m_SIZE);
	m_outBuff = (uint8_t*)malloc(m_SIZE);
	m_DataPointer = m_inBuff;
	m_ash.cbStruct = sizeof(ACMSTREAMHEADER);
	m_ash.pbSrc = m_inBuff;
	m_ash.pbDst = m_outBuff;

	m_pcmfmt->wFormatTag = WAVE_FORMAT_PCM;
	m_pcmfmt->nSamplesPerSec = in->nSamplesPerSec;
	m_pcmfmt->wBitsPerSample = 16;
	m_pcmfmt->nChannels = 1;
	m_pcmfmt->nBlockAlign = 2;
	m_pcmfmt->nAvgBytesPerSec = m_pcmfmt->nSamplesPerSec*m_pcmfmt->nBlockAlign;
	m_pcmfmt->cbSize = 0;

	m_cdcfmt->wFormatTag = (uint16_t)GetTag();
	m_cdcfmt->nSamplesPerSec = in->nSamplesPerSec;
	m_cdcfmt->wBitsPerSample = 0;
	m_cdcfmt->nChannels = 1;
	m_cdcfmt->nBlockAlign = 120;
	m_cdcfmt->cbSize = 0;
	m_cdcfmt->nAvgBytesPerSec = 4000;

	IsacSamplingRate sampRate = kIsacWideband;
	int samplerate = 16, bitrate = 32000, framesize = 30, maxrate = 53400;
	if (in->nSamplesPerSec >= 32000) {
		sampRate = kIsacSuperWideband;
		samplerate = 32;
		bitrate = 56000;
		maxrate = 160000;
	}
	res = WebRtcIsac_Create(&m_state);
	if (res == 0) {
		if (IsCoder()) {
			res = WebRtcIsac_EncoderInit(m_state, 1);
			if (res == 0) {
				res = WebRtcIsac_SetEncSampRate(m_state, sampRate*1000);
				if (res == 0) {
					res = WebRtcIsac_SetMaxRate(m_state, maxrate);
					res = WebRtcIsac_Control(m_state, bitrate, framesize);
					m_GRAN = samplerate * 2 * framesize;
					m_frame_size = m_GRAN / 3;
				}
			}
		} else {
			WebRtcIsac_DecoderInit(m_state);
			if (m_state) {
				res = WebRtcIsac_SetDecSampRate(m_state, sampRate*1000);
				if (res == 0) {
					m_frame_size = samplerate * 2 * framesize;
					m_GRAN = 1;
				}
			}
		}
	}

	m_valid = true;
	return res;
}

void VS_iSACAudioCodec::Release()
{
	if (m_valid) {
		free(m_inBuff); m_inBuff = 0; m_DataPointer = 0;
		free(m_outBuff); m_outBuff = 0;
		free(m_cdcfmt); m_cdcfmt = 0;
		free(m_pcmfmt); m_pcmfmt = 0;
		memset(&m_ash, 0, sizeof(ACMSTREAMHEADER));
		WebRtcIsac_Free(m_state);
		m_state = 0;
		m_valid = false;
	}
}

int VS_iSACAudioCodec::ConvertFunction()
{
	int nBytes = 0;
	unsigned len = m_ash.cbSrcLength;
	m_ash.cbSrcLengthUsed = 0;
	m_ash.cbDstLengthUsed = 0;
	if (IsCoder()) {
		while (len >= m_GRAN) {
			while (nBytes == 0) {
				nBytes = WebRtcIsac_Encode(m_state, (short*)(m_ash.pbSrc + m_ash.cbSrcLengthUsed), m_ash.pbDst + m_ash.cbDstLengthUsed);
				m_ash.cbSrcLengthUsed += m_frame_size;
			}
			if (nBytes > 0) m_ash.cbDstLengthUsed += nBytes;
			len -= m_GRAN;
			nBytes = 0;
		}
	} else {
		short speechtype = 0;
		short dbytes = (short)m_ash.cbSrcLength;
		while (dbytes > 0) {
			short lenFrame = 0;
			nBytes = WebRtcIsac_Decode_LenFrame(m_state, m_ash.pbSrc + m_ash.cbSrcLengthUsed, dbytes, (short*)(m_ash.pbDst + m_ash.cbDstLengthUsed), &speechtype, &lenFrame);
			if (nBytes >= 0) {
				m_ash.cbSrcLengthUsed += lenFrame;
				dbytes -= lenFrame;
				m_ash.cbDstLengthUsed += nBytes * 2;
			} else {
				m_ash.cbSrcLengthUsed += dbytes;
				dbytes = 0;
				m_ash.cbDstLengthUsed += m_frame_size;
			}
		}
	}
	return 0;
}

const int isac_quality2bitrate[11] =
{
	10400, 10400, 10400, 10400, 12800, 16800, 20800, 24000, 28000, 32000, 32000
};

void VS_iSACAudioCodec::SetQuality(int quality)
{
	if (m_valid && IsCoder()) {
		WebRtcIsac_Control(m_state, isac_quality2bitrate[quality], 30);
	}
}
