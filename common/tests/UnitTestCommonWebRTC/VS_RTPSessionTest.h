#include <gtest/gtest.h>

#include <memory>

class VS_RTPSessionTestImpl;
class VS_FFLSourceCollection;
struct RTPSessionTest : public ::testing::Test {
	std::string my_id = "17496DA8C485B810A57EC9ED577041B7";
	std::string my_part_id = "#sip:@127.0.0.1/17496DA8C485B810A57EC9ED577041B7";
	std::string my_session_key = "008E859993056C3A0BFDFA62B41567A299740A19";

	std::shared_ptr<VS_FFLSourceCollection> my_collection;
	void SetUp();
	void TearDown();
};