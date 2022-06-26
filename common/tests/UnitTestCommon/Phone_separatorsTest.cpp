#if defined(_WIN32) // Not ported yet

#include "Phone_separatorsTest.h"

bool PhoneSeparators::MakeTest(std::string input_call_string, std::string expected_output){
	return identSIP->PostResolve(cfg, input_call_string.c_str(), from_user.get(), false)
		&& expected_output == cfg.resolveResult.NewCallId;
}

TEST_F(PhoneSeparators, RemovingSeparators){
	ASSERT_TRUE(MakeTest("#tel:+3(123) 456-78.90@1.2.3.4", "#tel:+31234567890@1.2.3.4"));
	ASSERT_TRUE(MakeTest("#tel:+3(123) 456 - 78.90@1.2.3.4", "#tel:+31234567890@1.2.3.4"));
	ASSERT_TRUE(MakeTest("+3(123) 456-78.90@1.2.3.4", "#tel:+31234567890@1.2.3.4"));
	ASSERT_TRUE(MakeTest("+49/ (0) 228/ 18 20@1.2.3.4", "#tel:+4902281820@1.2.3.4"));
}

#endif
