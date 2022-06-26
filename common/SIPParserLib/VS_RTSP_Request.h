#pragma once

#include "VS_RTSPMessage.h"

class VS_RTSP_ParserInfo;

enum eRequestType : int;

class VS_RTSP_Request : public VS_RTSPMessage
{
	bool MakeRequestGeneric(VS_RTSP_ParserInfo * ctx, eRequestType type);

public:
	bool MakeOptions(VS_RTSP_ParserInfo * ctx);
	bool MakeGetParameter(VS_RTSP_ParserInfo * ctx);
	bool MakeDescribe(VS_RTSP_ParserInfo * ctx);
	bool MakeSetup(VS_RTSP_ParserInfo * ctx);
	bool MakePlay(VS_RTSP_ParserInfo * ctx);
	bool MakePause(VS_RTSP_ParserInfo * ctx);
	bool MakeTeardown(VS_RTSP_ParserInfo * ctx);

};