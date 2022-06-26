#pragma once

#include "../SIPParserBase/VS_BaseField.h"
#include <boost/regex.hpp>
#include <chrono>

class VS_SIPField_RetryAfter : public VS_BaseField
{
public:
	const static boost::regex e;

	VS_SIPField_RetryAfter();

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;
	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;

	std::chrono::seconds Value() const { return this->m_value; }
	void Value(std::chrono::seconds val) { this->m_value = val; }
	std::chrono::steady_clock::duration  Duration() const { return this->m_duration; }
	void  Duration(std::chrono::steady_clock::duration val) { this->m_duration = val; }
	const std::string &Comment() const { return this->m_comment; }
private:
	std::string   m_comment;						// Message after "Retry-After: val" such as: Retry-After: 120 (I'm in a meeting)
	std::chrono::seconds m_value;					// seconds to retry after
	std::chrono::steady_clock::duration m_duration;	// Retry-After: 18000;duration=3600
};

std::unique_ptr<VS_BaseField> VS_SIPField_RetryAfter_Instance();