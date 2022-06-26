#pragma once
#include "webrtc/common_audio/vad/include/webrtc_vad.h"


class VS_RtcVoiceActivity
{
private:
	VadInst* m_vad;
	bool  m_isInit;
	int m_fs;
public:
	VS_RtcVoiceActivity();
	~VS_RtcVoiceActivity();

	int Init(int fs, int mode);
	int Release();

	int Process(short int* data, int len);

	int SetMode(int mode);

};