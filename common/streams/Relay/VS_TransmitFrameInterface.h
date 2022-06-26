#pragma once

#include "../fwd.h"

class VS_TransmitFrameInterface
{
public:
	virtual void TransmitFrame(const char *conf_name, const char *part, const stream::FrameHeader *frame_head, const void *frame_data) = 0;
};