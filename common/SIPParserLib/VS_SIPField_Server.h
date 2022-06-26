#pragma once

#include "../SIPParserBase/VS_BaseField.h"

#include <boost/regex.hpp>

#include <string>

enum eSIP_UserAgents;

class VS_SIPField_Server : public VS_BaseField
{
public:
	const static boost::regex e;

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;
	void Clean() noexcept override;

	const std::string &GetServer() const;

	VS_SIPField_Server();
	~VS_SIPField_Server();

private:
	std::string			m_server;
};

std::unique_ptr<VS_BaseField> VS_SIPField_Server_Instance();