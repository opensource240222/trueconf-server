#include "VS_MetaField_DTMF_Relay.h"

#include <cstdio>
#include <cstring>

VS_MetaField_DTMF_Relay::VS_MetaField_DTMF_Relay(const char dtmf_digit): m_dtmf_digit(dtmf_digit)
{
}

VS_MetaField_DTMF_Relay::~VS_MetaField_DTMF_Relay()
{
}

TSIPErrorCodes VS_MetaField_DTMF_Relay::Decode(VS_SIPBuffer &aBuffer)
{
	//std::unique_ptr<char[]> ptr;
	//unsigned int ptr_sz = 0;
	//int err = e_null;

	//err = aBuffer.GetAllDataAllocConst(ptr, ptr_sz);
	//if ( e_ok != err )
	//{
	//	if ( ptr ) { delete ptr; ptr = 0; } ptr_sz = 0;

	//	SetValid(false);
	//	SetError(err);
	//	return err;
	//}

	//if ( !ptr || !ptr_sz )
	//{
	//	if ( ptr ) { delete ptr; ptr = 0; } ptr_sz = 0;

	//	SetValid(false);
	//	SetError(e_buffer);
	//	return e_buffer;
	//}

	//boost::cmatch m;

	//if ( !boost::regex_match(ptr.get(), m, e) )
	//	m_fast_update_picture = false;

	//m_fast_update_picture = true;

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_MetaField_DTMF_Relay::Encode(VS_SIPBuffer &aBuffer) const
{
	char dtmf_body[128] = {0};
	snprintf(dtmf_body, sizeof(dtmf_body), "Signal= %c\r\nDuration= 250\r\n", m_dtmf_digit);
	return aBuffer.AddData(dtmf_body, strlen(dtmf_body));
}

TSIPErrorCodes VS_MetaField_DTMF_Relay::Init(const VS_SIPGetInfoInterface& call)
{
	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}