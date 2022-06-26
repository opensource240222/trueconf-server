#pragma once
#include "../SIPParserBase/VS_BaseField.h"

class VS_MetaField_MediaControl_XML: public VS_BaseField
{
public:
	VS_MetaField_MediaControl_XML();
	~VS_MetaField_MediaControl_XML();

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;

	bool IsFastUpdatePicture() const;

private:
	bool m_fast_update_picture;
};