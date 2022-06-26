#pragma once

#include "../SIPParserBase/VS_BaseField.h"

#include <boost/regex.hpp>

enum eContentType : int;

class VS_SIPField_ContentType: public VS_BaseField
{
	eContentType iContentType;
	bool compact;

public:
	const static boost::regex e;

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	void Clean() noexcept override;


	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;
	eContentType GetContentType() const;

	VS_SIPField_ContentType();
	~VS_SIPField_ContentType();

	int order() const override
	{
		return 200;
	}
};

std::unique_ptr<VS_BaseField> VS_SIPField_ContentType_Instance();