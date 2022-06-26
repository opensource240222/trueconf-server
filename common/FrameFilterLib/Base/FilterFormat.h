#pragma once

#include "std/cpplib/VS_MediaFormat.h"

namespace ffl {
	struct FilterFormat
	{
		enum Type
		{
			e_invalid = 0,
			e_mf,
			e_rtp,
		} type;
		VS_MediaFormat mf;

		FilterFormat()
			: type(e_invalid)
		{
			mf.SetZero();
		}

		static FilterFormat MakeMF(const VS_MediaFormat& mf)
		{
			FilterFormat result;
			result.type = e_mf;
			result.mf = mf;
			return result;
		}
		static FilterFormat MakeMF(const VS_MediaFormat& mf_audio, const VS_MediaFormat& mf_video)
		{
			FilterFormat result;
			result.type = e_mf;
			result.mf.SetAudio(mf_audio.dwAudioSampleRate, mf_audio.dwAudioCodecTag);
			result.mf.SetVideo(mf_video.dwVideoWidht, mf_video.dwVideoHeight, mf_video.dwVideoCodecFCC);
			return result;
		}
		static FilterFormat MakeVideo(unsigned fourcc, unsigned width, unsigned height)
		{
			FilterFormat result;
			result.type = e_mf;
			result.mf.SetZero();
			result.mf.SetVideo(width, height, fourcc);
			return result;
		}
		static FilterFormat MakeAudio(unsigned codec, unsigned sample_rate)
		{
			FilterFormat result;
			result.type = e_mf;
			result.mf.SetZero();
			result.mf.SetAudio(sample_rate, codec);
			return result;
		}
		static FilterFormat MakeRTP()
		{
			FilterFormat result;
			result.type = e_rtp;
			return result;
		}

		friend bool operator==(const FilterFormat& l, const FilterFormat& r)
		{
			if (l.type != r.type)
				return false;
			if (l.type == e_mf)
			{
				if (!l.mf.AudioEq(r.mf) && !(l.mf.dwAudioCodecTag == 0 && r.mf.dwAudioCodecTag == 0))
					return false;
				if (!l.mf.VideoEq(r.mf) && !(l.mf.dwVideoCodecFCC == 0 && r.mf.dwVideoCodecFCC == 0))
					return false;
			}
			return true;
		}
	};
}