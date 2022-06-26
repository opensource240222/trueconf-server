#include "VS_RTSP_Request.h"
#include "VS_RTSP_Const.h"
#include "VS_RTSP_ParserInfo.h"
#include "VS_RTSP_MetaField.h"
#include "VS_SIPObjectFactory.h"
#include "VS_SIPAuthScheme.h"

bool VS_RTSP_Request::MakeOptions(VS_RTSP_ParserInfo * info)
{return MakeRequestGeneric(info, REQUEST_OPTIONS); }

bool VS_RTSP_Request::MakeGetParameter(VS_RTSP_ParserInfo * info)
{return MakeRequestGeneric(info, REQUEST_GET_PARAMETER); }

bool VS_RTSP_Request::MakeDescribe(VS_RTSP_ParserInfo * info)
{return MakeRequestGeneric(info, REQUEST_DESCRIBE);}

bool VS_RTSP_Request::MakePlay(VS_RTSP_ParserInfo * info)
{return MakeRequestGeneric(info, REQUEST_PLAY);}

bool VS_RTSP_Request::MakePause(VS_RTSP_ParserInfo * info)
{return MakeRequestGeneric(info, REQUEST_PAUSE);}

bool VS_RTSP_Request::MakeSetup(VS_RTSP_ParserInfo * info)
{	return MakeRequestGeneric(info, REQUEST_SETUP);}

bool VS_RTSP_Request::MakeTeardown(VS_RTSP_ParserInfo * info )
{	return MakeRequestGeneric(info, REQUEST_TEARDOWN);}

bool VS_RTSP_Request::MakeRequestGeneric(VS_RTSP_ParserInfo * info, eRequestType type)
{
	if(!info) return false;

	VS_RTSPObjectFactory* factory = VS_RTSPObjectFactory::Instance();
	assert(factory);

	VS_SIPObjectFactory* sip_factory = VS_SIPObjectFactory::Instance();
	assert(sip_factory);

	info->IsRequest(true);
	info->IncreaseCSeq();
	info->SetRequestType(type);

	if(m_meta_field!=NULL) delete m_meta_field;
	m_meta_field = new VS_RTSP_MetaField;

	if (!InsertRTSPField(VS_RTSPObjectFactory::RTSPHeader::STARTLINE, info)) return false;
	if (!InsertRTSPField(VS_RTSPObjectFactory::RTSPHeader::CSEQ, info)) return false;
	if (!InsertRTSPField(VS_RTSPObjectFactory::RTSPHeader::USER_AGENT, info)) return false;

	if (auto auth_info = info->GetAuthScheme())
	{
		auth_info->uri(info->GetUrl());
		auth_info->method(factory->GetMethod(info->GetRequestType()));
		sip_factory->CalcDigestResponse(auth_info.get());
		if (!InsertRTSPField(VS_RTSPObjectFactory::RTSPHeader::AUTHORIZATION, info)) return false;
	}

	if (type == REQUEST_SETUP && !InsertRTSPField(VS_RTSPObjectFactory::RTSPHeader::TRANSPORT, info)) return false;
	if (type == REQUEST_DESCRIBE && !InsertRTSPField(VS_RTSPObjectFactory::RTSPHeader::ACCEPT, info)) return false;
	if (!info->GetSessionsID().empty() && !InsertRTSPField(VS_RTSPObjectFactory::RTSPHeader::SESSION, info)) return false;

	return true;
}
