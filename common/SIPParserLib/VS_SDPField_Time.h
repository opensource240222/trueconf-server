#pragma once

#include <boost/regex.hpp>
#include "../SIPParserBase/VS_BaseField.h"

class VS_SDPField_Time: public VS_BaseField
{
public:
	const static boost::regex e;

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	void Clean() noexcept override;


	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;
	VS_SDPField_Time();
	~VS_SDPField_Time();

	int order() const override
	{
		return 60;
	}
private:
	std::uint64_t						m_start_time;
	std::uint64_t						m_stop_time;
};

std::unique_ptr<VS_BaseField>  VS_SDPField_Time_Instance();