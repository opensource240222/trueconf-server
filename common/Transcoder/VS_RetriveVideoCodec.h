#pragma once

#include "VideoCodec.h"
#include "LoadBalancing/VS_LoadBalancer.h"

VideoCodec* VS_RetriveVideoCodec(int Id, bool isCodec, unsigned int typeHardware = 0, int32_t deviceId = 0);
VideoCodec* VS_RetriveVideoCodec(const VS_MediaFormat &mf, bool encoder);
void VS_UnregisteredVideoCodec(VideoCodec *codec);
std::uintptr_t VS_GetContextDevice(VideoCodec *codec);
load_balancing::BalancingDevice VS_GetTypeDevice(VideoCodec *codec);
