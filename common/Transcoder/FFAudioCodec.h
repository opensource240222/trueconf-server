#pragma once

extern "C"
{
#ifndef __STDC_CONSTANT_MACROS
# define __STDC_CONSTANT_MACROS
#endif
#include "libavcodec/avcodec.h"
}

#include <inttypes.h>

class FFAudioCodec
{
private:
	AVCodecContext* m_CodecContext;
	AVFrame* m_Frame;

	char* m_TmpIn;
	size_t m_TmpInSize;
	size_t m_TmpInCapacity;

public:
	FFAudioCodec();
	~FFAudioCodec();

	bool Init(int frequency, bool isCoder, int bitrate);
	void Release();

	int Convert(char* in, char* out, int insize);
};