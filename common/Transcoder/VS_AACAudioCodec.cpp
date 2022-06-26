#include <string>

#include "FFAudioCodec.h"

#include "VS_AACAudioCodec.h"

VS_AACAudioCodec::VS_AACAudioCodec(uint32_t tag, bool coder)
	: AudioCodec(tag, coder, 640)
{
	m_Codec = nullptr;
}

VS_AACAudioCodec::~VS_AACAudioCodec()
{
	Release();
}

const int aac_quality2bitrate[11] = {
	8000, 12000, 16000, 22050, 24000, 32000, 44100, 48000, 64000, 88200, 96000
};

int VS_AACAudioCodec::Init(WAVEFORMATEX *in)
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
	m_pcmfmt->nAvgBytesPerSec = m_pcmfmt->nSamplesPerSec * m_pcmfmt->nBlockAlign;
	m_pcmfmt->cbSize = 0;

	m_cdcfmt->wFormatTag = (uint16_t)GetTag();
	m_cdcfmt->nSamplesPerSec = in->nSamplesPerSec;
	m_cdcfmt->wBitsPerSample = 0;
	m_cdcfmt->nChannels = 1;
	m_cdcfmt->nBlockAlign = 120;
	m_cdcfmt->cbSize = 0;
	m_cdcfmt->nAvgBytesPerSec = 4000;

	m_Codec = new FFAudioCodec;

	if (!m_Codec || !m_Codec->Init(in->nSamplesPerSec, IsCoder(), aac_quality2bitrate[8]))
			return -1;

	m_valid = true;
	return res;
}

void VS_AACAudioCodec::Release()
{
	if (m_valid) {
		free(m_inBuff); m_inBuff = NULL; m_DataPointer = NULL;
		free(m_outBuff); m_outBuff = NULL;
		free(m_cdcfmt); m_cdcfmt = NULL;
		free(m_pcmfmt); m_pcmfmt = NULL;
		memset(&m_ash, 0, sizeof(ACMSTREAMHEADER));

		if (m_Codec)
			m_Codec->Release();

		m_Codec = nullptr;

		m_valid = false;
	}
}

int VS_AACAudioCodec::Convert(uint8_t *in, uint8_t *out, uint32_t insize)
{
	if (!m_valid)
		return -1;

	if (IsCoder())
	{
		short* shortInBuff = (short*)in;
		float* floatInBuff = (float*)m_inBuff;

		for (size_t i = 0; i < insize / sizeof(short); i++)
			floatInBuff[i] = float(shortInBuff[i]) / 32767.0f;

		return m_Codec->Convert((char*)floatInBuff, (char*)out, insize / sizeof(short) * sizeof(float));
	}
	else
	{
		int decodedSize = 0;
		short* shortOutBuff = (short*)out;
		float* floatOutBuff = (float*)m_outBuff;

		decodedSize = m_Codec->Convert((char*)in, (char*)floatOutBuff, insize);

		if (decodedSize < 0)
			return decodedSize;

		for (size_t i = 0; i < decodedSize / sizeof(float); i++)
			shortOutBuff[i] = short(floatOutBuff[i] * 32767.0f);

		return (decodedSize / sizeof(float)) * sizeof(short);
	}
}

int VS_AACAudioCodec::ConvertFunction()
{
	return 0;
}

void VS_AACAudioCodec::SetQuality(int quality)
{
	// TODO or not todo?
}
