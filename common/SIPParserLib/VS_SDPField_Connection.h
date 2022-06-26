#pragma once

#include "../SIPParserBase/VS_BaseField.h"
#include "VS_SDPConnect.h"

#include <boost/regex.hpp>


class VS_SDPField_Connection: public VS_BaseField
{
public:
	const static boost::regex e;

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	void Clean() noexcept override;

	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;
	VS_SDPField_Connection & operator=(const VS_SDPField_Connection &conn);

	VS_SDPField_Connection();
	VS_SDPField_Connection(const VS_SDPField_Connection &) = delete;
	~VS_SDPField_Connection();

	const std::string &GetHost() const;

	void SetHost(std::string host);

	int order() const override
	{
		return 40;
	}

private:
	VS_SDPConnect iConnect;
	bool operator!=(const VS_SDPField_Connection &conn) const;
};

std::unique_ptr<VS_BaseField> VS_SDPField_Connection_Instance();