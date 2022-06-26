#include "VS_MetaField_PIDF_XML.h"
#include "VS_SIPGetInfoInterface.h"

VS_MetaField_PIDF_XML::VS_MetaField_PIDF_XML()
{

}

VS_MetaField_PIDF_XML::~VS_MetaField_PIDF_XML()
{


}

TSIPErrorCodes VS_MetaField_PIDF_XML::Decode(VS_SIPBuffer &aBuffer)
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


	//boost::regex e(
	//	"(?i)"
	//		".*"
	//		"< *\\? *xml *[^>]+>"
	//			" *(?:\\n)+"
	//		"< *media_control *>"
	//			" *(?:\\n)+"
	//		"< *vc_primitive *>"
	//			" *(?:\\n)+"
	//		"< *to_encoder *>"
	//			" *(?:\\n)+"
	//		"< *picture_fast_update */? *>"
	//			" *(?:\\n)+"
	//		"(?:< */ *picture_fast_update *>"
	//			" *(?:\\n)+"
	//		")?"
	//		"< */ *to_encoder *>"
	//			" *(?:\\n)+"
	//		"< */ *vc_primitive *>"
	//			" *(?:\\n)+"
	//		"< */ *media_control *>"
	//			" *(?:\\n)?"
	//		".*"
	//	"(?-i) *"
	//);
	//boost::cmatch m;

	//if ( !boost::regex_match(ptr.get(), m, e) )
	//	m_fast_update_picture = false;

	//m_fast_update_picture = true;

	SetValid(false);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_MetaField_PIDF_XML::Encode(VS_SIPBuffer &aBuffer) const
{
	if ( !m_alias.length() )
		return TSIPErrorCodes::e_InputParam;

	std::string out =
		"<?xml version=\"1.0\" encoding=\"utf-8\" ?>\r\n"
		"<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" entity=\""; out += m_alias; out += "\">\r\n"
			"<tuple id=\"id\">\r\n"
				"<status>\r\n"
					"<basic>open</basic>\r\n"
				"</status>\r\n"
			"</tuple>\r\n"
		"</presence>\r\n"
		;

	return aBuffer.AddData(out);
}

TSIPErrorCodes VS_MetaField_PIDF_XML::Init(const VS_SIPGetInfoInterface& call)
{


	string_view alias = call.GetAliasMy();
	if (alias.empty())
	{
		SetValid(false);
		SetError(TSIPErrorCodes::e_InputParam);
		return TSIPErrorCodes::e_InputParam;
	}

	m_alias = std::string(alias);

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}