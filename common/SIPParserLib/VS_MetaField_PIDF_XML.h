#pragma once

#include "../SIPParserBase/VS_BaseField.h"


class VS_MetaField_PIDF_XML: public VS_BaseField
{
public:
	VS_MetaField_PIDF_XML();
	virtual ~VS_MetaField_PIDF_XML();

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;

private:
	std::string			m_alias;
};