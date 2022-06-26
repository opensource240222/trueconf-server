#include "VS_RTSPMessage.h"
#include "VS_RTSP_MetaField.h"
#include "VS_SDPMetaField.h"
#include "VS_RTSP_Content_type.h"
#include "VS_RTSP_ContentSize.h"

#include <string>
#include "../SIPParserBase/VS_SIPBuffer.h"

VS_RTSPMessage::VS_RTSPMessage()
: m_request_type(eRequestType::REQUEST_invalid), m_response_type(eResponseType::RESPONSE_invalid), m_meta_field(nullptr), m_SdpMetaField(nullptr)
{
}

VS_RTSPMessage::~VS_RTSPMessage()
{
	delete m_meta_field;
	delete m_SdpMetaField;
}


TSIPErrorCodes VS_RTSPMessage::Decode(const char* aInput, std::size_t aSize)
{
	if ( !aInput || (aSize < 1) )
	{
		SetError(TSIPErrorCodes::e_InputParam);
		return TSIPErrorCodes::e_InputParam;
	}

	std::string __buffer;
	if ( CorrectNewLine(aInput, aSize, __buffer) )
	{
		aInput = __buffer.c_str();
		aSize = __buffer.size();
	}

	const char *body_sep = strstr(aInput, "\r\n\r\n");
	if (body_sep == nullptr)
		return TSIPErrorCodes::e_EndOfBuffer;

	VS_SIPBuffer head_buffer;
	head_buffer.AddData(aInput, (body_sep + 2) - aInput); // Store only the CRLF pair that terminates the line

	delete m_meta_field;

	m_meta_field = new VS_RTSP_MetaField;
	TSIPErrorCodes err = m_meta_field->Decode(head_buffer);

	if (m_meta_field->iContentType && m_meta_field->iContentType->GetContentType() == "APPLICATION/SDP")
	{
		const size_t body_size = (aInput + aSize) - (body_sep + 4); // body_end - body_begin
		if (m_meta_field->iContentLength ? body_size < m_meta_field->iContentLength->GetContentLength() : body_size == 0)
			return TSIPErrorCodes::e_EndOfBuffer;

		VS_SIPBuffer body_buffer;
		body_buffer.AddData(body_sep + 4, body_size);

		delete m_SdpMetaField;

		m_SdpMetaField = new VS_SDPMetaField;
		err = m_SdpMetaField->Decode(body_buffer);
	}

	return err;
}

TSIPErrorCodes VS_RTSPMessage::Encode(char* aOutput, std::size_t &aSize) const
{
	TSIPErrorCodes err = TSIPErrorCodes::e_null;
	VS_SIPBuffer RTSP, SDP;

	if (m_meta_field)
	{
		err = m_meta_field->Encode(RTSP);
		RTSP.AddData("\r\n");
	}
	if (err != TSIPErrorCodes::e_ok)
		return err;

	if (m_SdpMetaField)
	{
		err = m_SdpMetaField->Encode(SDP);
		SDP.AddData("\r\n");
	}
	if (err != TSIPErrorCodes::e_ok)
		return err;

	const size_t totalSize = RTSP.GetWriteIndex() + SDP.GetWriteIndex();
	if (totalSize > aSize)
	{
		aSize = totalSize;
		return TSIPErrorCodes::e_EndOfBuffer;
	}

	err = TSIPErrorCodes::e_null;
	char* p = aOutput;
	if (RTSP.GetWriteIndex() > 0)
	{
		err = RTSP.GetData(p, RTSP.GetWriteIndex());
		if (err != TSIPErrorCodes::e_ok)
			return err;
		p += RTSP.GetWriteIndex();
	}
	if (SDP.GetWriteIndex() > 0)
	{
		err = SDP.GetData(p, SDP.GetWriteIndex());
		if (err != TSIPErrorCodes::e_ok)
			return err;
		p += SDP.GetWriteIndex();
	}
	assert(p <= aOutput + aSize);
	aSize = p - aOutput;

	return err;
}
eRequestType VS_RTSPMessage::GetType() const
{
	return m_request_type;
}


//void VS_RTSPMessage::AddField(VS_BaseField* aBaseField)
//{
//
//}

bool VS_RTSPMessage::InsertRTSPField(VS_RTSPObjectFactory::RTSPHeader header, VS_RTSP_ParserInfo * info)
{
	if(!info) return false;

	VS_RTSPObjectFactory * factory = VS_RTSPObjectFactory::Instance();
	assert(factory);

	VS_ObjectFactory::CreateFieldResult result = factory->CreateField(header);

	if(result.error_code != TSIPErrorCodes::e_ok)return false;

	result.error_code = result.p_field->Init(info);
	if(result.error_code != TSIPErrorCodes::e_ok)	return false;

	if(m_meta_field == nullptr)
		m_meta_field = new VS_RTSP_MetaField;

	result.error_code = m_meta_field->AddField(std::move(result.p_field));
	if(result.error_code != TSIPErrorCodes::e_ok)	return false;

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return true;
}

VS_RTSP_MetaField *VS_RTSPMessage::GetRTSPMetaField() const
{
	return m_meta_field;
}

VS_SDPMetaField *VS_RTSPMessage::GetSDPPMetaField() const
{
	return m_SdpMetaField;
}
