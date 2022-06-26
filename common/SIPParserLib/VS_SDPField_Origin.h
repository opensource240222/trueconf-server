#pragma once

#include "../SIPParserBase/VS_BaseField.h"

#include <boost/regex.hpp>

class VS_SDPConnect;

class VS_SDPField_Origin : public VS_BaseField
{
public:
	const static boost::regex e;

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	void Clean() noexcept override;

	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;
	VS_SDPField_Origin();
	~VS_SDPField_Origin();
	int order() const override
	{
		return 20;
	}
	const std::string &UserName() const;
	const std::string &SessionId() const;
	const std::string &Version() const;
	const VS_SDPConnect* Connect() const;

private:
	std::string iUserName;
	std::string iSessID;
	std::string iVersion;
	VS_SDPConnect* iConnect;
};

std::unique_ptr<VS_BaseField> VS_SDPField_Origin_Instance();