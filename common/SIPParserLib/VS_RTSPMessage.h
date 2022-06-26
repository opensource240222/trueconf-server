#pragma once

#include "../SIPParserBase/VS_SIPError.h"
#include "VS_RTSPObjectFactory.h"

class VS_RTSP_ParserInfo;
class VS_RTSP_MetaField;
class VS_SDPMetaField;

enum eRequestType : int;
enum eResponseType : int;

class VS_RTSPMessage : public VS_SIPError
{
public:
	~VS_RTSPMessage();
	VS_RTSPMessage();
	TSIPErrorCodes Decode(const char* aInput, std::size_t aSize);
	TSIPErrorCodes Encode(char* aOutput, std::size_t &aSize) const;
	eRequestType GetType() const;
	bool InsertRTSPField(VS_RTSPObjectFactory::RTSPHeader header, VS_RTSP_ParserInfo * info);

	VS_RTSP_MetaField *GetRTSPMetaField() const;
	VS_SDPMetaField *GetSDPPMetaField() const;

protected:
	eRequestType m_request_type;
	eResponseType m_response_type;
	VS_RTSP_MetaField  * m_meta_field;
	VS_SDPMetaField * m_SdpMetaField;
};