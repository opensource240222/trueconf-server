
#ifndef WEBRTC_API_TRUECONF_VIDEODECODER_H_
#define WEBRTC_API_TRUECONF_VIDEODECODER_H_

#include "api/video_codecs/video_decoder.h"
#include "rtc_base/sigslot.h"

namespace tc_webrtc_api
{

	class TrueConfVideoDecoder : public webrtc::VideoDecoder, public sigslot::has_slots<>
	{

	public:

		TrueConfVideoDecoder(webrtc::VideoCodecType type, const std::string & id);
		~TrueConfVideoDecoder() final;
		int32_t InitDecode(const webrtc::VideoCodec* codec_settings, int32_t number_of_cores) override;
		int32_t Decode( const webrtc::EncodedImage& input_image,
						bool missing_frames,
						const webrtc::CodecSpecificInfo* codec_specific_info,
						int64_t render_time_ms) override;
		int32_t RegisterDecodeCompleteCallback(webrtc::DecodedImageCallback* callback) override;
		int32_t Release() override;
		bool PrefersLateDecoding() const override;
		const char* ImplementationName() const override;

	public:

		void OnKeyFrameRequest();
		sigslot::signal5<const std::string&, const uint8_t*, int32_t, bool, uint32_t, sigslot::multi_threaded_local> fireReceiveEncodedFrame;
		sigslot::signal5<const std::string&, std::string, uint8_t, uint16_t, uint16_t, sigslot::multi_threaded_local> fireUpdatePayload;

	private:

		const char* PayloadName() const;

	private:

		bool m_keyFrameRequest = false;
		webrtc::EncodedImage m_h264image;
		webrtc::VideoCodecType m_decoderType;
		bool m_inited = false;
		std::string m_id;

	};

}

#endif /* WEBRTC_API_TRUECONF_VIDEODECODER_H_ */