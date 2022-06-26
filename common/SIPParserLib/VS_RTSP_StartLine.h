#pragma once

#include "../SIPParserBase/VS_BaseField.h"

#include <memory>

enum eRequestType : int;
enum eResponseType : int;

class VS_RTSP_StartLine : public VS_BaseField
{
public:
	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;
	TSIPErrorCodes Init (VS_RTSP_ParserInfo * parsingInfo) override;

	VS_RTSP_StartLine();

	int getReturnCode() const;

private:
	std::string m_url;
	eResponseType m_responseType;
	eRequestType m_requestType;
	int m_returnCode;
	bool bResponseOrRequest;// true - response, false - request
};

std::unique_ptr<VS_BaseField> VS_RTSP_StartLine_Instance();