#pragma once

#include "VS_ClientCaps.h"
#include "VS_MediaFormat_io.h"

#include <ostream>
#include <iomanip>

template <class charT, class traits>
std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& s, const VS_ClientCaps& x)
{
	auto flags(s.flags());
	s.width(0);

	s << std::hex << std::setfill('0');
	s <<   "stream: 0x" << std::setw(4) << x.GetStreamsDC();
	s << ", audio_rcv: 0x" << std::setw(4) << x.GetAudioRcv();
	s << ", video_rcv: 0x" << std::setw(4) << x.GetVideoRcv();
	s << ", audio_snd: 0x" << std::setw(4) << x.GetAudioSnd();
	s << ", video_snd: 0x" << std::setw(4) << x.GetVideoSnd();
	s << '\n';

	s << std::dec;
	s <<   "band_rcv: " << x.GetBandWRcv();
	s << ", rating: " << x.GetRating();
	s << ", level: " << x.GetLevel();
	s << ", level_group: " << x.GetLevelGroup();
	s << ", screen_w: " << x.GetScreenWidth();
	s << ", screen_h: " << x.GetScreenHeight();
	s << '\n';

	VS_MediaFormat mf;
	x.GetMediaFormat(mf);
	s << "mf: \"" << mf << "\"\n";

	s << "ac: { ";
	uint16_t acodecs[100];
	size_t acodecs_size = sizeof(acodecs)/sizeof(acodecs[0]);
	x.GetAudioCodecs(acodecs, acodecs_size);
	for (auto i = 0u; i < acodecs_size; ++i)
		s << '"' << stream_acodec(acodecs[i]) << "\" ";
	s << "}\n";

	s << "vc: { ";
	uint32_t vcodecs[100];
	size_t vcodecs_size = sizeof(vcodecs)/sizeof(vcodecs[0]);
	x.GetVideoCodecs(vcodecs, vcodecs_size);
	for (auto i = 0u; i < vcodecs_size; ++i)
		s << '"' << stream_vcodec(vcodecs[i]) << "\" ";
	s << "}\n";

	s.flags(flags);
	return s;
}
