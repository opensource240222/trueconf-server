#pragma once

#include "../SIPParserBase/VS_ObjectFactory.h"
#include "VS_RTSP_Const.h"

#include <string>

class VS_RTSPObjectFactory : public VS_ObjectFactory
{
public:
	enum class RTSPHeader
	{
		CSEQ,
		SERVER,
		STARTLINE,
		DATE,
		TRANSPORT,
		USER_AGENT,
		PUBLIC,
		SESSION,
		ACCEPT,
		CONTENT_TYPE,
		CONTENT_LENGTH,
		AUTHORIZATION,
		WWW_AUTHENTICATE
	};
	static VS_RTSPObjectFactory* Instance();
	CreateFieldResult CreateField(VS_SIPBuffer &sbInput)const override;
	CreateFieldResult CreateField(VS_RTSPObjectFactory::RTSPHeader header) const;

	const std::string& GetMethod(eRequestType method) const;
	eRequestType GetMethod(const std::string& method) const;

private:
	TSIPErrorCodes Init();
	static VS_RTSPObjectFactory * iThis;
};
