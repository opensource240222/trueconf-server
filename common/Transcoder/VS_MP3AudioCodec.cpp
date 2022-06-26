#include <string>

#include "lame/lame.h"

#include "VS_MP3AudioCodec.h"

VS_MP3AudioCodec::VS_MP3AudioCodec(uint32_t tag, bool coder) :AudioCodec(tag, coder, 640)
{
	m_Lame = NULL;
	m_Hip = NULL;
}

VS_MP3AudioCodec::~VS_MP3AudioCodec()
{
	Release();
}

const int mp3_quality2bitrate[11] =
{
	8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112
};

int VS_MP3AudioCodec::Init(WAVEFORMATEX *in)
{
	Release();

	int res = 0;

	m_cdcfmt = (WAVEFORMATEX*)malloc(sizeof(WAVEFORMATEX));
	m_pcmfmt = (WAVEFORMATEX*)malloc(sizeof(WAVEFORMATEX));
	m_inBuff = (uint8_t*)malloc(m_SIZE);
	m_outBuff = (uint8_t*)malloc(m_SIZE);
	m_rightBuff = (uint8_t*)malloc(m_SIZE);
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

	if (IsCoder())
	{
		m_Lame = lame_init();

		if (m_Lame)
		{
			int err;

			lame_set_num_channels(m_Lame, 1);
			lame_set_mode(m_Lame, MPEG_mode::MONO);
			lame_set_in_samplerate(m_Lame, in->nSamplesPerSec);
			lame_set_out_samplerate(m_Lame, in->nSamplesPerSec);
			lame_set_VBR(m_Lame, vbr_mode::vbr_off);

			lame_set_brate(m_Lame, mp3_quality2bitrate[7]);

			err = lame_init_params(m_Lame);

			if (err != 0)
				res = -1;
		}
		else
		{
			res = -1;
		}
	}
	else
	{
		m_Hip = hip_decode_init();

		if (!m_Hip)
		{
			res = -1;
		}
	}

	m_valid = true;
	return res;
}

void VS_MP3AudioCodec::Release()
{
	if (m_valid) {
		free(m_inBuff); m_inBuff = NULL; m_DataPointer = NULL;
		free(m_outBuff); m_outBuff = NULL;
		free(m_cdcfmt); m_cdcfmt = NULL;
		free(m_pcmfmt); m_pcmfmt = NULL;
		free(m_rightBuff); m_rightBuff = NULL;
		memset(&m_ash, 0, sizeof(ACMSTREAMHEADER));

		if (IsCoder())
			lame_close(m_Lame);
		else
			hip_decode_exit(m_Hip);

		m_valid = false;
	}
}

int VS_MP3AudioCodec::Convert(uint8_t *in, uint8_t *out, uint32_t insize)
{
	if (IsCoder())
	{
		int encodedSize = 0;

		encodedSize = lame_encode_buffer(m_Lame, (short*)in, NULL, insize / sizeof(short), m_outBuff, m_SIZE);

		if (encodedSize)
		{
			encodedSize += lame_encode_flush_nogap(m_Lame, m_outBuff + encodedSize, m_SIZE - encodedSize);

			memcpy(out, m_outBuff, encodedSize);
		}

		return encodedSize;
	}
	else
	{
		int decodedSize = 0;

		decodedSize = hip_decode(m_Hip, in, insize, (short*)m_outBuff, (short*)m_rightBuff);

		if (decodedSize)
		{
			memcpy(out, m_outBuff, decodedSize * sizeof(short));
		}

		return decodedSize * sizeof(short);
	}
}

int VS_MP3AudioCodec::ConvertFunction()
{
	return 0;
}

void VS_MP3AudioCodec::SetQuality(int quality)
{
	if (m_valid && IsCoder()) {
		lame_set_brate(m_Lame, mp3_quality2bitrate[quality]);
	}
}
