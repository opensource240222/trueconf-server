#include "VS_BaseField.h"

TSIPErrorCodes VS_BaseField::Init(const VS_SIPGetInfoInterface&)
{
	return TSIPErrorCodes::e_InputParam;
}

TSIPErrorCodes VS_BaseField::Init(VS_RTSP_ParserInfo*)
{
	return TSIPErrorCodes::e_InputParam;
}

VS_BaseField::VS_BaseField():
	VS_SIPError(TSIPErrorCodes::e_null, false)
{

}

VS_BaseField::VS_BaseField(const VS_BaseField& src) : VS_SIPError(src)
{
}

VS_BaseField::~VS_BaseField()
{

}

int VS_BaseField::order() const
{
	return -1;
}

VS_BaseField& VS_BaseField::operator=(const VS_BaseField& src)
{
	this->VS_SIPError::operator=(src);
	return *this;
}

bool VS_BaseField::operator!=(const VS_BaseField& src) const
{
	return VS_SIPError::operator!=(src);
}
