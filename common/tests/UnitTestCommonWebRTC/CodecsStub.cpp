#include "Transcoder/AudioCodec.h"
#include "Transcoder/VideoCodec.h"
#include "Transcoder/LoadBalancing/VS_LoadBalancer.h"

AudioCodec* VS_RetriveAudioCodec(int, bool)
{
	return nullptr;
}

VideoCodec* VS_RetriveVideoCodec(int, bool, unsigned int, int32_t)
{
	return nullptr;
}

VideoCodec* VS_RetriveVideoCodec(const VS_MediaFormat &, bool)
{
	return nullptr;
}

void VS_UnregisteredVideoCodec(VideoCodec *codec)
{
}

std::uintptr_t VS_GetContextDevice(VideoCodec *codec)
{
	return 0;
}

load_balancing::BalancingDevice VS_GetTypeDevice(VideoCodec *codec)
{
	return load_balancing::BalancingDevice::software;
}