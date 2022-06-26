
#include "trueconf_video_frame_buffer.h"

#include "rtc_base/refcountedobject.h"

#include <string.h>

using namespace tc_webrtc_api;

TrueConfCompressFrameBuffer::TrueConfCompressFrameBuffer(const uint8_t* buffer, int width, int height, int cmpsize, bool key)
	: m_width(width), m_height(height)
{
	m_buffer = new uint8_t[cmpsize];
	::memcpy(m_buffer, buffer, cmpsize);
	m_cmpsize = cmpsize;
	m_keyframe = key;
}

TrueConfCompressFrameBuffer::~TrueConfCompressFrameBuffer()
{
	delete[] m_buffer;
}

uint8_t* TrueConfCompressFrameBuffer::GetBuffer()
{
	return m_buffer;
}

bool TrueConfCompressFrameBuffer::KeyFrame()
{
	return m_keyframe;
}

int32_t TrueConfCompressFrameBuffer::CompressSize()
{
	return m_cmpsize;
}

int TrueConfCompressFrameBuffer::width() const
{
	return m_width;
}

int TrueConfCompressFrameBuffer::height() const
{
	return m_height;
}

rtc::scoped_refptr<webrtc::I420BufferInterface> TrueConfCompressFrameBuffer::ToI420()
{
	return nullptr;
}

webrtc::VideoFrameBuffer::Type TrueConfCompressFrameBuffer::type() const
{
	return webrtc::VideoFrameBuffer::Type::kNative;
}