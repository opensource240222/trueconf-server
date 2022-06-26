#pragma once

#include "../SIPParserBase/VS_BaseField.h"

#include <boost/regex.hpp>
#include <chrono>

class VS_SIPField_MinExpires: public VS_BaseField
{
public:
	const static boost::regex e;

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	void Clean() noexcept override;

	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;
	VS_SIPField_MinExpires();
	~VS_SIPField_MinExpires(){}

	std::chrono::seconds GetValue() const { return this->m_value; }
	void SetValue(std::chrono::seconds value) { this->m_value = value; }

private:
	std::chrono::seconds		m_value;

};

std::unique_ptr<VS_BaseField> VS_SIPField_MinExpires_Instance();
