#if defined(_WIN32) // Not ported yet

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "..\SIPParserLib\VS_SIPURI.h"
#include "..\SIPParserLib\VS_SIPField_Via.h"
#include "..\SIPParserLib\VS_SDPField_Connection.h"
#include "..\SIPParserLib\VS_SDPConnect.h"


#include "../../SIPParserBase/VS_SIPBuffer.h"//need test, mb, may be deleted



TEST(SIPParserLibOperatorsTest, VS_SIPURI_OperatorsTest) {
	VS_SIPBuffer buff_1;
	VS_SIPBuffer buff_2;
	VS_SIPURI sURI_1;
	VS_SIPURI sURI_2;

	buff_1.AddData("sip:#sip:hdx8000@192.168.62.42\r\n");

	sURI_1.Decode(buff_1);

	sURI_2 = sURI_1;//verifiable function

	buff_1.Clean();
	sURI_1.Encode(buff_1);
	sURI_2.Encode(buff_2);

	ASSERT_TRUE(buff_1 == buff_2);
}


TEST(SIPParserLibOperatorsTest, VS_SIPField_Via_OperatorsTest) {
	VS_SIPBuffer buff_1;
	VS_SIPBuffer buff_2;
	VS_SIPField_Via via_1;
	VS_SIPField_Via via_2;

	buff_1.AddData("Via: SIP/2.0/UDP 192.168.41.195;branch=%s\r\n");

	via_1.Decode(buff_1);

	via_2 = via_1;//verifiable function

	buff_1.Clean();
	via_1.Encode(buff_1);
	via_2.Encode(buff_2);

	ASSERT_TRUE(buff_1 == buff_2);
}


TEST(SIPParserLibOperatorsTest, VS_SDPField_Connection_OperatorsTest) {
	VS_SIPBuffer buff_1;
	VS_SIPBuffer buff_2;
	VS_SDPField_Connection conn_1;
	VS_SDPField_Connection conn_2;

	buff_1.AddData("c=IN IP4 192.168.62.42\r\n");

	conn_1.Decode(buff_1);

	conn_2 = conn_1;//verifiable function

	buff_1.Clean();
	conn_1.Encode(buff_1);
	conn_2.Encode(buff_2);

	ASSERT_TRUE(buff_1 == buff_2);
}



TEST(SIPParserLibOperatorsTest, VS_SDPConnect_OperatorsTest) {
	VS_SIPBuffer buff_1;
	VS_SIPBuffer buff_2;
	VS_SDPConnect conn_1;
	VS_SDPConnect conn_2;

	buff_1.AddData("IN IP4 192.168.62.42\r\n");

	conn_1.Decode(buff_1);

	conn_2 = conn_1;//verifiable function

	buff_1.Clean();
	conn_1.Encode(buff_1);
	conn_2.Encode(buff_2);

	ASSERT_TRUE(buff_1 == buff_2);
}

#endif
