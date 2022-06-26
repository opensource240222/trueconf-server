#pragma once

#include "VS_SIPError.h"
#include "VS_SIPBuffer.h"

class VS_RTSP_ParserInfo;
class VS_SIPGetInfoInterface;

class VS_BaseField: public VS_SIPError
{
public:
	virtual TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) = 0;
	virtual TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const = 0;

	virtual TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call);
	virtual TSIPErrorCodes Init(VS_RTSP_ParserInfo* call);

	VS_BaseField();
	VS_BaseField(const VS_BaseField& src);

	virtual ~VS_BaseField();
	virtual int order() const;

	VS_BaseField& operator=(const VS_BaseField& src);
protected:
	bool operator!=(const VS_BaseField& src) const;
};