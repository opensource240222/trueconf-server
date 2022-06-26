
#ifndef WEBRTC_API_TRUECONF_ADM_H_
#define WEBRTC_API_TRUECONF_ADM_H_

#include "modules/audio_device/include/audio_device.h"
#include "rtc_base/sigslot.h"

namespace tc_webrtc_api
{
	class TrueConfAudioDeviceModuleCallback;
}

namespace tc_webrtc_api
{

	class TrueConfAudioDeviceModule :	public webrtc::AudioDeviceModule,
										public sigslot::has_slots<>
	{

	public:

		TrueConfAudioDeviceModule(TrueConfAudioDeviceModuleCallback *callback);
		~TrueConfAudioDeviceModule() override;
		void OnDeliverAudioFrame(const uint8_t *audio, int length, int samples, int samplerate, int channels);

	public:

		int32_t ActiveAudioLayer(AudioLayer* audioLayer) const override;
		int32_t RegisterAudioCallback(webrtc::AudioTransport* audioCallback) override;
		int32_t Init() override;
		int32_t Terminate() override;
		bool Initialized() const override;
		int16_t PlayoutDevices() override;
		int16_t RecordingDevices() override;
		int32_t PlayoutDeviceName(uint16_t index, char name[webrtc::kAdmMaxDeviceNameSize], char guid[webrtc::kAdmMaxGuidSize]) override;
		int32_t RecordingDeviceName(uint16_t index, char name[webrtc::kAdmMaxDeviceNameSize], char guid[webrtc::kAdmMaxGuidSize]) override;
		int32_t SetPlayoutDevice(uint16_t index) override;
		int32_t SetPlayoutDevice(WindowsDeviceType device) override;
		int32_t SetRecordingDevice(uint16_t index) override;
		int32_t SetRecordingDevice(WindowsDeviceType device) override;
		int32_t PlayoutIsAvailable(bool* available) override;
		int32_t InitPlayout() override;
		bool PlayoutIsInitialized() const override;
		int32_t RecordingIsAvailable(bool* available) override;
		int32_t InitRecording() override;
		bool RecordingIsInitialized() const override;
		int32_t StartPlayout() override;
		int32_t StopPlayout() override;
		bool Playing() const override;
		int32_t StartRecording() override;
		int32_t StopRecording() override;
		bool Recording() const override;
		int32_t InitSpeaker() override;
		bool SpeakerIsInitialized() const override;
		int32_t InitMicrophone() override;
		bool MicrophoneIsInitialized() const override;
		int32_t SpeakerVolumeIsAvailable(bool* available) override;
		int32_t SetSpeakerVolume(uint32_t volume) override;
		int32_t SpeakerVolume(uint32_t* volume) const override;
		int32_t MaxSpeakerVolume(uint32_t* maxVolume) const override;
		int32_t MinSpeakerVolume(uint32_t* minVolume) const override;
		int32_t MicrophoneVolumeIsAvailable(bool* available) override;
		int32_t SetMicrophoneVolume(uint32_t volume) override;
		int32_t MicrophoneVolume(uint32_t* volume) const override;
		int32_t MaxMicrophoneVolume(uint32_t* maxVolume) const override;
		int32_t MinMicrophoneVolume(uint32_t* minVolume) const override;
		int32_t SpeakerMuteIsAvailable(bool* available) override;
		int32_t SetSpeakerMute(bool enable) override;
		int32_t SpeakerMute(bool* enabled) const override;
		int32_t MicrophoneMuteIsAvailable(bool* available) override;
		int32_t SetMicrophoneMute(bool enable) override;
		int32_t MicrophoneMute(bool* enabled) const override;
		int32_t StereoPlayoutIsAvailable(bool* available) const override;
		int32_t SetStereoPlayout(bool enable) override;
		int32_t StereoPlayout(bool* enabled) const override;
		int32_t StereoRecordingIsAvailable(bool* available) const override;
		int32_t SetStereoRecording(bool enable) override;
		int32_t StereoRecording(bool* enabled) const override;
		int32_t PlayoutDelay(uint16_t* delayMS) const override;
		bool BuiltInAECIsAvailable() const override;
		bool BuiltInAGCIsAvailable() const override;
		bool BuiltInNSIsAvailable() const override;
		int32_t EnableBuiltInAEC(bool enable) override;
		int32_t EnableBuiltInAGC(bool enable) override;
		int32_t EnableBuiltInNS(bool enable) override;

	private:

		int16_t *m_frameBuffer;
		bool m_initialized = false;
		bool m_initPlayout = false;
		bool m_initRecord = false;
		bool m_startPlayout = false;
		bool m_startRecord = false;
		TrueConfAudioDeviceModuleCallback *m_callback;
		webrtc::AudioTransport *m_transportCallback = nullptr;

	};

}

#endif /* WEBRTC_API_TRUECONF_ADM_H_ */