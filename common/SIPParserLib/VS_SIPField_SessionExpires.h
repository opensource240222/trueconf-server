#pragma once

#include "../SIPParserBase/VS_BaseField.h"

#include <boost/regex.hpp>
#include <chrono>

class VS_SIPField_SessionExpires: public VS_BaseField
{
public:
	const static boost::regex e;
	const static boost::regex e1;

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	void Clean() noexcept override;

	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;

	VS_SIPField_SessionExpires();
	~VS_SIPField_SessionExpires();


	REFRESHER GetRefresher() const;
	std::chrono::steady_clock::duration GetRefreshInterval() const;

private:
	std::chrono::steady_clock::duration		m_value;

	REFRESHER			m_refresher;

	bool FindParam_refresher(string_view aInput);
};

std::unique_ptr<VS_BaseField> VS_SIPField_SessionExpires_Instance();