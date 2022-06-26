
#include "trueconf_audio_encoder_factory.h"
#include "trueconf_audio_encoder.h"
#include "trueconf_callback_defines.h"

#include "rtc_base/string_to_number.h"

using namespace tc_webrtc_api;

namespace tc_webrtc_api
{
	rtc::Optional<std::string> GetFormatParameter(const webrtc::SdpAudioFormat& format, const std::string& param)
	{
		auto it = format.parameters.find(param);
		if (it == format.parameters.end()) {
			return rtc::nullopt;
		}
		return it->second;
	}

	template <typename T>
	rtc::Optional<T> GetFormatParameter(const webrtc::SdpAudioFormat& format, const std::string& param)
	{
		return rtc::StringToNumber<T>(tc_webrtc_api::GetFormatParameter(format, param).value_or(""));
	}

}

TrueConfAudioEncoderFactory::TrueConfAudioEncoderFactory(TrueConfAudioEncoderCallback *callback)
	: m_callback(callback)
{
	webrtc::SdpAudioFormat sdp = { "", 0, 0 };
	for (int sr : {16000, 32000, 48000}) {
		sdp = {"opus", sr, 2, {{"minptime", "10"}, {"useinbandfec", "1"}}};
		m_sdpFormats.push_back(sdp);
	}
	for (int sr : {16000, 32000}) {
		sdp = {"ISAC", sr, 1};
		m_sdpFormats.push_back(sdp);
	}
	for (const char* type : {"PCMU", "PCMA"}) {
		sdp = {type, 8000, 1};
		m_sdpFormats.push_back(sdp);
	}
	sdp = {"G722", 8000, 1};
	m_sdpFormats.push_back(sdp);
    for (const auto& it : m_sdpFormats) {
		auto info = QueryAudioEncoder(it);
		webrtc::AudioCodecSpec spec = {it, *info};
        m_supportedCodecs.push_back(spec);
	}
}

TrueConfAudioEncoderFactory::~TrueConfAudioEncoderFactory()
{

}

std::vector<webrtc::AudioCodecSpec> TrueConfAudioEncoderFactory::GetSupportedEncoders()
{
	return m_supportedCodecs;
}

rtc::Optional<webrtc::AudioCodecInfo> TrueConfAudioEncoderFactory::QueryAudioEncoder(const webrtc::SdpAudioFormat & format)
{
    int minBitrate(0);
	int maxBitrate(0);
    int maxAverageBitrate(0);
	size_t numCh(1);
	bool noiseComfort(false);
	bool networkAdaptation(false);
	int sr(format.clockrate_hz);
    if (STR_CASE_CMP(format.name.c_str(), "opus") == 0) {
		if (format.clockrate_hz != 48000 &&
			format.clockrate_hz != 32000 &&
			format.clockrate_hz != 16000) {
			return rtc::nullopt;
		}
        if (format.num_channels != 2) {
			return rtc::nullopt;
		}
		int maxPlaybackRate = 48000;
		minBitrate = 6000;
		maxBitrate = 510000;
		noiseComfort = false;
		networkAdaptation = true;
		{
			const auto param = GetFormatParameter(format, "stereo");
			if (param == "1") {
				numCh = 2;
			}
		}
        {
			const auto param = GetFormatParameter<int>(format, "maxplaybackrate");
			if (param && *param >= 8000) {
				int hz = static_cast<int>(*param);
				maxPlaybackRate = std::min(hz, maxPlaybackRate);
			}
		}
		{
			maxAverageBitrate = (maxPlaybackRate <= 8000) ? 12000 * numCh : ((maxPlaybackRate <= 16000) ? 20000 * numCh : 32000 * numCh);
			const auto param = GetFormatParameter<int>(format, "maxaveragebitrate");
            if (param) {
				int bitrate = static_cast<int>(*param);
				maxAverageBitrate = std::max(minBitrate, std::min(bitrate, maxBitrate));
            }
		}
    }
	else if (STR_CASE_CMP(format.name.c_str(), "ISAC") == 0) {
		if (format.clockrate_hz != 16000 &&
			format.clockrate_hz != 32000) {
			return rtc::nullopt;
		}
        if (format.num_channels != 1) {
			return rtc::nullopt;
		}
		minBitrate = 10000;
		maxBitrate = (format.clockrate_hz == 16000) ? 32000 : 56000;
		maxAverageBitrate = maxBitrate;
		noiseComfort = false;
		networkAdaptation = false;
    }
	else if (STR_CASE_CMP(format.name.c_str(), "g722") == 0) {
		if (format.clockrate_hz != 8000) {
			return rtc::nullopt;
		}
        if (format.num_channels < 1) {
			return rtc::nullopt;
		}
		minBitrate = 64000 * format.num_channels;
		maxBitrate = minBitrate;
		maxAverageBitrate = maxBitrate;
		numCh = format.num_channels;
		noiseComfort = false;
		networkAdaptation = false;
		sr = 16000;
    }
	else if (STR_CASE_CMP(format.name.c_str(), "PCMU") == 0 || STR_CASE_CMP(format.name.c_str(), "PCMA") == 0) {
		if (format.clockrate_hz != 8000) {
			return rtc::nullopt;
		}
        if (format.num_channels < 1) {
			return rtc::nullopt;
		}
		minBitrate = 64000 * format.num_channels;
		maxBitrate = minBitrate;
		maxAverageBitrate = maxBitrate;
		numCh = format.num_channels;
		noiseComfort = false;
		networkAdaptation = false;
	}
    if (maxAverageBitrate > 0) {
		webrtc::AudioCodecInfo info(sr, numCh, maxAverageBitrate, minBitrate, maxBitrate);
		info.allow_comfort_noise = noiseComfort;
		info.supports_network_adaption = networkAdaptation;
		return info;
	}
	return rtc::nullopt;
}

std::unique_ptr<webrtc::AudioEncoder> TrueConfAudioEncoderFactory::MakeAudioEncoder(int payload_type, const webrtc::SdpAudioFormat& format, rtc::Optional<webrtc::AudioCodecPairId> codec_pair_id)
{
	std::unique_ptr<webrtc::AudioEncoder> encoder;
	for (const auto & it : m_sdpFormats) {
		if (it.Matches(format)) {
            auto info = QueryAudioEncoder(format);
            if (info != rtc::nullopt) {
				encoder = std::make_unique<TrueConfAudioEncoder>(*info, payload_type, format.name);
				m_callback->OnUpdateSendAudioPayload(format.name, info->sample_rate_hz, static_cast<int>(info->num_channels));
			}
		}
	}
	return encoder;
}