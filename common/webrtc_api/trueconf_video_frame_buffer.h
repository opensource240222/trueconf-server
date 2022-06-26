
#ifndef WEBRTC_API_TRUECONF_VIDEOFRAMEBUFFER_H_
#define WEBRTC_API_TRUECONF_VIDEOFRAMEBUFFER_H_

#include "api/video/video_frame_buffer.h"

namespace tc_webrtc_api
{

	class TrueConfCompressFrameBuffer : public webrtc::VideoFrameBuffer
	{
	public:
		TrueConfCompressFrameBuffer(const uint8_t* buffer, int width, int height, int cmpsize, bool key);
		~TrueConfCompressFrameBuffer() override;
		webrtc::VideoFrameBuffer::Type type() const override;
		int width() const override;
		int height() const override;
		rtc::scoped_refptr<webrtc::I420BufferInterface> ToI420() override;
	public:
		uint8_t* GetBuffer();
		bool KeyFrame();
		int32_t CompressSize();
	private:
		uint8_t *m_buffer;
		int m_width;
		int m_height;
		bool m_keyframe;
		int32_t m_cmpsize;
	};

}

#endif /* WEBRTC_API_TRUECONF_VIDEOFRAMEBUFFER_H_ */