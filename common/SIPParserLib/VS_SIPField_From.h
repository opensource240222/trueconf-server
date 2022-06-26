#pragma once


#include <boost/regex.hpp>
#include "../SIPParserBase/VS_BaseField.h"

class VS_SIPURI;

class VS_SIPField_From: public VS_BaseField
{
public:
	const static boost::regex e;

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;
	void Clean() noexcept override;

	VS_SIPURI* GetURI() const;
	void SetURI(const VS_SIPURI* uri);

	VS_SIPField_From();
	~VS_SIPField_From();

	int order() const override
	{
		return 40;
	}
private:
	VS_SIPURI* iSIPURI;
	bool compact;
};

std::unique_ptr<VS_BaseField> VS_SIPField_From_Instance();