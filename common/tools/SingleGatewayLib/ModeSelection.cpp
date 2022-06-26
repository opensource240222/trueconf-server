#include "ModeSelection.h"
#include "../Server/CommonTypes.h"
#include "VS_H264ResolutionCalc.h"

#include "std/cpplib/VS_MediaFormat.h"
#include "std/cpplib/VS_ClientCaps.h"
#include "std/cpplib/VS_VideoLevelCaps.h"

#include <cmath>

void FillMediaFormat(VS_MediaFormat& mf, const VS_GatewayVideoMode& mode, bool isSender, bool isGroupConf)
{
	mf.SetVideo(0, 0, 0);
	int CodecId = 0;
	if (mode.CodecType == e_videoH261)
		CodecId = VS_VCODEC_H261;
	else if (isSender&&mode.CodecType==e_videoH263)
	{
		CodecId = VS_VCODEC_H263;
	}
	else if (mode.CodecType == e_videoH263 ||
		mode.CodecType == e_videoH263plus ||
		mode.CodecType == e_videoH263plus2)
	{
		CodecId = VS_VCODEC_H263P;
	}
	else if (mode.CodecType == e_videoH264 || mode.CodecType == e_videoXH264UC)
	{
		CodecId = VS_VCODEC_H264;
	}
	else
		return;

	// determine TC->SIP video resolution depending on bitrate and codec type
	size_t maxW = 0, maxH = 0;
	if (isSender)
	{
		auto bitrate = mode.Bitrate / 1000;
		if (mode.CodecType == e_videoH263 || mode.CodecType == e_videoH263plus || mode.CodecType == e_videoH263plus2)
		{
			maxW = bitrate >= 384 ? 704 : 352;
			maxH = bitrate >= 384 ? 576 : 288;
		}
		else if (mode.CodecType == e_videoH264)
		{
			// Find largest mixer resolution with bitrate that is not greater than requested bitrate.
			// This relies on video_presets::video_info entries being sorted by bitrate.
			for (const auto& x : video_presets::video_info)
			{
				if (static_cast<unsigned>(x.bitrate) <= bitrate)
				{
					maxW = x.width;
					maxH = x.height;
				}
				else
					break;
			}
		}
		else if (mode.CodecType == e_videoXH264UC)
		{
			maxW = bitrate >= 768 ? 1280 : (bitrate >= 256 ? 640 : 320);
			maxH = bitrate >= 768 ? 720 : (bitrate >= 256 ? 360 : 180);
		}
	}

	unsigned long w = 0, h = 0;
	if (mode.CodecType == e_videoH264 || mode.CodecType == e_videoXH264UC)
	{
		VS_H264ResolutionCalc calc(mode.Mode);
		if (mode.MaxFs)
			calc.SetMaxFs(mode.MaxFs);
		if (mode.MaxMbps)
			calc.SetMaxMbps(mode.MaxMbps);

		w = calc.GetWidth();
		h = calc.GetHeight();
	}
	else
	{
		if      (mode.Mode & 0x08) {w = 704; h = 576;}
		else if (mode.Mode & 0x04) {w = 352; h = 288;}
		else if (mode.Mode & 0x02) {w = 176; h = 144;}
		else if (mode.Mode & 0x01) {w = 128; h = 96;}
		else                       {w = 352; h = 288;}
	}

	if (maxH && maxH < h)
	{
		w = maxW;
		h = maxH;
	}

	if ((mode.CodecType == e_videoH264 || mode.CodecType == e_videoXH264UC) && mode.preferred_width > 0 && mode.preferred_height > 0)
	{
		w = mode.preferred_width;
		h = mode.preferred_height;
	}

	mf.SetVideo(w, h, CodecId, 30);
	//mf.SetVideo(320, 180, CodecId, 15);
	//mf.SetVideo(640, 360, CodecId, 15);
}

void FillMediaFormat(VS_MediaFormat& mf, const VS_GatewayAudioMode& mode)
{
	mf.SetAudio(0, 0, 0);
	if (mode.CodecType ==e_rcvG723)
		mf.SetAudio(8000, VS_ACODEC_G723);
	if (mode.CodecType==e_rcvG711Alaw64k)
		mf.SetAudio(8000, VS_ACODEC_G711a);
	if (mode.CodecType==e_rcvG711Ulaw64k)
		mf.SetAudio(8000, VS_ACODEC_G711mu);
	if (mode.CodecType==e_rcvG728)
		mf.SetAudio(8000, VS_ACODEC_G728);
	if (mode.CodecType==e_rcvG729a)
		mf.SetAudio(8000, VS_ACODEC_G729A);
	if (mode.CodecType==e_rcvG722_64k)
		mf.SetAudio(16000, VS_ACODEC_G722);
	if (mode.CodecType==e_rcvG722124)
		mf.SetAudio(16000, VS_ACODEC_G7221_24);
	if(mode.CodecType==e_rcvG722132)
		mf.SetAudio(16000, VS_ACODEC_G7221_32);
	if (mode.CodecType==e_rcvSPEEX_16kHz)
		mf.SetAudio(16000, VS_ACODEC_SPEEX);
	if (mode.CodecType==e_rcvOPUS)
		mf.SetAudio(16000, VS_ACODEC_OPUS_B0914);
	if (mode.CodecType==e_rcvSIREN14_24)
		mf.SetAudio(32000, VS_ACODEC_G7221C_24);
	if (mode.CodecType==e_rcvSIREN14_32)
		mf.SetAudio(32000, VS_ACODEC_G7221C_32);
	if (mode.CodecType==e_rcvSIREN14_48)
		mf.SetAudio(32000, VS_ACODEC_G7221C_48);
	if (mode.CodecType == e_rcvMPA)
		mf.SetAudio(16000/*fake*/, VS_ACODEC_MP3);
	if (mode.CodecType == e_rcvAAC)
		mf.SetAudio(32000/*fake*/, VS_ACODEC_AAC);
}

bool LimitRTP2VSResolution(const VS_ClientCaps& caps, const VS_MediaFormat& rcvmf, VS_MediaFormat& sndmf)
{
	tc_VideoLevelCaps levelCaps;
	bool bCanMul8 = (caps.GetStreamsDC() & VSCC_STREAM_CAN_USE_SVC) || (caps.GetVideoRcv() & VSCC_VIDEO_MULTIPLICITY8);
	int rating = caps.GetRating();
	unsigned char level = (unsigned char)(caps.GetLevel() & 0x000000ff);
	level = levelCaps.MergeRatingVsLevel(rating, level);
	tc_levelVideo_t lvlDesc;
	levelCaps.GetLevelDesc(level, &lvlDesc);
	int rcvMBps = rcvmf.GetMBps();
	if (rcvMBps > lvlDesc.maxMBps) {
		int roundRes = bCanMul8 ? 0x7 : 0xf;
		double rcvAR = (double)rcvmf.dwVideoWidht / (double) rcvmf.dwVideoHeight;
		double MBFrame = (double)lvlDesc.maxMBps / (double)rcvmf.dwFps;
		double w = sqrt(256.0 * MBFrame * rcvAR);
		double h = w / rcvAR;
		sndmf.dwVideoWidht  = ((int)w) &~ roundRes;
		sndmf.dwVideoHeight = ((int)h) &~ roundRes;
	}
	if (!bCanMul8) {
		if (sndmf.dwVideoWidht != (sndmf.dwVideoWidht &~ 0xf)) return false;
		if (sndmf.dwVideoHeight != (sndmf.dwVideoHeight &~ 0xf)) return false;
	}
	return true;
}