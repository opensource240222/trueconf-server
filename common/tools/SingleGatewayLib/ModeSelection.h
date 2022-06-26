#pragma once

class VS_MediaFormat;
struct VS_GatewayVideoMode;
struct VS_GatewayAudioMode;
class VS_ClientCaps;

void FillMediaFormat(VS_MediaFormat& mf, const VS_GatewayVideoMode& mode, bool isSender, bool isGroupConf);
void FillMediaFormat(VS_MediaFormat& mf, const VS_GatewayAudioMode& mode);

bool LimitRTP2VSResolution(const VS_ClientCaps& caps, const VS_MediaFormat& rcvmf, VS_MediaFormat& sndmf);