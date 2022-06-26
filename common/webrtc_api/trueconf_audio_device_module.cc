
#include "trueconf_audio_device_module.h"
#include "trueconf_callback_defines.h"

#include "api/audio/audio_frame.h"

using namespace tc_webrtc_api;

TrueConfAudioDeviceModule::TrueConfAudioDeviceModule(TrueConfAudioDeviceModuleCallback *callback)
	: m_callback(callback)
{
	m_frameBuffer = new int16_t[webrtc::AudioFrame::kMaxDataSizeSamples];
}

TrueConfAudioDeviceModule::~TrueConfAudioDeviceModule()
{
	delete[] m_frameBuffer;
}

void TrueConfAudioDeviceModule::OnDeliverAudioFrame(const uint8_t *audio, int length, int samples, int samplerate, int channels)
{
	if (m_initialized && m_startRecord && m_transportCallback) {
		uint32_t lvl(0);
		m_frameBuffer[0] = static_cast<int16_t>(length);
		memcpy(m_frameBuffer + 1, audio, length);
		m_transportCallback->RecordedDataIsAvailable(m_frameBuffer, samples, 2 * channels, channels, samplerate, 0, 0, 0, false, lvl);
	}
}

int32_t TrueConfAudioDeviceModule::ActiveAudioLayer(AudioLayer* audioLayer) const
{
	*audioLayer = kDummyAudio;
	return 0;
}

int32_t TrueConfAudioDeviceModule::RegisterAudioCallback(webrtc::AudioTransport* audioCallback)
{
	m_transportCallback = audioCallback;
	return 0;
}

int32_t TrueConfAudioDeviceModule::Init()
{
	if (m_initialized) {
		return 0;
	}
	m_initialized = true;

#ifdef TRUECONF_ADM_TEST
	g_testADM = new EmulateAudioCapturer(this);
#endif

	return 0;
}

int32_t TrueConfAudioDeviceModule::Terminate()
{
	if (!m_initialized) {
		return 0;
	}
	m_startPlayout = false;
	m_startRecord = false;
	m_initRecord = false;
	m_initPlayout = false;
	m_initialized = false;

#ifdef TRUECONF_ADM_TEST
	delete g_testADM; g_testADM = nullptr;
#endif

	return 0;
}

bool TrueConfAudioDeviceModule::Initialized() const
{
	return m_initialized;
}

int16_t TrueConfAudioDeviceModule::PlayoutDevices()
{
	return 0;
}

int16_t TrueConfAudioDeviceModule::RecordingDevices()
{
	return 0;
}

int32_t TrueConfAudioDeviceModule::PlayoutDeviceName(uint16_t index, char name[webrtc::kAdmMaxDeviceNameSize], char guid[webrtc::kAdmMaxGuidSize])
{
	return 0;
}

int32_t TrueConfAudioDeviceModule::RecordingDeviceName(uint16_t index, char name[webrtc::kAdmMaxDeviceNameSize], char guid[webrtc::kAdmMaxGuidSize])
{
	return 0;
}

int32_t TrueConfAudioDeviceModule::SetPlayoutDevice(uint16_t index)
{
	return 0;
}

int32_t TrueConfAudioDeviceModule::SetPlayoutDevice(WindowsDeviceType device)
{
	return 0;
}

int32_t TrueConfAudioDeviceModule::SetRecordingDevice(uint16_t index)
{
	return 0;
}

int32_t TrueConfAudioDeviceModule::SetRecordingDevice(WindowsDeviceType device)
{
	return 0;
}

int32_t TrueConfAudioDeviceModule::PlayoutIsAvailable(bool* available)
{
	*available = true;
	return 0;
}

int32_t TrueConfAudioDeviceModule::InitPlayout()
{
	if (!m_initialized) {
		return -1;
	}
	m_initPlayout = true;
	return 0;
}

bool TrueConfAudioDeviceModule::PlayoutIsInitialized() const
{
	return m_initPlayout;
}

int32_t TrueConfAudioDeviceModule::RecordingIsAvailable(bool* available)
{
	*available = true;
	return 0;
}

int32_t TrueConfAudioDeviceModule::InitRecording()
{
	if (!m_initialized) {
		return -1;
	}
	m_initRecord = true;
	return 0;
}

bool TrueConfAudioDeviceModule::RecordingIsInitialized() const
{
	return m_initRecord;
}

int32_t TrueConfAudioDeviceModule::StartPlayout()
{
	if (!m_initPlayout) {
		return -1;
	}
	m_startPlayout = true;
	return 0;
}

int32_t TrueConfAudioDeviceModule::StopPlayout()
{
	m_startPlayout = false;
	return 0;
}

bool TrueConfAudioDeviceModule::Playing() const
{
	return m_startPlayout;
}

int32_t TrueConfAudioDeviceModule::StartRecording()
{
	if (!m_initRecord) {
		return -1;
	}
	m_callback->fireDeliverAudioFrame.connect(this, &TrueConfAudioDeviceModule::OnDeliverAudioFrame);
	m_startRecord = true;
	return 0;
}

int32_t TrueConfAudioDeviceModule::StopRecording()
{
	m_callback->fireDeliverAudioFrame.disconnect_all();
	m_startRecord = false;
	return 0;
}

bool TrueConfAudioDeviceModule::Recording() const
{
	return m_startRecord;
}

int32_t TrueConfAudioDeviceModule::InitSpeaker()
{
	return 0;
}

bool TrueConfAudioDeviceModule::SpeakerIsInitialized() const
{
	return true;
}

int32_t TrueConfAudioDeviceModule::InitMicrophone()
{
	return 0;
}

bool TrueConfAudioDeviceModule::MicrophoneIsInitialized() const
{
	return false;
}

int32_t TrueConfAudioDeviceModule::SpeakerVolumeIsAvailable(bool* available)
{
	return 0;
}

int32_t TrueConfAudioDeviceModule::SetSpeakerVolume(uint32_t volume)
{
	return 0;
}

int32_t TrueConfAudioDeviceModule::SpeakerVolume(uint32_t* volume) const
{
	return 0;
}

int32_t TrueConfAudioDeviceModule::MaxSpeakerVolume(uint32_t* maxVolume) const
{
	return 0;
}

int32_t TrueConfAudioDeviceModule::MinSpeakerVolume(uint32_t* minVolume) const
{
	return 0;
}

int32_t TrueConfAudioDeviceModule::MicrophoneVolumeIsAvailable(bool* available)
{
	return 0;
}

int32_t TrueConfAudioDeviceModule::SetMicrophoneVolume(uint32_t volume)
{
	return 0;
}

int32_t TrueConfAudioDeviceModule::MicrophoneVolume(uint32_t* volume) const
{
	return 0;
}

int32_t TrueConfAudioDeviceModule::MaxMicrophoneVolume(uint32_t* maxVolume) const
{
	return -1;
}

int32_t TrueConfAudioDeviceModule::MinMicrophoneVolume(uint32_t* minVolume) const
{
	return -1;
}

int32_t TrueConfAudioDeviceModule::SpeakerMuteIsAvailable(bool* available)
{
	return 0;
}

int32_t TrueConfAudioDeviceModule::SetSpeakerMute(bool enable)
{
	return 0;
}

int32_t TrueConfAudioDeviceModule::SpeakerMute(bool* enabled) const
{
	return 0;
}

int32_t TrueConfAudioDeviceModule::MicrophoneMuteIsAvailable(bool* available)
{
	return 0;
}

int32_t TrueConfAudioDeviceModule::SetMicrophoneMute(bool enable)
{
	return 0;
}

int32_t TrueConfAudioDeviceModule::MicrophoneMute(bool* enabled) const
{
	return 0;
}

int32_t TrueConfAudioDeviceModule::StereoPlayoutIsAvailable(bool* available) const
{
	return 0;
}

int32_t TrueConfAudioDeviceModule::SetStereoPlayout(bool enable)
{
	return 0;
}

int32_t TrueConfAudioDeviceModule::StereoPlayout(bool* enabled) const
{
	return 0;
}

int32_t TrueConfAudioDeviceModule::StereoRecordingIsAvailable(bool* available) const
{
	return 0;
}

int32_t TrueConfAudioDeviceModule::SetStereoRecording(bool enable)
{
	return 0;
}

int32_t TrueConfAudioDeviceModule::StereoRecording(bool* enabled) const
{
	return 0;
}

int32_t TrueConfAudioDeviceModule::PlayoutDelay(uint16_t* delayMS) const
{
	return 0;
}

bool TrueConfAudioDeviceModule::BuiltInAECIsAvailable() const
{
	return false;
}

bool TrueConfAudioDeviceModule::BuiltInAGCIsAvailable() const
{
	return false;
}

bool TrueConfAudioDeviceModule::BuiltInNSIsAvailable() const
{
	return false;
}

int32_t TrueConfAudioDeviceModule::EnableBuiltInAEC(bool enable)
{
	return -1;
}

int32_t TrueConfAudioDeviceModule::EnableBuiltInAGC(bool enable)
{
	return -1;
}

int32_t TrueConfAudioDeviceModule::EnableBuiltInNS(bool enable)
{
	return -1;
}
