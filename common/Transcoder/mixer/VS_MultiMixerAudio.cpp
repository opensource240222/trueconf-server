#include "VS_MultiMixerAudio.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "Transcoder/VSVideoUtils.h"

VS_MultiMixerAudio::VS_MultiMixerAudio()
{
	m_outFormat.SetAudio(16000, VS_ACODEC_PCM);
	m_mixBuffer.reset(new int16_t[m_outFormat.dwAudioSampleRate * 2]);
	m_amixer.Init(AMM_SUMMKOEF);
}

VS_MultiMixerAudio::~VS_MultiMixerAudio()
{
}

bool VS_MultiMixerAudio::Add(unsigned char* buff, int size, const std::string &handle)
{
	auto ii = m_mr.find(handle);

	if (ii == m_mr.end())
		return false;

	ii->second->AddAudio(buff, size);

	return true;
}

uint32_t VS_MultiMixerAudio::GetAudio(uint8_t *buff, int32_t lenght)
{
	if (m_mr.empty()) {
		return 0;
	}
	uint32_t getSize(0);
	int16_t* abuff[1024] = { 0 };
	int32_t chunkSize(m_outFormat.dwAudioSampleRate / 1000 * 20);
	while (true) {
		int32_t stream_num(0);
		for (auto &ii : m_mr) {
			int16_t* audio = (int16_t*)ii.second->GetAudio(chunkSize);
			if (audio) {
				abuff[stream_num] = audio;
				stream_num++;
			}
		}
		if (stream_num == 0) {
			break;
		}

		m_amixer.Mix(abuff, stream_num, (int16_t*)(buff + getSize), chunkSize / 2);

		for (auto &ii : m_mr) {
			ii.second->DropAudio(chunkSize);
		}
		getSize += chunkSize;
		if ((getSize + chunkSize) > lenght) {
			break;
		}
	}
	return getSize;
}

uint32_t VS_MultiMixerAudio::GetAudio(const std::string &handle, uint8_t *buff, int32_t lenght)
{
	if (lenght == 0) {
		return 0;
	}
	int16_t *src(m_mixBuffer.get());
	if (!handle.empty()) {
		auto ray = m_mr.find(handle);
		if (ray == m_mr.end()) {
			return 0;
		}
		src = ray->second->GetStorageBuffer();
	}
	memcpy(buff, src, lenght);
	return lenght;
}

uint32_t VS_MultiMixerAudio::PrepareAudio(int32_t lenght)
{
	if (m_mr.empty()) {
		return 0;
	}
	int16_t *in[1024], *out[1024 + 1];
	uint32_t getSizeBytes(0);
	int32_t chunkSizeBytes(m_outFormat.dwAudioSampleRate / 1000 * 10 * 2); /// 10 ms
	while (true) {
		int32_t n_input(0), n_output(0);
		for (auto &it : m_mr) {
			auto ray = it.second;
			in[n_output] = (int16_t*)ray->GetAudio(chunkSizeBytes);
			out[n_output] = ray->GetStorageBuffer() + getSizeBytes / 2;
			if (in[n_output] != nullptr) {
				n_input++;
			}
			else {
				memset(out[n_output], 0, chunkSizeBytes);
			}
			n_output++;
		}
		if (n_input == 0) {
			break;
		}
		out[n_output] = m_mixBuffer.get() + getSizeBytes / 2;
		if (!m_amixer.Mix(in, n_input, out, n_output, chunkSizeBytes / 2)) {
			for (int32_t i = 0; i <= n_output; i++) {
				memset(out[i], 0, chunkSizeBytes);
			}
		}
		for (auto &ii : m_mr) {
			ii.second->DropAudio(chunkSizeBytes);
		}
		getSizeBytes += chunkSizeBytes;
		if ((getSizeBytes + chunkSizeBytes) > lenght) {
			break;
		}
	}
	return getSizeBytes;
}

bool VS_MultiMixerAudio::ChangeSampleRate(int SampleRate)
{
	if (m_outFormat.dwAudioSampleRate != SampleRate) {
		m_outFormat.dwAudioSampleRate = SampleRate;
		for (const auto& ii : m_mr) {
			ii.second->SetOutputFormat(&m_outFormat);
		}
		m_mixBuffer.reset(new int16_t[m_outFormat.dwAudioSampleRate * 2]);
	}
	return true;
}

int32_t VS_MultiMixerAudio::GetSampleRate() const
{
	return m_outFormat.dwAudioSampleRate;
}

std::set<std::string> VS_MultiMixerAudio::GetHandleRays() const
{
	return m_handleRays;
}

bool VS_MultiMixerAudio::AddRay(const std::string &handle, const VS_MediaFormat &in)
{
	if (m_mr.size() >= LIMIT_CUR_STREAMS)
		return false;

	auto it = m_mr.find(handle);

	if (it != m_mr.end())
		return false;

	std::shared_ptr<VS_AudioRay> ray(CreateRay(m_outFormat));

	auto res = m_mr.insert({ handle, ray });

	if (res.second)
	{
		res.first->second->SetInputFormat(in);
	}

	m_handleRays.insert(handle);

	return res.second;
}

bool VS_MultiMixerAudio::DeleteRay(const std::string &handle)
{
	auto ii = m_mr.find(handle);
	if (ii == m_mr.end())
		return false;
	m_mr.erase(ii);
	m_handleRays.erase(handle);
	return true;
}

VS_AudioRay* VS_MultiMixerAudio::CreateRay(const VS_MediaFormat& out)
{
	return new VS_AudioRay(out);
}
