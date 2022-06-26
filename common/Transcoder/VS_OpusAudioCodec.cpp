#include "VS_OpusAudioCodec.h"
#include "opus/opus.h"

#define OPUS_MAX_PAYLOAD (4000)

VS_OpusAudioCodec::VS_OpusAudioCodec(uint32_t tag, bool coder):AudioCodec(tag, coder, 1280)
{
	m_state = 0;
	m_rp = 0;
	m_pRepacket = 0;
}

VS_OpusAudioCodec::~VS_OpusAudioCodec()
{
	Release();
}

int VS_OpusAudioCodec::Init(WAVEFORMATEX *in)
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
	m_cdcfmt->nBlockAlign = 75;
	m_cdcfmt->cbSize = 0;
	m_cdcfmt->nAvgBytesPerSec = 3750;

	int err = 0;
	int	bitrate = 30000,
		framesize = 40,
		cmplx = 5,
		signal_type = OPUS_SIGNAL_VOICE;

	m_samplerate = in->nSamplesPerSec,
	m_samplerate = (m_samplerate + 2000) / 4000 * 4000;
	if (m_samplerate > 24000) m_samplerate = 48000;

	if (IsCoder()) {
		err = OPUS_ALLOC_FAIL;
		m_rp = opus_repacketizer_create();
		if (m_rp) {
			m_pRepacket = (unsigned char*)malloc(OPUS_MAX_PAYLOAD);
			m_state = opus_encoder_create(m_samplerate, 1, OPUS_APPLICATION_VOIP, &err);
			if (err == OPUS_OK && m_state) {
				opus_encoder_ctl((OpusEncoder*)m_state, OPUS_SET_COMPLEXITY(cmplx));
				opus_encoder_ctl((OpusEncoder*)m_state, OPUS_SET_BITRATE(bitrate));
				opus_encoder_ctl((OpusEncoder*)m_state, OPUS_SET_SIGNAL(signal_type));
				opus_encoder_ctl((OpusEncoder*)m_state, OPUS_SET_VBR(1));
				m_GRAN = (m_samplerate / 1000) * 2 * framesize;
				m_frame_size = m_GRAN;
			}
		}
	} else {
		m_state = opus_decoder_create(m_samplerate, 1, &err);
		if (err == OPUS_OK && m_state) {
			m_frame_size = (m_samplerate / 1000) * 2 * framesize;
			m_GRAN = 1;
		}
	}

	if (err != OPUS_OK) {
		free(m_inBuff); m_inBuff = 0; m_DataPointer = 0;
		free(m_outBuff); m_outBuff = 0;
		free(m_cdcfmt); m_cdcfmt = 0;
		free(m_pcmfmt); m_pcmfmt = 0;
		memset(&m_ash, 0, sizeof(ACMSTREAMHEADER));
		res = -1;
	} else {
		m_valid = true;
	}

	return res;
}

void VS_OpusAudioCodec::Release()
{
	if (m_valid) {
		free(m_inBuff); m_inBuff = 0; m_DataPointer = 0;
		free(m_outBuff); m_outBuff = 0;
		free(m_cdcfmt); m_cdcfmt = 0;
		free(m_pcmfmt); m_pcmfmt = 0;
		memset(&m_ash, 0, sizeof(ACMSTREAMHEADER));
		if (IsCoder()) {
			opus_encoder_destroy((OpusEncoder*)m_state);
			opus_repacketizer_destroy((OpusRepacketizer*)m_rp); m_rp = 0;
			free(m_pRepacket); m_pRepacket = 0;
		} else {
			opus_decoder_destroy((OpusDecoder*)m_state);
		}
		m_state = 0;
		m_valid = false;
	}
}

int VS_OpusAudioCodec::ConvertFunction()
{
	int nBytes = 0;
	m_ash.cbSrcLengthUsed = 0;
	m_ash.cbDstLengthUsed = 0;
	if (IsCoder()) {
		unsigned char *pDst = m_pRepacket;
		opus_repacketizer_init((OpusRepacketizer*)m_rp);
		while (m_ash.cbSrcLengthUsed < m_ash.cbSrcLength) {
			nBytes = opus_encode((OpusEncoder*)m_state, (short*)(m_ash.pbSrc + m_ash.cbSrcLengthUsed), m_frame_size / 2, pDst, OPUS_MAX_PAYLOAD);
			if (nBytes > 0) {
				int res = opus_repacketizer_cat((OpusRepacketizer*)m_rp, pDst, nBytes);
				if (res < 0) break; /// only 120 ms
				pDst += nBytes;
			}
			m_ash.cbSrcLengthUsed += m_frame_size;
		}
		m_ash.cbDstLengthUsed = opus_repacketizer_out((OpusRepacketizer*)m_rp, m_ash.pbDst, OPUS_MAX_PAYLOAD);
	} else {
		nBytes = opus_decode((OpusDecoder*)m_state, (unsigned char*)m_ash.pbSrc, m_ash.cbSrcLength, (short*)m_ash.pbDst, m_SIZE / 2, 0);
		if (nBytes >= 0) {
			m_ash.cbSrcLengthUsed = m_ash.cbSrcLength;
			m_ash.cbDstLengthUsed = nBytes * 2;
		}
	}
	return 0;
}

const int opus_quality2bitrate[11] =
{
	6000, 6000, 8000, 10800, 12800, 16800, 20800, 24000, 28000, 34000, 44000
};

void VS_OpusAudioCodec::SetQuality(int quality)
{
	if (m_valid && IsCoder()) {
		int bitrate = opus_quality2bitrate[quality];
		if (m_samplerate >= 48000) bitrate = bitrate * 9 / 4;
		else if (m_samplerate >= 24000) bitrate = bitrate * 9 / 8;
		int ret = opus_encoder_ctl((OpusEncoder*)m_state, OPUS_SET_BITRATE(bitrate));
	}
}

void VS_OpusAudioCodec::SetComplexity(int complexity)
{
	if (m_valid && IsCoder()) {
		int ret = opus_encoder_ctl((OpusEncoder*)m_state, OPUS_SET_COMPLEXITY(complexity));
	}
}
