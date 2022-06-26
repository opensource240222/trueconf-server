
#include "trueconf_video_encoder.h"
#include "trueconf_video_frame_buffer.h"

#include "modules/video_coding/include/video_error_codes.h"
#include "modules/video_coding/include/video_codec_interface.h"
#include "common_video/libyuv/include/webrtc_libyuv.h"
#include "common_video/h264/h264_bitstream_parser.h"
#include "common_video/h264/h264_common.h"
#include "rtc_base/logging.h"

#include <memory>

using namespace tc_webrtc_api;

TrueConfVideoEncoder::TrueConfVideoEncoder(webrtc::VideoCodecType type)
{

}

TrueConfVideoEncoder::~TrueConfVideoEncoder()
{
	Release();
}

int32_t TrueConfVideoEncoder::RegisterEncodeCompleteCallback(webrtc::EncodedImageCallback* callback)
{
	m_encodedCompleteCallback = callback;
	return WEBRTC_VIDEO_CODEC_OK;
}

int32_t TrueConfVideoEncoder::InitEncode(const webrtc::VideoCodec* codec_settings, int32_t number_of_cores, size_t max_payload_size)
{
	if (codec_settings == nullptr) {
		return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
	}
	m_encodedImage._size = webrtc::CalcBufferSize(webrtc::kI420, codec_settings->width, codec_settings->height);
	m_encodedImage._buffer = new uint8_t[m_encodedImage._size];
	m_encodedImage._completeFrame = true;
	m_picture_id = static_cast<uint16_t>(rand()) & 0x7FFF;
	m_encoderType = codec_settings->codecType;

	std::string plName(PayloadName());

	RTC_LOG_V(rtc::LoggingSeverity::LS_INFO) << "[TrueConf] TrueConfVideoEncoder::InitEncode : plname = "
        << plName.c_str()
		<< ", pltype = "
		<< codec_settings->plType
		<< ", "
		<< codec_settings->width
		<< "x"
		<< codec_settings->height
		<< ", bitrate = "
		<< codec_settings->startBitrate
		<< "kbps, framerate = "
		<< codec_settings->maxFramerate;

	if (m_encoderType == webrtc::kVideoCodecVP8 || m_encoderType == webrtc::kVideoCodecH264) {
		if (m_encoderType == webrtc::kVideoCodecH264) {
			m_h264Parser = new webrtc::H264BitstreamParser();
		}
		fireUpdatePayload(plName, codec_settings->plType, codec_settings->width, codec_settings->height);
		fireSetRates(codec_settings->startBitrate, codec_settings->maxFramerate);
		fireRequestKeyFrame();
		m_inited = true;
		return WEBRTC_VIDEO_CODEC_OK;
	} else {
        return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
	}
}

int32_t TrueConfVideoEncoder::Release()
{
	delete[] m_encodedImage._buffer; m_encodedImage._buffer = nullptr;
	delete m_h264Parser; m_h264Parser = nullptr;
	m_encoderType = webrtc::kVideoCodecUnknown;
	m_inited = false;
	return WEBRTC_VIDEO_CODEC_OK;
}

int32_t TrueConfVideoEncoder::SetChannelParameters(uint32_t packet_loss, int64_t rtt)
{
	return WEBRTC_VIDEO_CODEC_OK;
}

int32_t TrueConfVideoEncoder::SetRates(uint32_t bitrate, uint32_t framerate)
{
	if (!m_inited) {
		return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
	}
	if (bitrate > 0) {

		RTC_LOG_V(rtc::LoggingSeverity::LS_INFO)
					 << "[TrueConf] TrueConfVideoEncoder::SetRates : bitrate = "
					 << bitrate
					 << "kbps, framerate = "
					 << framerate;

		fireSetRates(bitrate, framerate);
	}
	return WEBRTC_VIDEO_CODEC_OK;
}

bool TrueConfVideoEncoder::SupportsNativeHandle() const
{
	return true;
}

const char* TrueConfVideoEncoder::PayloadName() const
{
	if (m_encoderType == webrtc::kVideoCodecVP8) {
		return "VP8";
	}
	else if (m_encoderType == webrtc::kVideoCodecH264) {
		return "H264";
	}
	else {
		return "";
	}
}

const char* TrueConfVideoEncoder::ImplementationName() const
{
	if (m_encoderType == webrtc::kVideoCodecVP8) {
		return "TrueConf VP8";
	}
	else if (m_encoderType == webrtc::kVideoCodecH264) {
		return "TrueConf H264";
	}
	else {
		return "TrueConf Unknown";
	}
}

int32_t TrueConfVideoEncoder::Encode(const webrtc::VideoFrame& frame,
									 const webrtc::CodecSpecificInfo* codec_specific_info,
									 const std::vector<webrtc::FrameType>* frame_types)
{
	webrtc::EncodedImageCallback::Result ret(webrtc::EncodedImageCallback::Result::ERROR_SEND_FAILED);
	if (!m_inited)
		return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
	if (frame.size() == 0)
		return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
	if (m_encodedCompleteCallback == nullptr) {
		return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
	}

	TrueConfCompressFrameBuffer *vb = reinterpret_cast<TrueConfCompressFrameBuffer*>(frame.video_frame_buffer().get());
	int32_t length = vb->CompressSize();

	webrtc::RTPFragmentationHeader frag_info;
	webrtc::CodecSpecificInfo codec_specific;
	m_encodedImage._frameType = (vb->KeyFrame()) ? webrtc::kVideoFrameKey : webrtc::kVideoFrameDelta;
	m_encodedImage._timeStamp = frame.timestamp();
	m_encodedImage.capture_time_ms_ = frame.render_time_ms();
	m_encodedImage.rotation_ = webrtc::kVideoRotation_0;
	m_encodedImage._encodedWidth = frame.width();
	m_encodedImage._encodedHeight = frame.height();
	m_encodedImage.SetMaxTargetBitrate(m_targetReceiveBitrate * 1000);
	if (m_encoderType == webrtc::kVideoCodecVP8) {
		frag_info.VerifyAndAllocateFragmentationHeader((1 << 0) + 1);
		PopulateCodecSpecificVP8(&codec_specific, frame.timestamp());
		frag_info.fragmentationOffset[0] = 0;
		frag_info.fragmentationLength[0] = length;
		frag_info.fragmentationPlType[0] = 0;
		frag_info.fragmentationTimeDiff[0] = 0;
		m_encodedImage._length = length;
		m_encodedImage.qp_ = -1;
		memcpy(m_encodedImage._buffer, vb->GetBuffer(), length);
	}
	else if (m_encoderType == webrtc::kVideoCodecH264) {
		PopulateCodecSpecificH264(&codec_specific, frame.timestamp());
		RtpFragmentizeH264(&m_encodedImage, &frag_info, (const uint8_t*)vb->GetBuffer(), length);
		m_h264Parser->ParseBitstream(m_encodedImage._buffer, m_encodedImage._length);
		m_h264Parser->GetLastSliceQp(&m_encodedImage.qp_);
	}
	ret = m_encodedCompleteCallback->OnEncodedImage(m_encodedImage, &codec_specific, &frag_info);
	if (frame_types && (*frame_types)[0] == webrtc::kVideoFrameKey && m_encodedImage._frameType != webrtc::kVideoFrameKey) {
		RTC_LOG_V(rtc::LoggingSeverity::LS_INFO) << "[TrueConf] request key frame from relay";
		fireRequestKeyFrame();
	}

	return (ret.error == webrtc::EncodedImageCallback::Result::OK) ? WEBRTC_VIDEO_CODEC_OK : WEBRTC_VIDEO_CODEC_ERROR;
}

void TrueConfVideoEncoder::PopulateCodecSpecificVP8(webrtc::CodecSpecificInfo* codec_specific, uint32_t timestamp)
{
	codec_specific->codecType = webrtc::kVideoCodecVP8;
	codec_specific->codec_name = ImplementationName();
	webrtc::CodecSpecificInfoVP8* vp8Info = &(codec_specific->codecSpecific.VP8);
	vp8Info->pictureId = m_picture_id;
	vp8Info->simulcastIdx = 0;
	vp8Info->keyIdx = webrtc::kNoKeyIdx;
	vp8Info->nonReference = false;
	vp8Info->temporalIdx = webrtc::kNoTemporalIdx;
	vp8Info->layerSync = false;
	vp8Info->tl0PicIdx = webrtc::kNoTl0PicIdx;
	m_picture_id = (m_picture_id + 1) & 0x7FFF;
}

void TrueConfVideoEncoder::PopulateCodecSpecificH264(webrtc::CodecSpecificInfo* codec_specific, uint32_t timestamp)
{
	codec_specific->codecType = webrtc::kVideoCodecH264;
	codec_specific->codec_name = ImplementationName();
	codec_specific->codecSpecific.H264.packetization_mode = webrtc::H264PacketizationMode::NonInterleaved;
}

void TrueConfVideoEncoder::RtpFragmentizeH264(webrtc::EncodedImage* encoded_image, webrtc::RTPFragmentationHeader* frag_header, const uint8_t *buffer, int32_t length)
{
	auto nalu_indices = webrtc::H264::FindNaluIndices(buffer, length);
	// Calculate minimum buffer size required to hold encoded data.
	size_t required_size = 0;
	for (const auto& index : nalu_indices) {
		required_size += (index.payload_size + 4);
	}
	if (encoded_image->_size < required_size) {
		encoded_image->_size = required_size;
		delete[] encoded_image->_buffer;
		encoded_image->_buffer = new uint8_t[encoded_image->_size];
	}
	frag_header->VerifyAndAllocateFragmentationHeader(nalu_indices.size());
	size_t frag = 0;
	encoded_image->_length = 0;
	for (const auto& index : nalu_indices) {
		frag_header->fragmentationOffset[frag] = index.payload_start_offset;
		frag_header->fragmentationLength[frag] = index.payload_size;
		encoded_image->_length += (index.payload_size + 4);
		frag++;
	}
	// Copy the entire layer's data (including start codes).
	memcpy(encoded_image->_buffer, buffer, length);
}

void TrueConfVideoEncoder::OnUpdateTargetReceiveBitrate(uint32_t bitrate)
{
	m_targetReceiveBitrate = bitrate;
}