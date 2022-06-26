#pragma once

#include "VS_MediaFormat.h"

#include <ostream>

struct stream_vcodec
{
	uint32_t value;
	explicit stream_vcodec(uint32_t value_) : value(value_) {}
};
template <class CharT, class Traits>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& s, const stream_vcodec& x)
{
	switch (x.value)
	{
	case VS_VCODEC_H261:  s << "H.261"; break;
	case VS_VCODEC_H263:  s << "H.263"; break;
	case VS_VCODEC_H263P: s << "H.263+"; break;
	case VS_VCODEC_H264:  s << "H.264"; break;
	case VS_VCODEC_H265:  s << "H.265"; break;
	case VS_VCODEC_VPX:   s << "VP8"; break;
	case VS_VCODEC_VP9:   s << "VP9"; break;
	case VS_VCODEC_VPXHD:
	case VS_VCODEC_VPXSTEREO:
	case VS_VCODEC_XC02:
	case VS_VCODEC_MPEG4:
	default: s.write(reinterpret_cast<const char*>(&x.value), sizeof(x.value));
	}
	return s;
}

struct stream_acodec
{
	uint32_t value;
	explicit stream_acodec(uint32_t value_) : value(value_) {}
};
template <class CharT, class Traits>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& s, const stream_acodec& x)
{
	switch (x.value)
	{
	case VS_ACODEC_PCM:        s << "PCM"; break;
	case VS_ACODEC_G711a:      s << "G.711 a-law"; break;
	case VS_ACODEC_G711mu:     s << "G.711 mu-law"; break;
	case VS_ACODEC_GSM610:     s << "GSM"; break;
	case VS_ACODEC_G723:       s << "G.723"; break;
	case VS_ACODEC_G728:       s << "G.728"; break;
	case VS_ACODEC_G729A:      s << "G.729 Annex A"; break;
	case VS_ACODEC_G722:       s << "G.722"; break;
	case VS_ACODEC_G7221_24:   s << "G.722.1 24kbit/s"; break;
	case VS_ACODEC_G7221_32:   s << "G.722.1 32kbit/s"; break;
	case VS_ACODEC_SPEEX:      s << "Speex"; break;
	case VS_ACODEC_ISAC:       s << "iSAC"; break;
	case VS_ACODEC_G7221C_24:  s << "G.722.1 Annex C 24kbit/s"; break;
	case VS_ACODEC_G7221C_32:  s << "G.722.1 Annex C 32kbit/s"; break;
	case VS_ACODEC_G7221C_48:  s << "G.722.1 Annex C 48kbit/s"; break;
	case VS_ACODEC_OPUS_B0914: s << "Opus"; break;
	case VS_ACODEC_MP3:        s << "MP3"; break;
	case VS_ACODEC_AAC:        s << "AAC"; break;
	default: s << reinterpret_cast<void*>(x.value);
	}
	return s;
}

struct stream_vformat
{
	const VS_MediaFormat& value;
	explicit stream_vformat(const VS_MediaFormat& value_) : value(value_) {}
};
template <class CharT, class Traits>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& s, const stream_vformat& x)
{
	if (x.value.dwVideoCodecFCC != 0)
	{
		s << stream_vcodec(x.value.dwVideoCodecFCC) << " " << x.value.dwVideoWidht << "x" << x.value.dwVideoHeight << "@" << x.value.dwFps;
		s << " (stereo=" << x.value.dwStereo << ", svc=" << x.value.dwSVCMode << ", hw=" << x.value.dwHWCodec << ")";
	}
	else
		s << "none";
	return s;
}

struct stream_aformat
{
	const VS_MediaFormat& value;
	explicit stream_aformat(const VS_MediaFormat& value_) : value(value_) {}
};
template <class CharT, class Traits>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& s, const stream_aformat& x)
{
	if (x.value.dwAudioCodecTag != 0)
	{
		s << stream_acodec(x.value.dwAudioCodecTag) << " " << x.value.dwAudioSampleRate << "Hz";
		s << " (buf=" << x.value.dwAudioBufferLen << ")";
	}
	else
		s << "none";
	return s;
}

template <class CharT, class Traits>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& s, const VS_MediaFormat& x)
{
	s << "v: " << stream_vformat(x) << " a: " << stream_aformat(x);
	return s;
}