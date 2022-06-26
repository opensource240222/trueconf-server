#pragma once

#include "VS_RTSPMessage.h"

class VS_RTSP_ParserInfo;

class VS_RTSP_Response : public VS_RTSPMessage
{
public:
	bool MakeOptions(VS_RTSP_ParserInfo * ctx) const;
};