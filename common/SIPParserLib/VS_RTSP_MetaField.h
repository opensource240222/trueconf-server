#pragma once

#include "../SIPParserBase/VS_BaseField.h"

#include <memory>
#include <string>
#include <vector>

class VS_RTSP_StartLine;
class VS_RTSP_Session;
class VS_RTSP_Content_type;
class VS_RTSP_ContentLength;
class VS_RTSP_Transport;
class VS_RTSP_Public;
class VS_RTSP_Server;
class VS_RTSP_UserAgent;
class VS_SIPField_Auth;

enum eRequestType : int;

class VS_RTSP_MetaField : public VS_BaseField
{
public:

	VS_RTSP_MetaField();
	~VS_RTSP_MetaField();
	TSIPErrorCodes Decode (VS_SIPBuffer & buffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;
	TSIPErrorCodes AddField(std::unique_ptr<VS_BaseField>&& field);
	eRequestType m_MessageType;

	VS_RTSP_StartLine		*iStartLine;
	VS_RTSP_Session			*iSession;
	VS_RTSP_Content_type	*iContentType;
	VS_RTSP_ContentLength	*iContentLength;
	VS_RTSP_Transport		*iTransport;
	VS_RTSP_Public			*iPublic;
	VS_RTSP_Server          *iServer;
	VS_RTSP_UserAgent       *iUserAgent;
	VS_SIPField_Auth		*iAuth;

	//TODO: not found impl
	std::string GetId();
	int Clear();
	TSIPErrorCodes CreateStatic();
private:
	std::vector<std::unique_ptr<VS_BaseField> > m_field_container;

};