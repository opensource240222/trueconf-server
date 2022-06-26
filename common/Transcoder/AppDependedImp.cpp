#include "VS_IppAudiCodec.h"
#include "VideoCodec.h"

#ifdef _WIN32	// not ported
#include "Transcoder/GetTypeHardwareCodec.h"
#include "VS_VPXStereoVideoCodec.h"
#include "VSClient/VS_HardwareH264VideoCodec.h"
#include "AudioCodecSystem.h"
#include "VS_H264IntelVideoCodec.h"
#include "VS_NvidiaVideoCodec.h"
#endif

#include "VS_SpeexAudioCodec.h"
#include "VS_iSACAudioCodec.h"
#include "VS_OpusAudioCodec.h"
#include "VS_MP3AudioCodec.h"
#include "VS_AACAudioCodec.h"
#include "VS_IppAudiCodec.h"
#include "std/cpplib/VS_VideoLevelCaps.h"
#include "Audio/EchoCancel/VS_RtcEchoCancellation.h"

#include "VS_OpenH264VideoCodec.h"
#include "VS_OpenH264SlidesVideoCodec.h"
#include "VS_VPXVideoCodec.h"
#include "VS_FFVideoCodec.h"
#include "VS_RetriveVideoCodec.h"

#ifdef _WIN32
HINSTANCE VS_LoadCodecsLib()
{
	static HINSTANCE ippCodecExtLib = 0;
	static bool ippCodecExtLibLoaded = false;
	if (ippCodecExtLib == 0 && !ippCodecExtLibLoaded) {
		ippCodecExtLib = LoadLibrary("CodecsDll");
		ippCodecExtLibLoaded = true;
	}
	return ippCodecExtLib;
}
#endif

// AudioCodec support

#if defined(_ZOOMCALL_) || defined(_TESTAEC_)
AudioCodec* VS_RetriveAudioCodec(int Id, bool isCodec)
{
	if (Id == VS_ACODEC_PCM) {
		return new VS_PcmAudioCodec(VS_ACODEC_PCM, isCodec);
	}
	else if	(Id == VS_ACODEC_G723)
		return new VS_IppAudiCodec(Id, isCodec);
	else if	(Id==VS_ACODEC_GSM610)
		if (isCodec)return new VS_AudioCoderGSM610;
		else		return new VS_AudioDecoderGSM610;
	else if (Id==VS_ACODEC_SPEEX)
		return new VS_SpeexAudioCodec(Id, isCodec);
	else if (Id==VS_ACODEC_ISAC)
		return new VS_iSACAudioCodec(Id, isCodec);
	else if (Id==VS_ACODEC_OPUS_B0914)
		return new VS_OpusAudioCodec(Id, isCodec);
	else if (Id == VS_ACODEC_MP3)
		return new VS_MP3AudioCodec(Id, isCodec);
	else if (Id == VS_ACODEC_AAC)
		return new VS_AACAudioCodec(Id, isCodec);
	else {
		HINSTANCE ippCodecExtLib = VS_LoadCodecsLib();
		if (ippCodecExtLib) {
			typedef void* (*GetCodecType)(int , bool);
			GetCodecType get_codec = (GetCodecType)GetProcAddress(ippCodecExtLib, "GetAudioCodec");
			if (get_codec) {
				return (AudioCodec *)get_codec(Id, isCodec);
			}
		}
		return 0;
	}
}

#elif defined(_CODECSDLL_)
AudioCodec* VS_RetriveAudioCodec(int Id, bool isCodec)
{
	if (Id == VS_ACODEC_PCM) {
		return new VS_PcmAudioCodec(VS_ACODEC_PCM, isCodec);
	}
	else if (Id == VS_ACODEC_G711a || Id == VS_ACODEC_G711mu ||
			 Id == VS_ACODEC_G728 || Id == VS_ACODEC_G729A || Id == VS_ACODEC_G722 || Id == VS_ACODEC_G7221_24 || Id == VS_ACODEC_G7221_32 ||
			 Id == VS_ACODEC_G7221C_24 || Id == VS_ACODEC_G7221C_32 || Id == VS_ACODEC_G7221C_48) {
			return new VS_IppAudiCodec(Id, isCodec);
	}
	else return 0;
}

#elif defined(_H323GATEWAYCLIENT_)

#pragma comment (lib,"libmp3lame.lib")

AudioCodec* VS_RetriveAudioCodec(int Id, bool isCodec)
{
	if (Id == VS_ACODEC_PCM) {
		return new VS_PcmAudioCodec(VS_ACODEC_PCM, isCodec);
	}
	else if (Id == VS_ACODEC_G711a || Id == VS_ACODEC_G711mu ||
		Id == VS_ACODEC_G728 || Id == VS_ACODEC_G729A || Id == VS_ACODEC_G722 ||
		Id == VS_ACODEC_G723 || Id == VS_ACODEC_G7221_24 || Id == VS_ACODEC_G7221_32 ||
		Id == VS_ACODEC_G7221C_24 || Id == VS_ACODEC_G7221C_32 || Id == VS_ACODEC_G7221C_48) {
			return new VS_IppAudiCodec(Id, isCodec);
	}
#ifdef _WIN32
	else if	(Id==VS_ACODEC_GSM610)
		if (isCodec)return new VS_AudioCoderGSM610;
		else		return new VS_AudioDecoderGSM610;
#endif
	else if (Id==VS_ACODEC_SPEEX)
		return new VS_SpeexAudioCodec(Id, isCodec);
	else if (Id==VS_ACODEC_ISAC)
		return new VS_iSACAudioCodec(Id, isCodec);
	else if (Id==VS_ACODEC_OPUS_B0914)
		return new VS_OpusAudioCodec(Id, isCodec);
	else if (Id==VS_ACODEC_MP3)
		return new VS_MP3AudioCodec(Id, isCodec);
	else if (Id==VS_ACODEC_AAC)
		return new VS_AACAudioCodec(Id, isCodec);
	else return 0;
}
#else
AudioCodec* VS_RetriveAudioCodec(int Id, bool isCodec)
{
	return 0;
}
#endif

// VideoCodec support
//#define _H264_INCLUDED_

#if defined(_ZOOMCALL_) || defined(_H323GATEWAYCLIENT_)
VideoCodec* VS_RetriveVideoCodec(int Id, bool isCodec, unsigned int typeHardware, int32_t deviceId)
{
#if defined(_H323GATEWAYCLIENT_)
	if	(Id == VS_VCODEC_H261)
		if (isCodec) return new VS_VideoCoderH261;
		else return new VS_VideoDecoderH261;
	else
#endif
	if	(Id == VS_VCODEC_H263)
		if (isCodec) return new VS_VideoCoderH263;
		else return new VS_VideoDecoderH263;
	else if	(Id == VS_VCODEC_H263P)
		if (isCodec) return new VS_VideoCoderH263P;
		else return new VS_VideoDecoderH263P;
	else if (Id == VS_VCODEC_H264) {
#if defined(_H264_INCLUDED_) || defined(_H323GATEWAYCLIENT_)
		eHardwareEncoder typehw = (eHardwareEncoder) typeHardware;
		if (isCodec) {
#ifdef _WIN32 // not ported
			if (typehw == ENCODER_H264_INTEL) {
				if (GetTypeHardwareCodec() == ENCODER_H264_INTEL ||
					GetTypeHardwareCodec() == ENCODER_H264_INTEL_MSS) {
					return new VS_H264IntelVideoCodec(Id, isCodec);
				}
			}
			else if (typehw == ENCODER_H264_INTEL_MSS) {
				if (GetTypeHardwareCodec() == ENCODER_H264_INTEL_MSS) {
					return new VS_H264TranscoderIntelVideoCodec(Id, isCodec, deviceId);
				}
			}
			else if (typehw == ENCODER_H264_NVIDIA) {
				return new VS_NvidiaVideoEncoder(Id, deviceId);
			}
#endif
			if (typehw == ENCODER_SLIDES) {
				return new VS_OpenH264SlidesVideoCodec(Id, isCodec);
			}
			if (typehw == ENCODER_SOFTWARE) {
				return new VS_OpenH264VideoCodec(Id, isCodec);
			}
			else {
				return 0;
			}
		} else {
#ifdef _WIN32 // not ported
			if (typehw == ENCODER_H264_INTEL) {
				if (GetTypeHardwareCodec() == ENCODER_H264_INTEL ||
					GetTypeHardwareCodec() == ENCODER_H264_INTEL_MSS) {
					return new VS_H264IntelVideoCodec(Id, isCodec);
				}
			}
			else if (typehw == ENCODER_H264_NVIDIA) {
				return new VS_NvidiaVideoDecoder(Id, deviceId);
			}
#endif
			return new VS_VideoDecoderH264();
		}
#else
		VideoCodec *vc = nullptr;
		eHardwareEncoder typehw = (eHardwareEncoder)typeHardware;
		if (isCodec) {
			if (typehw == ENCODER_H264_INTEL) {
				vc = new VS_H264IntelVideoCodec(Id, isCodec);
			} else if (typehw == ENCODER_H264_INTEL_MSS) {
				vc = new VS_H264TranscoderIntelVideoCodec(Id, isCodec);
			} else if (typehw == ENCODER_H264_LOGITECH) {
				vc = new VS_HardwareH264VideoCodec(Id, isCodec);
			} else if (typehw == ENCODER_H264_NVIDIA) {
				vc = new VS_NvidiaVideoEncoder(Id);
			} else {
				vc = new VS_OpenH264VideoCodec(Id, isCodec);
			}
		} else {
			if ((GetTypeHardwareCodec() == ENCODER_H264_INTEL ||
				GetTypeHardwareCodec() == ENCODER_H264_INTEL_MSS) &&
				typeHardware > 0) {
				vc = new VS_H264IntelVideoCodec(Id, isCodec);
			}
			else {
				vc = new VS_VideoDecoderH264();
			}
		}
		if (vc) return vc;
		HINSTANCE ippCodecExtLib = VS_LoadCodecsLib();
		if (ippCodecExtLib) {
			typedef void* (*GetCodec)(int, bool);
			GetCodec create_codec = (GetCodec)GetProcAddress(ippCodecExtLib, "GetVideoCodec");
			if (create_codec) {
				return (VideoCodec*)create_codec(Id, isCodec);
			}
		}
#endif
	}
	else if (Id == VS_VCODEC_H265) {
		if (isCodec) {

		}
		else {
			return new VS_VideoDecoderH265();
		}
	}

#if defined(_VPX_INCLUDED_) || defined(_H323GATEWAYCLIENT_)
#ifdef _VPX_INCLUDED_
#	if !defined(VZOCHAT7)
#		pragma comment (lib,"libvpx.lib")
#	endif
#else
	#ifdef _DEBUG
		#pragma         comment (lib,"../../../libd/libvpx.lib")
	#else
		#pragma         comment (lib,"../../../lib/libvpx.lib")
	#endif
#endif
	else if (Id == VS_VCODEC_VPX)
	{
		eHardwareEncoder typehw = (eHardwareEncoder) typeHardware;
		if (isCodec) {
			return new VS_VPXVideoCodec(Id, isCodec);
		} else {
#ifdef _WIN32 // not ported
			if (typehw == ENCODER_H264_NVIDIA) {
				return new VS_NvidiaVideoDecoder(Id, deviceId);
			}
#endif
			return new VS_VideoDecoderVP8;
		}
	}
#ifdef _WIN32
	else if (Id == VS_VCODEC_VPXHD)
		return new VS_VPXHDVideoCodec(Id, isCodec);
	else if (Id == VS_VCODEC_VPXSTEREO)
		return new VS_VPXStereoVideoCodec(Id, isCodec);
#endif
#endif
	return 0;
}

#if !(defined(_VPX_INCLUDED_) || defined(_H323GATEWAYCLIENT_))
VPXCodec* VS_RetriveVPXCodec(int Id, bool isCodec)
{
	return 0;
}
#endif

#elif defined(_CODECSDLL_)

VideoCodec* VS_RetriveVideoCodec(int Id, bool isCodec, unsigned int typeHardware, int32_t deviceId)
{
	if (Id == VS_VCODEC_H264) {
		return new VS_OpenH264VideoCodec(Id, isCodec);
	}
	return 0;
}

VPXCodec* VS_RetriveVPXCodec(int Id, bool isCodec)
{
	return 0;
}

/// don't link ffmpeg in codecsdll.dll

FF_VideoCodec::FF_VideoCodec(FF_VCodecID CocecId, bool IsCoder){}
FF_VideoCodec::~FF_VideoCodec() {}
int FF_VideoCodec::Init(int w, int h, int annex, int framerate){return 0;}
void FF_VideoCodec::Release(){}
int FF_VideoCodec::Convert(unsigned char *in, unsigned char* out, int* param){ return 0; }
int FF_VideoCodec::SetBitrate(int bitrate) {return 0;}

/// don't link h.264 hardware in codecsdll.dll

int  VS_HardwareH264VideoCodec::Init(int w, int h, uint32_t ColorMode, unsigned char sndLvl, int numThreads, unsigned int framerate) { return 0; }
void VS_HardwareH264VideoCodec::Release() {};
int	 VS_HardwareH264VideoCodec::Convert(BYTE *in, BYTE* out, VS_VideoCodecParam* param) { return 0; }
bool VS_HardwareH264VideoCodec::UpdateBitrate() { return false; }

#else

VideoCodec* VS_RetriveVideoCodec(int Id, bool isCodec, unsigned int typeHardware, int32_t deviceId)
{
	return 0;
}

/// don't link ffmpeg

FF_VideoCodec::FF_VideoCodec(FF_VCodecID CocecId, bool IsCoder){}
FF_VideoCodec::~FF_VideoCodec() {}
int FF_VideoCodec::Init(int w, int h, int annex,int framerate){return 0;}
void FF_VideoCodec::Release(){}
int FF_VideoCodec::Convert(unsigned char *in, unsigned char* out, int* param){return 0;}
int FF_VideoCodec::SetBitrate(int bitrate) {return 0;}

#endif

VideoCodec* VS_RetriveVideoCodec(const VS_MediaFormat &mf, bool encoder)
{

#if defined(_H323GATEWAYCLIENT_) && defined(_WINDOWS_)
	auto info = LoadBalancingHardware::GetLoadBalancing().HoldResources(mf, encoder);
	auto codec = VS_RetriveVideoCodec(mf.dwVideoCodecFCC, encoder, (mf.dwHWCodec == ENCODER_SLIDES) ? ENCODER_SLIDES : LoadBalancingHardware::HardwareFromDevice(info.device), info.id);
	if (!LoadBalancingHardware::GetLoadBalancing().RegisterVideoCodec(reinterpret_cast<std::uintptr_t>(codec), info)) {
		LoadBalancingHardware::GetLoadBalancing().UnholdResources(info);
	}
	return codec;
#else
	return VS_RetriveVideoCodec(mf.dwVideoCodecFCC, encoder, mf.dwHWCodec);
#endif

}

void VS_UnregisteredVideoCodec(VideoCodec *codec)
{

#if defined(_H323GATEWAYCLIENT_) && defined(_WINDOWS_)
	LoadBalancingHardware::GetLoadBalancing().UnregisterVideoCodec(reinterpret_cast<std::uintptr_t>(codec));
#else
#endif

}

std::uintptr_t VS_GetContextDevice(VideoCodec *codec)
{

#if defined(_H323GATEWAYCLIENT_) && defined(_WINDOWS_)
	return LoadBalancingHardware::GetLoadBalancing().GetContextDevice(reinterpret_cast<std::uintptr_t>(codec));
#else
	return 0;
#endif

}

load_balancing::BalancingDevice VS_GetTypeDevice(VideoCodec *codec)
{

#if defined(_H323GATEWAYCLIENT_) && defined(_WINDOWS_)
	return LoadBalancingHardware::GetLoadBalancing().GetTypeDevice(reinterpret_cast<std::uintptr_t>(codec));
#else
	return load_balancing::BalancingDevice::software;
#endif

}

#if defined(_ZOOMCALL_) || defined (_TESTAEC_)

VS_EchoCancelBase* VS_RetriveEchoCancel(int Id)
{
	if (Id == 0) return new VS_SpeexEchoCancel();
	else if (Id == 1)  return new VS_WebRTCEchoCancel();
	else if (Id == 2)  return new VS_WebRTCFastEchoCancel();
	return 0;
}

#else

VS_EchoCancelBase* VS_RetriveEchoCancel(int Id)
{
	return 0;
}

#endif
