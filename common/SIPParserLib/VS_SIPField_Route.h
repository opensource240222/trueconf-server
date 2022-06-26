#pragma once

#include "../SIPParserBase/VS_BaseField.h"

#include <boost/regex.hpp>

class VS_SIPURI;

class VS_SIPField_Route: public VS_BaseField
{
public:
	const static boost::regex e;

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;


	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;
	void Clean() noexcept override;

	VS_SIPURI* GetURI() const;

	VS_SIPField_Route();
	~VS_SIPField_Route();

	int order() const override
	{
		return 25;
	}

private:
	VS_SIPURI* iSIPURI;
};

std::unique_ptr<VS_BaseField> VS_SIPField_Route_Instance();