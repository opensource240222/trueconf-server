#include "VS_MetaField_MediaControl_XML.h"
#include "std/debuglog/VS_Debug.h"

#include <boost/algorithm/string/case_conv.hpp>

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

VS_MetaField_MediaControl_XML::VS_MetaField_MediaControl_XML(): m_fast_update_picture(false)
{

}

VS_MetaField_MediaControl_XML::~VS_MetaField_MediaControl_XML()
{


}

TSIPErrorCodes VS_MetaField_MediaControl_XML::Decode(VS_SIPBuffer &aBuffer)
{
	std::unique_ptr<char[]> ptr;
	std::size_t ptr_sz = 0;
	TSIPErrorCodes err = aBuffer.GetAllDataAllocConst(ptr, ptr_sz);
	if (TSIPErrorCodes::e_ok != err )
	{
		SetValid(false);
		SetError(err);
		return err;
	}

	if ( !ptr || !ptr_sz )
	{
		SetValid(false);
		SetError(TSIPErrorCodes::e_buffer);
		return TSIPErrorCodes::e_buffer;
	}

	std::string lower = ptr.get();
	boost::to_lower(lower);
	m_fast_update_picture =
		lower.find("xml") != std::string::npos &&
		lower.find("media_control") != std::string::npos &&
		lower.find("vc_primitive") != std::string::npos &&
		lower.find("to_encoder") != std::string::npos &&
		lower.find("picture_fast_update") != std::string::npos;

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_MetaField_MediaControl_XML::Encode(VS_SIPBuffer &aBuffer) const
{
	return aBuffer.AddData(
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
			"<media_control>"
				"<vc_primitive>"
					"<to_encoder>"
						"<picture_fast_update>"
						"</picture_fast_update>"
					"</to_encoder>"
				"</vc_primitive>"
			"</media_control>"
		);
}

bool VS_MetaField_MediaControl_XML::IsFastUpdatePicture() const
{
	return m_fast_update_picture;
}

TSIPErrorCodes VS_MetaField_MediaControl_XML::Init(const VS_SIPGetInfoInterface& call)
{

	m_fast_update_picture = true;

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

#undef DEBUG_CURRENT_MODULE
