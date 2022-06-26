/****************************************************************************
 * (c) 2004 Visicron Inc.  http://www.visicron.net/
 *
 * Project: std - Media format description
 *
 * Craeted by SMinrovK 24.02.2004
 *
 ****************************************************************************/
/****************************************************************************
 * \file VS_MediaFormat.h
 ****************************************************************************/

#ifndef VS_MEDIAFORMAT_H
#define VS_MEDIAFORMAT_H

#include <cstdint>
#include <cstring>
#include <sstream>

#define VSFOURCC(a, b, c, d) ( \
	static_cast<uint32_t>(a)       | \
	static_cast<uint32_t>(b) <<  8 | \
	static_cast<uint32_t>(c) << 16 | \
	static_cast<uint32_t>(d) << 24 \
)

const uint32_t VS_VCODEC_H261(VSFOURCC('h', '2', '6', '1'));
const uint32_t VS_VCODEC_H263(VSFOURCC('h', '2', '6', '3'));
const uint32_t VS_VCODEC_H263P(VSFOURCC('h', '2', '6', 'p'));
const uint32_t VS_VCODEC_H264(VSFOURCC('v', 'h', '6', '4'));
const uint32_t VS_VCODEC_H265(VSFOURCC('v', 'h', '6', '5'));
const uint32_t VS_VCODEC_VPX(VSFOURCC('p', 'v', '8', '0'));
const uint32_t VS_VCODEC_VP9(VSFOURCC('v', 'p', '9', '0'));
const uint32_t VS_VCODEC_VPXHD(VSFOURCC('p', 'v', 'h', 'd'));
const uint32_t VS_VCODEC_VPXSTEREO(VSFOURCC('p', 'v', 's', 't'));
const uint32_t VS_VCODEC_XC02(VSFOURCC('x', 'c', '0', '2'));
const uint32_t VS_VCODEC_MPEG4(VSFOURCC('4', 'p', 'm', 'f'));

const uint32_t FOURCC_I420(VSFOURCC('I', '4', '2', '0'));

#define VS_ACODEC_PCM         0x0001 // WAVE_FORMAT_PCM
#define VS_ACODEC_G711a       0x0006 // WAVE_FORMAT_ALAW
#define VS_ACODEC_G711mu      0x0007 // WAVE_FORMAT_MULAW
#define VS_ACODEC_GSM610      0x0031 // WAVE_FORMAT_GSM610
#define VS_ACODEC_G723        0x0042 // WAVE_FORMAT_MSG723
#define VS_ACODEC_G728        0x0041 // WAVE_FORMAT_G728_CELP
#define VS_ACODEC_G729A       0x0083 // WAVE_FORMAT_G729A
#define VS_ACODEC_G722        0x0065 // WAVE_FORMAT_G722_ADPCM
#define VS_ACODEC_G7221_24    0x7001
#define VS_ACODEC_G7221_32    0x7002
#define VS_ACODEC_SPEEX       0x7003
#define VS_ACODEC_ISAC        0x7004
#define VS_ACODEC_G7221C_24   0x7005
#define VS_ACODEC_G7221C_32   0x7006
#define VS_ACODEC_G7221C_48   0x7007
#define VS_ACODEC_OPUS_B0914  0x7ff0
#define VS_ACODEC_AAC         0xA106 // WAVE_FORMAT_MPEG4_AAC
#define VS_ACODEC_MP3         0x0055 // WAVE_FORMAT_MPEGLAYER3

#define MMFMT_DEFAULT_VIDEOCODEC      VS_VCODEC_XC02
#define MMFMT_DEFAULT_VIDEOHEIGHT     240u
#define MMFMT_DEFAULT_VIDEOWIDHT      320u
#define MMFMT_DEFAULT_VIDEOFPS        15u
#define MMFMT_DEFAULT_VIDEOSTEREO     0u
#define MMFMT_DEFAULT_VIDEOSVCMODE    0u
#define MMFMT_DEFAULT_HWCODEC         0u
#define MMFMT_DEFAULT_AUDIOSAMPLERATE 8000u
#define MMFMT_DEFAULT_AUDIOCODEC      VS_ACODEC_GSM610
#define MMFMT_DEFAULT_AUDIOBUFFDURR   0xffffffff

static const uint32_t VS_EnumVCodecs[9] =
{
	VS_VCODEC_XC02,  VS_VCODEC_H264,
	VS_VCODEC_H263P, VS_VCODEC_H263,
	VS_VCODEC_H261,  VS_VCODEC_VPX,
	VS_VCODEC_VPXHD, VS_VCODEC_VPXSTEREO,
	VS_VCODEC_H265
};

class VS_MediaFormat {
	// do not change order of variables!!!
	//-------------------------
	uint32_t dwSize;
	//-------------------------
public:
	//-------------------------
	uint32_t dwVideoHeight;
	uint32_t dwVideoWidht;
	uint32_t dwVideoCodecFCC;
	uint32_t dwAudioSampleRate;
	uint32_t dwAudioCodecTag;
	//-------------------------
	uint32_t dwAudioBufferLen;
	//-------------------------
	uint32_t dwFps;
	//-------------------------
	uint32_t dwStereo;
	//-------------------------
	uint32_t dwSVCMode;
	//-------------------------
	uint32_t dwHWCodec;
	//-------------------------
	VS_MediaFormat() {
		ReSet();
	}
	VS_MediaFormat &operator =(const VS_MediaFormat& src) {
		SetVideo(src.dwVideoWidht, src.dwVideoHeight, src.dwVideoCodecFCC);
		SetAudio(src.dwAudioSampleRate, src.dwAudioCodecTag);
		if (src.dwSize > 6 * sizeof(uint32_t))
			dwAudioBufferLen = src.dwAudioBufferLen;
		if (src.dwSize > 7 * sizeof(uint32_t))
			dwFps = src.dwFps;
		if (src.dwSize > 8 * sizeof(uint32_t))
			dwStereo = src.dwStereo;
		if (src.dwSize > 9 * sizeof(uint32_t))
			dwSVCMode = src.dwSVCMode;
		if (src.dwSize > 10 * sizeof(uint32_t))
			dwHWCodec = src.dwHWCodec;
		return *this;
	}

	bool operator ==(const VS_MediaFormat& src) const {
		const uint32_t size = dwSize < src.dwSize ? dwSize : src.dwSize;
		return memcmp(this, &src, size)==0;
	}
	void SetVideo(uint32_t Widht, uint32_t Height, uint32_t CodecFCC = MMFMT_DEFAULT_VIDEOCODEC, uint32_t Fps = MMFMT_DEFAULT_VIDEOFPS,
	              uint32_t Stereo = MMFMT_DEFAULT_VIDEOSTEREO, uint32_t SVCMode = MMFMT_DEFAULT_VIDEOSVCMODE, uint32_t HWCodec = MMFMT_DEFAULT_HWCODEC) {
		dwVideoHeight		= Height;
		dwVideoWidht		= Widht;
		dwVideoCodecFCC		= CodecFCC;
		dwFps				= Fps;
		dwStereo			= Stereo;
		dwSVCMode			= SVCMode;
		dwHWCodec			= HWCodec;
	}
	void SetAudio(uint32_t SampleRate, uint32_t codecTag = MMFMT_DEFAULT_AUDIOCODEC, uint32_t bufferDurr = MMFMT_DEFAULT_AUDIOBUFFDURR) {
		// avoid to use any sample rate exept 8000, 11025, 16000, 22050, 32000 Hz
		dwAudioSampleRate	= SampleRate;
		dwAudioCodecTag		= codecTag;
		dwAudioBufferLen = 0;
		if (SampleRate < 8000)
			return;
		uint32_t minLen = 0;
		switch (codecTag)
		{
		case VS_ACODEC_ISAC:
			if (bufferDurr == MMFMT_DEFAULT_AUDIOBUFFDURR) bufferDurr = 30;
			minLen = 480; // 30 ms
			if (SampleRate > 32000) SampleRate = 32000; // max 32 KHz for iSAC
			break;
		case VS_ACODEC_G723:
			if (bufferDurr == MMFMT_DEFAULT_AUDIOBUFFDURR) bufferDurr = 80;
			minLen = 480; // 30 ms
			break;
		case VS_ACODEC_OPUS_B0914:
			if (bufferDurr == MMFMT_DEFAULT_AUDIOBUFFDURR) bufferDurr = 40;
			minLen = 640; // 40 ms
			SampleRate = (SampleRate + 2000) / 4000 * 4000; // restrict to 8, 12, 16, 24, 48 KHz
			if (SampleRate > 24000) SampleRate = 48000;
			else if (SampleRate == 12000) minLen = 960;
			break;
		case VS_ACODEC_MP3:
			if (bufferDurr == MMFMT_DEFAULT_AUDIOBUFFDURR) bufferDurr = 0;
			// 16 KHz - 36 ms, other 576s * 2
			minLen = 1152;
			if (SampleRate <= 16000) SampleRate = 8000;
			else SampleRate = 16000;
			break;
		case VS_ACODEC_AAC:
			if (bufferDurr == MMFMT_DEFAULT_AUDIOBUFFDURR) bufferDurr = 0;
			// for all 1024 samples : 16KHz = 64ms ... 48KHz ~ 20ms
			minLen = 2048;
			SampleRate = 8000;
			break;
		default :
			if (bufferDurr == MMFMT_DEFAULT_AUDIOBUFFDURR) bufferDurr = 40;
			minLen = 320; // 20 ms
		}
		minLen*= SampleRate/8000;
		dwAudioBufferLen = (int)(SampleRate*2*0.001*bufferDurr/minLen + 0.5)*minLen;
		if (dwAudioBufferLen < minLen) dwAudioBufferLen = minLen;
	}
	void ReSet() {
		dwSize				= sizeof(VS_MediaFormat);
		SetVideo(MMFMT_DEFAULT_VIDEOWIDHT, MMFMT_DEFAULT_VIDEOHEIGHT);
		SetAudio(MMFMT_DEFAULT_AUDIOSAMPLERATE);
	}
	void SetZero() {
		dwVideoHeight		= 0;
		dwVideoWidht		= 0;
		dwVideoCodecFCC		= 0;
		dwAudioSampleRate	= 0;
		dwAudioCodecTag		= 0;
		dwAudioBufferLen	= 0;
		dwFps				= 0;
		dwStereo			= 0;
		dwSVCMode			= 0;
		dwHWCodec			= 0;
	}
	uint32_t GetSize() const {
		return dwSize;
	}
	bool IsVideoValid() const {
		return dwVideoCodecFCC!=0
			&& dwVideoHeight>=64 && dwVideoWidht>=64
			&& dwVideoHeight<=131072 && dwVideoWidht<=131072
			&& ((dwVideoHeight|dwVideoWidht)&0x1) == 0;
	}
	bool IsVideoValid_WithoutMultiplicity8() const {
		return dwVideoCodecFCC!=0
			&& dwVideoHeight>=64 && dwVideoWidht>=64
			&& dwVideoHeight<=131072 && dwVideoWidht<=131072;
	}
	bool IsAudioValid() const {
		return dwAudioCodecTag!=0 && dwAudioSampleRate>=8000 && dwAudioSampleRate<=48000 && dwAudioBufferLen!=0;
	}
	bool VideoEq(const VS_MediaFormat& src) const {
		return dwVideoCodecFCC == src.dwVideoCodecFCC
			&& dwVideoHeight == src.dwVideoHeight
			&& dwVideoWidht == src.dwVideoWidht;
	}
	bool VideoNonCmpEq(const VS_MediaFormat& src) const {
		return dwVideoHeight == src.dwVideoHeight
			&& dwVideoWidht == src.dwVideoWidht
			&& dwFps == src.dwFps
			&& dwStereo == src.dwStereo
			&& dwHWCodec == src.dwHWCodec;
	}
	bool VideoSpatialEq(const VS_MediaFormat& src) const {
		return dwVideoHeight == src.dwVideoHeight
			&& dwVideoWidht == src.dwVideoWidht
			&& dwStereo == src.dwStereo
			&& dwHWCodec == src.dwHWCodec;
	}
	bool AudioEq(const VS_MediaFormat& src) const {
		return dwAudioCodecTag == src.dwAudioCodecTag
			&& dwAudioSampleRate == src.dwAudioSampleRate;
	}
	int GetModeResolution() const {
		auto square = dwVideoHeight * dwVideoWidht;
		if (square >= 2073600) return 4; /// 1920 x 1080
		if (square >=  921600) return 3; /// 1280 x  720
		if (square >=  320000) return 2; ///  800 x  400
		if (square >=  225280) return 1; ///  640 x  352
		return 0;
	}
	unsigned GetMBps() const {
		return (dwVideoHeight * dwVideoWidht * dwFps / 256);
	};
	unsigned GetFrameSizeMB() const {
		return (dwVideoHeight * dwVideoWidht / 256);
	};
	std::string ToString() const {
		std::string vCodec;
		vCodec.append(reinterpret_cast<const char*>(&dwVideoCodecFCC), 4);
		std::stringstream ss;
		ss<<"VCodec = "<<vCodec<<"\nACodec = 0x"<<std::hex<<dwAudioCodecTag;
		return ss.str();
	}
};

#endif
