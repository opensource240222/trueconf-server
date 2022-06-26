#pragma once

#include "streams/Protocol.h"

namespace ffl {

struct FrameMetadata
{
	stream::Track track;
	unsigned long interval;
	bool keyframe;

	static FrameMetadata MakeAudio()
	{
		return FrameMetadata { stream::Track::audio, 0, 0 };
	}
	static FrameMetadata MakeVideo(unsigned long interval, bool keyframe)
	{
		return FrameMetadata { stream::Track::video, interval, keyframe};
	}
	static FrameMetadata MakeCommand()
	{
		return FrameMetadata { stream::Track::command, 0, 0 };
	}
};

}