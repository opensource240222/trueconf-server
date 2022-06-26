
#ifndef WEBRTC_API_TRUECONF_AUDIOENCODER_H_
#define WEBRTC_API_TRUECONF_AUDIOENCODER_H_

#include "api/audio_codecs/audio_encoder.h"
#include "api/audio_codecs/audio_format.h"

#include "common_types.h"

namespace tc_webrtc_api
{

	class TrueConfAudioEncoder : public webrtc::AudioEncoder
	{

	public:

		TrueConfAudioEncoder(const webrtc::AudioCodecInfo& info, int payloadType, const std::string & payloadName);
		~TrueConfAudioEncoder() final;
		int SampleRateHz() const override;
		size_t NumChannels() const override;
		int RtpTimestampRateHz() const override;
		size_t Num10MsFramesInNextPacket() const override;
		size_t Max10MsFramesInAPacket() const override;
		int GetTargetBitrate() const override;
		void Reset() override;

	protected:

		EncodedInfo EncodeImpl(uint32_t rtp_timestamp,
								rtc::ArrayView<const int16_t> audio,
								rtc::Buffer* encoded) override;

	private:

		webrtc::AudioCodecInfo m_codecInfo = {8000, 1, 0};
		webrtc::AudioEncoder::CodecType m_codecType;
		int m_codecPayload;

	};

}

#endif /* WEBRTC_API_TRUECONF_AUDIOENCODER_H_ */