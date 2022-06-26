#include "SIPField_DateTest.h"
#include "TrueGateway/sip/VS_SIPGetInfoImpl.h"

TEST_F(SIPFieldDate, EncodingDecoding){
	VS_SIPBuffer buff;

	auto separator = "\r\n";
	boost::shared_ptr<VS_SIPParserInfo> pCallInfo = boost::make_shared<VS_SIPParserInfo>("serverVendor");

	const VS_SIPGetInfoImpl get_info(*pCallInfo);
	ASSERT_EQ(TSIPErrorCodes::e_ok, date_field.Init(get_info));
	ASSERT_EQ(TSIPErrorCodes::e_ok, date_field.Encode(buff));
	buff.AddData(separator, strlen(separator));
	ASSERT_EQ(TSIPErrorCodes::e_ok, date_field.Decode(buff));
}