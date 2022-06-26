#include <Guiddef.h>
#pragma once
int VS_GetAudioDevicesSampleRate(const wchar_t *audio_capture, const wchar_t * audio_render);
int VS_GetAudioDeviceSampleRate(const wchar_t* device, const bool is_capture);
int VS_GetAudioDeviceGuidSampleRate(GUID guid, const bool is_capture, wchar_t *dev_interface);
int VS_GetCorrectedDeviceSampleRate(const wchar_t* device, const bool is_capture);
int VS_GetCorrectedDeviceGuidSampleRate(GUID guid, const bool is_capture, wchar_t *dev_interface);
bool VS_CompareDeviceInteface(const wchar_t *interface1, const wchar_t *interface2);

