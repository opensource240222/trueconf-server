#include "VS_SpeexAudioCodec.h"

#include <speex/speex.h>
#include <speex/speex_callbacks.h>
#include <speex/speex_preprocess.h>

#include <math.h>

VS_SpeexAudioCodec::VS_SpeexAudioCodec(uint32_t tag, bool coder):AudioCodec(tag, coder, 640)
{
	m_state = 0;
	m_bits = new SpeexBits;
}

VS_SpeexAudioCodec::~VS_SpeexAudioCodec()
{
	Release();
	if (m_bits) delete m_bits; m_bits = 0;
}

int VS_SpeexAudioCodec::Init(WAVEFORMATEX *in)
{
	Release();

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
	m_cdcfmt->nBlockAlign = 42;
	m_cdcfmt->cbSize = 0;
	m_cdcfmt->nAvgBytesPerSec = 2000;

	speex_bits_init(m_bits);

	spx_int32_t tmp;
	const SpeexMode* sp_mode = in->nSamplesPerSec < 16000 ? &speex_nb_mode : in->nSamplesPerSec < 32000 ? &speex_wb_mode : &speex_uwb_mode;
	if (IsCoder()) {
		m_state = speex_encoder_init(sp_mode);
		tmp = 8;
		speex_encoder_ctl(m_state, SPEEX_SET_QUALITY, &tmp);
		tmp = 4;
		speex_encoder_ctl(m_state, SPEEX_SET_COMPLEXITY, &tmp);
		speex_encoder_ctl(m_state, SPEEX_GET_FRAME_SIZE,&m_frame_size);
		m_frame_size*=2;
		m_GRAN = m_frame_size;
		tmp = 1;
		speex_encoder_ctl(m_state, SPEEX_SET_VAD,&tmp);
		//speex_encoder_ctl(m_state, SPEEX_SET_DTX,&tmp);
	} else {
		m_state = speex_decoder_init(sp_mode);
		tmp = 1;
		speex_decoder_ctl(m_state, SPEEX_SET_ENH, &tmp);
		speex_decoder_ctl(m_state, SPEEX_GET_FRAME_SIZE, &m_frame_size);
		m_frame_size*=2;
		m_GRAN = 1; // align to byte boundary
	}
	m_valid = true;
	return 0;
}


void VS_SpeexAudioCodec::Release()
{
	if (m_valid) {
		free(m_inBuff); m_inBuff = 0; m_DataPointer = 0;
		free(m_outBuff); m_outBuff = 0;
		free(m_cdcfmt); m_cdcfmt = 0;
		free(m_pcmfmt); m_pcmfmt = 0;
		memset(&m_ash, 0, sizeof(ACMSTREAMHEADER));

		speex_bits_destroy(m_bits);
		if (IsCoder()) speex_encoder_destroy(m_state);
		else speex_decoder_destroy(m_state);
		m_state = 0;
		m_valid = false;
	}
}


int VS_SpeexAudioCodec::ConvertFunction()
{
	int status = 0, nBytes = 0;
	m_ash.cbSrcLengthUsed = 0;
	m_ash.cbDstLengthUsed = 0;
	if (IsCoder()) {
		speex_bits_reset(m_bits);
		while (m_ash.cbSrcLengthUsed < m_ash.cbSrcLength) {
			status = speex_encode_int(m_state, (short*)(m_ash.pbSrc + m_ash.cbSrcLengthUsed), m_bits);
			m_ash.cbSrcLengthUsed += m_frame_size;
		}
		m_ash.cbDstLengthUsed = speex_bits_write(m_bits, (char*)m_ash.pbDst, 1000);
	}
	else {
		speex_bits_reset(m_bits);
		speex_bits_read_from(m_bits, (char*)m_ash.pbSrc, m_ash.cbSrcLength);
		while (speex_decode_int(m_state, m_bits, (short*)(m_ash.pbDst + m_ash.cbDstLengthUsed))==0)
			m_ash.cbDstLengthUsed += m_frame_size;
		m_ash.cbSrcLengthUsed = speex_bits_nbytes(m_bits);
	}
	return 0;
}

void VS_SpeexAudioCodec::SetQuality(int quality)
{
	if (m_valid && IsCoder()) {
		speex_encoder_ctl(m_state, SPEEX_SET_QUALITY, &quality);
	}
}
