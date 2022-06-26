
#include "trueconf_media_engine.h"
#include "trueconf_video_capturer.h"
#include "trueconf_video_encoder_factory.h"
#include "trueconf_video_decoder_factory.h"
#include "trueconf_audio_device_module.h"
#include "trueconf_audio_encoder_factory.h"
#include "trueconf_audio_decoder_factory.h"
#include "trueconf_video_frame_buffer.h"

#include "rtc_base/refcountedobject.h"

using namespace tc_webrtc_api;

TrueConfMediaEngine::TrueConfMediaEngine(const char *peerName)
{
	m_peerName = peerName;
	m_plAudioSamplerate = 16000;
	m_numChannels = 1;
}

TrueConfVideoCapturer* TrueConfMediaEngine::CreateVideoCapturer(bool screenCapturer)
{
	return new TrueConfVideoCapturer(this, screenCapturer);
}

std::unique_ptr<webrtc::VideoEncoderFactory> TrueConfMediaEngine::CreateVideoEncoderFactory()
{
	return std::make_unique<TrueConfVideoEncoderFactory>(this);
}

std::unique_ptr<webrtc::VideoDecoderFactory> TrueConfMediaEngine::CreateVideoDecoderFactory()
{
	return std::make_unique<TrueConfVideoDecoderFactory>(this);
}

rtc::scoped_refptr<webrtc::AudioDeviceModule> TrueConfMediaEngine::CreateAudioDeviceModule()
{
	return rtc::scoped_refptr<webrtc::AudioDeviceModule>(new rtc::RefCountedObject<TrueConfAudioDeviceModule>(this));
}

rtc::scoped_refptr<webrtc::AudioEncoderFactory> TrueConfMediaEngine::CreateAudioEncoderFactory()
{
	return rtc::scoped_refptr<webrtc::AudioEncoderFactory>(new rtc::RefCountedObject<TrueConfAudioEncoderFactory>(this));
}

rtc::scoped_refptr<webrtc::AudioDecoderFactory> TrueConfMediaEngine::CreateAudioDecoderFactory()
{
	return rtc::scoped_refptr<webrtc::AudioDecoderFactory>(new rtc::RefCountedObject<TrueConfAudioDecoderFactory>(this));
}

TrueConfMediaEngine::~TrueConfMediaEngine()
{

}

void TrueConfMediaEngine::OnPeerConnectionDead()
{
	firePeerConnectionDead();
}

void TrueConfMediaEngine::OnRequestKeyFrame()
{
	fireSendRequestKeyFrame();
}

void TrueConfMediaEngine::OnSetRates(uint32_t bitrate, uint32_t framerate)
{
	fireSetRates(m_peerName.c_str(), bitrate, framerate);
}

void TrueConfMediaEngine::OnUpdateSendVideoPayload(std::string plName, uint8_t plType, uint16_t width, uint16_t height)
{
	fireUpdateSendVideoPayload(m_peerName.c_str(), plName.c_str(), plType, width, height);
}

void TrueConfMediaEngine::OnUpdateReceiveVideoPayload(const std::string &streamid, std::string plName, uint8_t plType, uint16_t width, uint16_t height)
{
	std::lock_guard<std::recursive_mutex> lock(m_lockSimulcastDecoder);
	fireUpdateReceiveVideoPayload(m_peerName.c_str(), streamid.c_str(), plName.c_str(), plType, width, height);
}

void TrueConfMediaEngine::OnReceiveVideoFrame(const std::string &streamid, const uint8_t *frame, int32_t size, bool key, uint32_t timestamp)
{
	std::lock_guard<std::recursive_mutex> lock(m_lockSimulcastDecoder);
	fireReceiveVideoFrame(m_peerName.c_str(), streamid.c_str(), frame, size, key, timestamp);
}

void TrueConfMediaEngine::OnUpdateSendAudioPayload(std::string plName, int plFrequency, int channels)
{
	fireUpdateSendAudioPayload(plName.c_str(), plFrequency);
	m_plAudioSamplerate = plFrequency;
	m_numChannels = channels;
}

void TrueConfMediaEngine::OnUpdateReceiveAudioPayload(std::string plName, int plFrequency)
{
	fireUpdateReceiveAudioPayload(plName.c_str(), plFrequency);
}

void TrueConfMediaEngine::OnReceiveAudioFrame(const uint8_t *frame, uint32_t size, uint32_t timestamp)
{
	fireReceiveAudioFrame(m_peerName.c_str(), frame, static_cast<int>(size), timestamp);
}

void TrueConfMediaEngine::DeliverVideoFrame(const uint8_t *frame, int cmpsize, int width, int height, bool key, int64_t timestamp)
{
	rtc::scoped_refptr<TrueConfCompressFrameBuffer> buffer = new rtc::RefCountedObject<TrueConfCompressFrameBuffer>(frame, width, height, cmpsize, key);
	webrtc::VideoFrame vf(buffer, 0, 0, webrtc::kVideoRotation_0);
	vf.set_timestamp_us(timestamp * rtc::kNumMicrosecsPerMillisec);
	fireDeliverVideoFrame(vf);
}

void TrueConfMediaEngine::BitrateRequest(uint32_t bitrate)
{
	if (bitrate > 0) {
		fireReceiveRequestBitrate(bitrate);
	}
}

void TrueConfMediaEngine::KeyFrameRequest()
{
	fireReceiveRequestKeyFrame();
}

void TrueConfMediaEngine::DeliverAudioFrame(const uint8_t* audio, int len, int samples)
{
	fireDeliverAudioFrame(audio, len, samples, m_plAudioSamplerate, m_numChannels);
}