
#ifndef WEBRTC_API_TRUECONF_VIDEOENCODER_H_
#define WEBRTC_API_TRUECONF_VIDEOENCODER_H_

#include "api/video_codecs/video_encoder.h"
#include "rtc_base/sigslot.h"
#include "common_video/include/video_frame.h"

namespace webrtc
{
	class RTPFragmentationHeader;
	class H264BitstreamParser;
}

namespace tc_webrtc_api {

	class TrueConfVideoEncoder : public webrtc::VideoEncoder, public sigslot::has_slots<>
	{

	public:

		TrueConfVideoEncoder(webrtc::VideoCodecType type);
		~TrueConfVideoEncoder() final;
		int32_t InitEncode(const webrtc::VideoCodec* codec_settings, int32_t number_of_cores, size_t max_payload_size) override;
		int32_t RegisterEncodeCompleteCallback(webrtc::EncodedImageCallback* callback) override;
		int32_t Release() override;
		int32_t Encode(const webrtc::VideoFrame& frame, const webrtc::CodecSpecificInfo* codec_specific_info, const std::vector<webrtc::FrameType>* frame_types) override;
		int32_t SetChannelParameters(uint32_t packet_loss, int64_t rtt) override;
		int32_t SetRates(uint32_t bitrate, uint32_t framerate) override;
		bool SupportsNativeHandle() const override;
		const char* ImplementationName() const override;

	public:

		sigslot::signal2<uint32_t, uint32_t, sigslot::multi_threaded_local> fireSetRates;
		sigslot::signal0<sigslot::multi_threaded_local> fireRequestKeyFrame;
		sigslot::signal4<std::string, uint8_t, uint16_t, uint16_t, sigslot::multi_threaded_local> fireUpdatePayload;

	public:

		void OnUpdateTargetReceiveBitrate(uint32_t bitrate);

	private:

		const char* PayloadName() const;
		void PopulateCodecSpecificVP8(webrtc::CodecSpecificInfo* codec_specific, uint32_t timestamp);
		void PopulateCodecSpecificH264(webrtc::CodecSpecificInfo* codec_specific, uint32_t timestamp);
		void RtpFragmentizeH264(webrtc::EncodedImage* encoded_image, webrtc::RTPFragmentationHeader* frag_header, const uint8_t *buffer, int32_t length);

	private:

		webrtc::EncodedImageCallback* m_encodedCompleteCallback = nullptr;
		webrtc::H264BitstreamParser *m_h264Parser = nullptr;
		uint32_t m_targetReceiveBitrate = 512;
		bool m_inited = false;
		webrtc::VideoCodecType m_encoderType = webrtc::VideoCodecType::kVideoCodecUnknown;
		uint16_t m_picture_id = 0;
		webrtc::EncodedImage m_encodedImage;

	};

}

#endif /* WEBRTC_API_TRUECONF_VIDEOENCODER_H_ */