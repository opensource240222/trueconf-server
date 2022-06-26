#pragma once

#include "../SIPParserBase/VS_BaseField.h"

#include <boost/regex.hpp>

#define RFC822_TIME_FMT "%a, %d %b %Y %H:%M:%S GMT"

class VS_SIPField_Date : public VS_BaseField{
	time_t m_date;

	static int DayOfWeekToInt(const std::string &day);
	static int MonthToInt(const std::string &month);
public:
	const static boost::regex e;

	VS_SIPField_Date();
	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;
};

std::unique_ptr<VS_BaseField> VS_SIPField_Date_Instance();