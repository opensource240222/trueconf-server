#pragma once

#include "std/cpplib/VS_MediaFormat.h"

namespace ffl {
	struct FilterCommand
	{
		enum Type
		{
			e_keyFrameRequest,
			e_changeFormatRequest,
			e_setBitrateRequest,
		} type;
		VS_MediaFormat mf;
		unsigned int bitrate;

		static FilterCommand MakeKeyFrameRequest()
		{
			return FilterCommand {e_keyFrameRequest, VS_MediaFormat(), 0};
		}
		static FilterCommand MakeChangeFormatRequest(const VS_MediaFormat& mf)
		{
			return FilterCommand {e_changeFormatRequest, mf, 0};
		}
		static FilterCommand MakeSetBitrateRequest(unsigned int bitrate)
		{
			return FilterCommand {e_setBitrateRequest, VS_MediaFormat(), bitrate};
		}
	};
}