#pragma once

#include "../SIPParserBase/VS_BaseField.h"

#include <boost/regex.hpp>

class VS_SIPURI;

class VS_SIPField_To: public VS_BaseField
{
public:
	const static boost::regex e;

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;


	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;
	void Clean() noexcept override;

	VS_SIPURI* GetURI() const;
	void SetURI(const VS_SIPURI* uri);

	VS_SIPField_To();
	VS_SIPField_To(const VS_SIPField_To &) = delete;
	~VS_SIPField_To();

	int order() const override
	{
		return 50;
	}

private:
	VS_SIPURI* iSIPURI;
	bool compact;
};

std::unique_ptr<VS_BaseField> VS_SIPField_To_Instance();