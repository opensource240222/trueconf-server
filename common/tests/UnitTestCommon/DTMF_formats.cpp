#if defined(_WIN32) // Not ported yet

#include "DTMF_formats.h"

sipto_dtmf_pair DTMF_formats::GetDTMFTest(std::string call_string){
	identSIP->PostResolve(cfg, call_string.c_str(), from_user.get(), false);
	return std::make_pair(cfg.resolveResult.NewCallId,cfg.resolveResult.dtmf);
}

bool  DTMF_formats::MakeFormatTest(std::string call_string, std::string expected_call_string, std::string expected_dtmf){
	sipto_dtmf_pair result = GetDTMFTest(call_string);
	if (expected_dtmf != result.second ||
		expected_call_string != result.first)
		return false;

	return true;
}

/*
 * MakeFormatTest - returns true if VS_IndentifierSIP::PostResolve function
 * will parse call string with dtmf symbols like we expect
 */
TEST_F(DTMF_formats,ext_format_test){
	std::string dtmf = "1234";

	// test ext_format
	ASSERT_TRUE(MakeFormatTest("#sip:user@1.2.3.4;ext=" + dtmf, "#sip:user@1.2.3.4", dtmf));
	ASSERT_TRUE(MakeFormatTest("#sip:user@1.2.3.4;transport=udp;ext=" + dtmf, "#sip:user@1.2.3.4", dtmf));
	ASSERT_TRUE(MakeFormatTest("#sip:user@1.2.3.4;extension=" + dtmf, "#sip:user@1.2.3.4", dtmf));
	ASSERT_TRUE(MakeFormatTest("#sip:user@1.2.3.4;transport=udp;extension=" + dtmf, "#sip:user@1.2.3.4", dtmf));

	// test comma
	ASSERT_TRUE(MakeFormatTest("#sip:user@1.2.3.4," + dtmf, "#sip:user@1.2.3.4", "," + dtmf));
	ASSERT_TRUE(MakeFormatTest("#sip:user@1.2.3.4;transport=udp," + dtmf, "#sip:user@1.2.3.4", "," + dtmf));

	// semicolumn test
	ASSERT_TRUE(MakeFormatTest("#sip:user@1.2.3.4;" + dtmf, "#sip:user@1.2.3.4", dtmf));
	ASSERT_TRUE(MakeFormatTest("#sip:user@1.2.3.4;transport=udp;" + dtmf, "#sip:user@1.2.3.4", dtmf));
	ASSERT_TRUE(MakeFormatTest("#sip:user@1.2.3.4;" + dtmf + ";transport=udp", "#sip:user@1.2.3.4", dtmf));
}

#endif
