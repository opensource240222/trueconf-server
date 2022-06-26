
#include "trueconf_video_decoder.h"

#include "modules/video_coding/include/video_error_codes.h"
#include "common_video/h264/h264_common.h"
#include "rtc_base/logging.h"

using namespace tc_webrtc_api;

TrueConfVideoDecoder::TrueConfVideoDecoder(webrtc::VideoCodecType type, const std::string& id) : m_id(id)
{

}

TrueConfVideoDecoder::~TrueConfVideoDecoder()
{
	Release();
}

int32_t TrueConfVideoDecoder::InitDecode(const webrtc::VideoCodec* codec_settings, int32_t number_of_cores)
{
	Release();

	m_decoderType = codec_settings->codecType;
	std::string plName(PayloadName());

	RTC_LOG_V(rtc::LoggingSeverity::LS_INFO) << "[TrueConf] TrueConfVideoDecoder::InitDecode ["
		<< "0"
		<< "] : plname = "
		<< plName.c_str()
		<< ", pltype = "
		<< codec_settings->plType
		<< ", "
		<< codec_settings->width
		<< "x"
		<< codec_settings->height;

	if (m_decoderType == webrtc::kVideoCodecVP8 || m_decoderType == webrtc::kVideoCodecH264) {
		fireUpdatePayload(m_id, plName, codec_settings->plType, codec_settings->width, codec_settings->height);
		m_inited = true;
		return WEBRTC_VIDEO_CODEC_OK;
	} else {
        return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
	}
}

int32_t TrueConfVideoDecoder::Decode(const webrtc::EncodedImage& input_image, bool missing_frames,
									 const webrtc::CodecSpecificInfo* codec_specific_info,
									 int64_t render_time_ms)
{
	if (!m_inited) {
		return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
	}
	int32_t ret(WEBRTC_VIDEO_CODEC_OK);
	if (m_decoderType == webrtc::kVideoCodecH264) {
		/// remove AUD nals
		if (m_h264image._size < input_image._length) {
			delete[] m_h264image._buffer;
			m_h264image._buffer = new uint8_t[input_image._length];
			m_h264image._size = input_image._length;
		}
		m_h264image._length = 0;
		auto nalu_indices = webrtc::H264::FindNaluIndices(input_image._buffer, input_image._length);
		for (const auto& index : nalu_indices) {
			uint8_t typeNAL = (input_image._buffer[index.payload_start_offset] & 0x1f);
			if (typeNAL != 9) {
				size_t s = index.payload_size + (index.payload_start_offset - index.start_offset);
				memcpy(m_h264image._buffer + m_h264image._length, input_image._buffer + index.start_offset, s);
				m_h264image._length += s;
			}
		}
		fireReceiveEncodedFrame(m_id, m_h264image._buffer, static_cast<int32_t>(m_h264image._length), input_image._frameType == webrtc::kVideoFrameKey, input_image._timeStamp);
	}
	else {
		fireReceiveEncodedFrame(m_id, input_image._buffer, static_cast<int32_t>(input_image._length), input_image._frameType == webrtc::kVideoFrameKey, input_image._timeStamp);
	}
	if (m_keyFrameRequest) {
		RTC_LOG_V(rtc::LoggingSeverity::LS_INFO) << "[TrueConf] request key frame from peerconnection";
		m_keyFrameRequest = false;
		ret = WEBRTC_VIDEO_CODEC_OK_REQUEST_KEYFRAME;
	}

#ifdef TRUECONF_VIDEODECODER_TEST
	ret = m_webrtcDecoder->Decode(input_image, missing_frames, fragmentation, codec_specific_info, render_time_ms);
#endif

	return ret;
}

int32_t TrueConfVideoDecoder::RegisterDecodeCompleteCallback(webrtc::DecodedImageCallback* callback)
{
	return WEBRTC_VIDEO_CODEC_OK;
}

int32_t TrueConfVideoDecoder::Release()
{
	m_inited = false;
	return WEBRTC_VIDEO_CODEC_OK;
}

bool TrueConfVideoDecoder::PrefersLateDecoding() const
{
	return false;
}

const char* TrueConfVideoDecoder::PayloadName() const
{
	if (m_decoderType == webrtc::kVideoCodecVP8) {
		return "VP8";
	}
	else if (m_decoderType == webrtc::kVideoCodecH264) {
		return "H264";
	}
	else {
		return "";
	}
}

const char* TrueConfVideoDecoder::ImplementationName() const
{
	if (m_decoderType == webrtc::kVideoCodecVP8) {
		return "TrueConf VP8";
	}
	else if (m_decoderType == webrtc::kVideoCodecH264) {
		return "TrueConf H264";
	}
	else {
		return "TrueConf Unknown";
	}
}

void TrueConfVideoDecoder::OnKeyFrameRequest()
{
	m_keyFrameRequest = true;
}