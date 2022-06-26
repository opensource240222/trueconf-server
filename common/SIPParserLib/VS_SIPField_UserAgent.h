#pragma once

#include "../SIPParserBase/VS_BaseField.h"

#include <boost/regex.hpp>

#include <string>

class VS_SIPField_UserAgent: public VS_BaseField
{
public:
	const static boost::regex e1;

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;
	void Clean() noexcept override;

	const std::string &GetUserAgent() const;

	VS_SIPField_UserAgent();
	~VS_SIPField_UserAgent();

private:
	std::string			m_user_agent;
};

std::unique_ptr<VS_BaseField> VS_SIPField_UserAgent_Instance();