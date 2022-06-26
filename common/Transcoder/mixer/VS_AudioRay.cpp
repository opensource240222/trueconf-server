#include "VS_AudioRay.h"

VS_AudioRay::VS_AudioRay(const VS_MediaFormat& out)
	: m_Audio(32000 * sizeof(uint16_t) * 4)
{
	m_infmt.SetVideo(640, 360);
	m_outfmt = out;
	m_storageBuffer.reset(new int16_t[out.dwAudioSampleRate * 2]);
}

VS_AudioRay::~VS_AudioRay()
{
}

bool VS_AudioRay::AddAudio(uint8_t* audio, int size)
{
	int len = size;
	uint8_t *pDst = audio;
	if (m_infmt.dwAudioSampleRate != m_outfmt.dwAudioSampleRate) {
		len = (int)((double)m_outfmt.dwAudioSampleRate / m_infmt.dwAudioSampleRate*size*1.1);
		if (m_resampleBuffer.size() < len) {
			m_resampleBuffer.resize(len);
		}
		len = m_resmp.Process(audio, m_resampleBuffer.data(), size, m_infmt.dwAudioSampleRate, m_outfmt.dwAudioSampleRate);
		pDst = m_resampleBuffer.data();
	}

	m_Audio.AddData(pDst, len);

	return true;
}

uint8_t* VS_AudioRay::GetAudio(size_t requestSize)
{
	if (m_Audio.GetDataLen() < requestSize)
		return nullptr;

	if (m_Audio.GetFirstArray().second < requestSize)
		m_Audio.Linearize();

	return (uint8_t*)m_Audio.GetFirstArray().first;
}

void VS_AudioRay::DropAudio(int size)
{
	if (m_Audio.GetDataLen() >= size)
		m_Audio.Discard(size);
}

int16_t* VS_AudioRay::GetStorageBuffer()
{
	return m_storageBuffer.get();
}

bool VS_AudioRay::SetOutputFormat(VS_MediaFormat * out)
{
	if (out) {
		m_outfmt = *out;
		m_storageBuffer.reset(new int16_t[out->dwAudioSampleRate * 2]);
	}
	return true;
}

bool VS_AudioRay::SetInputFormat(const VS_MediaFormat & mf)
{
	m_infmt = mf;
	return true;
}
