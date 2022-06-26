#include "VS_RTSP_MetaField.h"
#include "VS_RTSPObjectFactory.h"
#include "VS_RTSP_StartLine.h"
#include "VS_RTSP_Session.h"
#include "VS_RTSP_Content_type.h"
#include "VS_RTSP_ContentSize.h"
#include "VS_RTSP_Transport.h"
#include "VS_RTSP_Public.h"
#include "VS_RTSP_Server.h"
#include "VS_RTSP_UserAgent.h"
#include "VS_SIPField_Auth.h"

VS_RTSP_MetaField::VS_RTSP_MetaField():m_MessageType(eRequestType::REQUEST_invalid)
{
	m_field_container.clear();
	CreateStatic();
}

VS_RTSP_MetaField::~VS_RTSP_MetaField()
{}

TSIPErrorCodes VS_RTSP_MetaField::Decode(VS_SIPBuffer & buffer)
{
	VS_ObjectFactory::CreateFieldResult result(nullptr, TSIPErrorCodes::e_null);
	VS_RTSPObjectFactory* factory = VS_RTSPObjectFactory::Instance();
	assert(factory);

	if ( !factory -> IsValid() )
		return factory->GetLastClassError();

	for(auto i = 0 ; i < 100; i++)
	{
		result = factory->CreateField(buffer);
		//code translation
		if(TSIPErrorCodes::e_ok == result.error_code)
		{
			result.error_code = result.p_field->Decode( buffer );
			//Add to Container
			if(result.error_code == TSIPErrorCodes::e_ok)
			m_field_container.push_back(std::move(result.p_field));


		}
		else if (result.error_code == TSIPErrorCodes::e_EndOfBuffer)
		{
			if (buffer.GetReadIndex() == buffer.GetWriteIndex())
				result.error_code = TSIPErrorCodes::e_ok;
			break;
		}
		else
		{
			std::unique_ptr<char[]> cP;
			std::size_t size;
			buffer.GetNextBlockAlloc(cP,size);//пропуск незнакомого поля
		}
	}
	CreateStatic();
	return result.error_code;
}


TSIPErrorCodes VS_RTSP_MetaField::Encode(VS_SIPBuffer &aBuffer) const
{
	for (const auto& i : m_field_container)
	{
		const TSIPErrorCodes err = i->Encode(aBuffer);
		if(err != TSIPErrorCodes::e_ok)
		{
			return err;
		}

	}
	return TSIPErrorCodes::e_ok;
}


TSIPErrorCodes VS_RTSP_MetaField::AddField(std::unique_ptr<VS_BaseField>&& field)
{
	m_field_container.push_back(std::move(field));
	CreateStatic();
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_RTSP_MetaField::CreateStatic()
{
	iSession		= NULL;
	iStartLine		= NULL;
	iContentType	= NULL;
	iContentLength	= NULL;
	iTransport		= NULL;
	iPublic			= NULL;
	iServer         = NULL;
	iUserAgent      = NULL;
	iAuth			= NULL;

	for (auto &i : m_field_container)
	{
		const auto& base = i.get();
		if(iSession		== NULL) iSession		= dynamic_cast<VS_RTSP_Session *> (base);
		if(iStartLine	== NULL) iStartLine		= dynamic_cast<VS_RTSP_StartLine *> (base);
		if(iContentType == NULL) iContentType	= dynamic_cast<VS_RTSP_Content_type *> (base);
		if(iContentLength == NULL) iContentLength	= dynamic_cast<VS_RTSP_ContentLength *> (base);
		if(iTransport	== NULL) iTransport		= dynamic_cast<VS_RTSP_Transport *> (base);
		if(iPublic		== NULL) iPublic		= dynamic_cast<VS_RTSP_Public *> (base);
		if(iServer      == NULL) iServer        = dynamic_cast<VS_RTSP_Server *> (base);
		if(iUserAgent   == NULL) iUserAgent     = dynamic_cast<VS_RTSP_UserAgent *> (base);
		if(iAuth		== NULL) iAuth			= dynamic_cast<VS_SIPField_Auth *> (base);
	}
	return TSIPErrorCodes::e_ok;
}

int VS_RTSP_MetaField::Clear()
{
	m_field_container.clear();
	CreateStatic();
	return 0;
}
