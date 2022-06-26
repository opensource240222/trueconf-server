#pragma once

#include "../SIPParserBase/VS_BaseField.h"

#include <boost/regex.hpp>

class VS_SIPField_MinSE: public VS_BaseField
{
public:
	const static boost::regex e;

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	void Clean() noexcept override;

	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;
	VS_SIPField_MinSE();
	~VS_SIPField_MinSE();

protected:
	unsigned int GetValue() const { return this->m_value; }
	void SetValue(const unsigned int value) { this->m_value = value; }

private:
	unsigned int		m_value;

};

std::unique_ptr<VS_BaseField> VS_SIPField_MinSE_Instance();