
#include "trueconf_audio_decoder_factory.h"
#include "trueconf_audio_decoder.h"

using namespace tc_webrtc_api;

TrueConfAudioDecoderFactory::TrueConfAudioDecoderFactory(TrueConfAudioDecoderCallback *callback)
	: m_callback(callback)
{
	for (int sr : {16000, 32000, 48000}) {
		webrtc::AudioCodecInfo info{ sr, 2, 64000, 6000, 510000 };
		info.allow_comfort_noise = false;
		info.supports_network_adaption = true;
		webrtc::AudioCodecSpec spec = { { "opus", sr, 2, { { "minptime", "10" }, { "useinbandfec", "1" } } }, info };
		m_supportedSpecs.push_back(spec);
	}
	{
		webrtc::AudioCodecInfo info{ 16000, 1, 32000, 10000, 56000 };
		webrtc::AudioCodecSpec spec = { { "ISAC", 16000, 1 }, info };
		m_supportedSpecs.push_back(spec);
	}
	{
		webrtc::AudioCodecInfo info{ 32000, 1, 56000, 10000, 56000 };
		webrtc::AudioCodecSpec spec = { { "ISAC", 32000, 1 }, info };
		m_supportedSpecs.push_back(spec);
	}
	m_supportedSpecs.push_back({ { "G722", 8000, 1 }, { 16000, 1, 64000 } });
	for (const char* type : {"PCMU", "PCMA"}) {
		m_supportedSpecs.push_back({{type, 8000, 1}, { 8000, 1, 64000 } });
	}
}

TrueConfAudioDecoderFactory::~TrueConfAudioDecoderFactory()
{

}

std::vector<webrtc::AudioCodecSpec> TrueConfAudioDecoderFactory::GetSupportedDecoders()
{
	return m_supportedSpecs;
}

bool TrueConfAudioDecoderFactory::IsSupportedDecoder(const webrtc::SdpAudioFormat & format)
{
	for (const auto& it : m_supportedSpecs) {
		if (STR_CASE_CMP(it.format.name.c_str(), format.name.c_str()) == 0) {
			return true;
		}
	}
	return false;
}

std::unique_ptr<webrtc::AudioDecoder> TrueConfAudioDecoderFactory::MakeAudioDecoder(const webrtc::SdpAudioFormat& format, rtc::Optional<webrtc::AudioCodecPairId> codec_pair_id)
{
	std::unique_ptr<webrtc::AudioDecoder> decoder;
	for (const auto& it : m_supportedSpecs) {
		if (STR_CASE_CMP(it.format.name.c_str(), format.name.c_str()) == 0) {
			decoder = std::make_unique<TrueConfAudioDecoder>(format.name, format.clockrate_hz, static_cast<int>(format.num_channels), m_callback);
		}
	}
	return decoder;
}